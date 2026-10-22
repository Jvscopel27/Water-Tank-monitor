#pragma once
#include <stdint.h>

namespace wtm {
constexpr bool finite(float v) { return v == v && v > -1.0e20f && v < 1.0e20f; }
constexpr float absValue(float v) { return v < 0 ? -v : v; }
constexpr bool validPoints(float mv1, float y1, float mv2, float y2) {
  return finite(mv1) && finite(mv2) && finite(y1) && finite(y2) &&
         mv1 >= 150 && mv1 <= 2300 && mv2 >= 150 && mv2 <= 2300 &&
         absValue(mv2 - mv1) >= 20 && y1 != y2;
}
constexpr bool between(float x, float a, float b) {
  return finite(x) && (a <= b ? x >= a && x <= b : x >= b && x <= a);
}
constexpr float interpolate(float x, float x1, float y1, float x2, float y2) {
  return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
}
constexpr bool current(uint64_t now, uint64_t stamp, bool hasValue) {
  return hasValue && now >= stamp && now - stamp <= 15000;
}
constexpr unsigned nextIndex(unsigned index, unsigned capacity) {
  return (index + 1) % capacity;
}
struct Calibration {
  uint32_t version = 1;
  float mv1 = 0, y1 = 0, mv2 = 0, y2 = 0;
  float low = 0, high = 0;
  bool verified = false, limits = false;
};
struct Reading {
  float value = 0, mv = 0;
  uint64_t stamp = 0;
  bool hasValue = false;
  const char* state = "aguardando";
};
struct Minute {
  uint64_t stamp = 0;
  float sum[4] = {};
  uint16_t count[4] = {};
};
}
