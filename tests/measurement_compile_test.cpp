#include "../firmware/WaterTankMonitor/Measurement.h"
using namespace wtm;
static_assert(validPoints(500, 4, 1500, 10), "calibracao crescente");
static_assert(validPoints(1500, 4, 500, 10), "calibracao decrescente");
static_assert(!validPoints(500, 4, 510, 10), "pontos proximos demais");
static_assert(!validPoints(10, 4, 1500, 10), "faixa inferior ADC");
static_assert(!validPoints(500, 4, 2400, 10), "faixa superior ADC");
static_assert(!validPoints(500, 7, 1500, 7), "referencias iguais");
static_assert(interpolate(1000, 500, 4, 1500, 10) == 7, "interpolacao crescente");
static_assert(interpolate(1000, 1500, 4, 500, 10) == 7, "interpolacao decrescente");
static_assert(between(1000, 1500, 500), "intervalo invertido");
static_assert(!between(1600, 1500, 500), "nao extrapolar");
static_assert(current(15000, 0, true), "limite de idade");
static_assert(!current(15001, 0, true), "dado antigo");
static_assert(!current(1000, 0, false), "sem amostra");
static_assert(!current(1000, 2000, true), "timestamp futuro");
static_assert(nextIndex(1439, 1440) == 0, "historico circular");
