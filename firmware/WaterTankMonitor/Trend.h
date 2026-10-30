#pragma once
#include <stdint.h>
struct TrendPoint {
  uint64_t stamp = 0;
  float value[4] = {};
  uint8_t valid = 0, simulated = 0;
};
constexpr unsigned TREND_SIZE = 48;
