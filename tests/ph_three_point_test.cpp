#include "../firmware/WaterTankMonitor/Measurement.h"
#include <cassert>
#include <cmath>
int main() {
  using namespace wtm;
  static_assert(sizeof(LegacyCalibration) == 32, "Legacy NVS layout changed");
  Calibration c; c.mv1=1699.975f; c.y1=3.1f; c.mv2=1253.525f; c.y2=8.3f;
  assert(validThird(c));
  assert(std::fabs(calibratedValue(c.mv1,c)-3.1f)<0.0001f);
  c.third=true; c.mv3=1425.375f; c.y3=6.3f;
  assert(validThird(c));
  assert(std::fabs(calibratedValue(c.mv3,c)-6.3f)<0.0001f);
  assert(std::fabs(calibratedValue(c.mv2,c)-8.3f)<0.0001f);
  assert(std::fabs(calibratedValue((c.mv1+c.mv3)/2,c)-4.7f)<0.0001f);
  assert(std::fabs(calibratedValue((c.mv2+c.mv3)/2,c)-7.3f)<0.0001f);
  for(float mv=c.mv2;mv<c.mv1-1;mv+=1) assert(calibratedValue(mv,c)>calibratedValue(mv+1,c));
  c.mv3=c.mv1; assert(!validThird(c));
  c.mv3=1425.375f; c.y3=9; assert(!validThird(c));
}
