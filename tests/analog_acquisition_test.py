"""Run the firmware's analog acquisition branch with a fake ADC and no thermometer."""
from pathlib import Path
import subprocess
import tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'firmware/WaterTankMonitor/WaterTankMonitor.ino').read_text()
validation=s[s.index('bool allowedReference('):s.index('void reply(')]
body=s[s.index('void collect(uint64_t now) {'):s.index('  updateAssessment(now);')]
code='''#include <cassert>
#include "Measurement.h"
using namespace wtm;
Calibration calibrations[3]; Reading readings[4];
bool demoChannels[3] = {};
unsigned ANALOG_PINS[] = {35,34,32};
float adc = 1000;
float readMillivolts(unsigned) { return adc; }
unsigned analogRead(unsigned) { return 2000; }
struct Stability { void add(float, uint64_t) {} } stableInputs[4];
''' + validation + body + '}\n' + '''
int main() {
  assert(allowedReference(2, 1413));
  assert(allowedReference(2, 12880));
  assert(!allowedReference(2, 100001));
  for(auto& c : calibrations) { c.mv1=500;c.mv2=1500;c.y1=0;c.y2=100;c.verified=true; }
  calibrations[0].y2=14;
  calibrations[2].y2=2000;
  collect(3600000);
  assert(!readings[0].hasValue); // no temperature measurement
  assert(readings[3].hasValue && readings[3].value == 1000);
  assert(readings[3].stamp == 3600000);
  adc=3000;collect(3605000);
  assert(readings[3].stamp == 3600000);
  adc=1250;collect(3610000);
  assert(readings[3].stamp == 3610000 && readings[3].value == 1500);
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++11','-I'+str(root/'firmware/WaterTankMonitor'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('EC without temperature, preserved units and recovery: PASS')
