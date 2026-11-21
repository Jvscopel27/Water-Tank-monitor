#pragma once
#include <stdint.h>

namespace phdiag {
constexpr unsigned WINDOW = 10;
constexpr uint16_t LOW_ADC = 40, HIGH_ADC = 4055;
inline float absolute(float x) { return x < 0 ? -x : x; }
struct Snapshot {
  unsigned count = 0;
  float rawMean = 0, mvMean = 0;
  uint16_t rawMin = 4095, rawMax = 0;
  uint32_t mvMin = UINT32_MAX, mvMax = 0;
  unsigned low = 0, high = 0;
  float spanMv() const { return count ? float(mvMax - mvMin) : 0; }
  // Percentual da escala completa: definido mesmo com ADC igual a zero.
  float variationPercent() const { return count ? 100.0f * (rawMax - rawMin) / 4095.0f : 0; }
  bool constant() const { return count == WINDOW && spanMv() <= 10 && rawMax - rawMin <= 12; }
  bool usable() const { return count == WINDOW && !low && !high && constant(); }
};
struct Window {
  uint16_t raw[WINDOW] = {};
  uint32_t mv[WINDOW] = {};
  unsigned count = 0, head = 0;
  void add(uint16_t r, uint32_t m) {
    raw[head] = r; mv[head] = m; head = (head + 1) % WINDOW;
    if (count < WINDOW) ++count;
  }
  Snapshot snapshot() const {
    Snapshot s; s.count = count;
    double rawSum = 0, mvSum = 0;
    for (unsigned i = 0; i < count; ++i) {
      rawSum += raw[i]; mvSum += mv[i];
      if (raw[i] < s.rawMin) s.rawMin = raw[i];
      if (raw[i] > s.rawMax) s.rawMax = raw[i];
      if (mv[i] < s.mvMin) s.mvMin = mv[i];
      if (mv[i] > s.mvMax) s.mvMax = mv[i];
      if (raw[i] <= LOW_ADC) ++s.low;
      if (raw[i] >= HIGH_ADC) ++s.high;
    }
    if (count) { s.rawMean = rawSum / count; s.mvMean = mvSum / count; }
    return s;
  }
};
struct Comparison {
  bool valid = false, significant = false;
  float deltaMv = 0, thresholdMv = 20;
};
inline Comparison compare(const Snapshot& a, const Snapshot& b) {
  Comparison c; c.valid = a.usable() && b.usable();
  c.deltaMv = b.mvMean - a.mvMean;
  const float noise = 3 * (a.spanMv() + b.spanMv());
  if (noise > c.thresholdMv) c.thresholdMv = noise;
  c.significant = c.valid && absolute(c.deltaMv) > c.thresholdMv;
  return c;
}
}
