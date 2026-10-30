#pragma once
#include <stdint.h>
#include "Measurement.h"
struct Settings {
  uint32_t version = 1;
  uint8_t demoMask = 0, dark = 0, layout = 0, focus = 0;
  uint8_t rotation = 0, inverse = 0, contrast = 160, reserved = 0;
  float temperatureOffset = 0;
};
inline bool validSettings(const Settings& s) {
  return s.version == 1 && s.demoMask <= 7 && s.dark <= 1 && s.layout <= 3 && s.focus <= 3 &&
    (s.rotation == 0 || s.rotation == 2) && s.inverse <= 1 && s.contrast >= 32 &&
    wtm::finite(s.temperatureOffset) && wtm::absValue(s.temperatureOffset) <= 5;
}
struct Stability {
  float values[4] = {};
  unsigned count = 0, head = 0;
  uint64_t stamp = 0;
  void add(float v, uint64_t now) { values[head] = v; head = (head + 1) % 4; if (count < 4) ++count; stamp = now; }
  float mean() const { float s = 0; for (unsigned i = 0; i < count; ++i) s += values[i]; return count ? s / count : 0; }
  float spread() const {
    if (!count) return 0;
    float lo = values[0], hi = lo;
    for (unsigned i = 0; i < count; ++i) { if (values[i] < lo) lo = values[i]; if (values[i] > hi) hi = values[i]; }
    return hi - lo;
  }
  bool stable(uint64_t now, float tolerance) const {
    if (count != 4 || !wtm::current(now, stamp, true)) return false;
    return spread() <= tolerance;
  }
};
struct CalibrationDraft { bool first = false; float mv = 0, reference = 0; uint64_t stamp = 0; };
