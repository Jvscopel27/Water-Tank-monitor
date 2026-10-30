// Protótipo real. Calibrar e validar antes de interpretar resultados.
// A demonstração com controles artificiais permanece em simulation/wokwi.
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_timer.h>
#include <esp_system.h>
#include "Measurement.h"
#include "Panel.h"
#include "Report.h"
#include "Interactions.h"
#include "Settings.h"
#include "Trend.h"
using namespace wtm;

constexpr uint8_t ANALOG_PINS[] = {35, 34, 32}; // pH, turbidez, EC
constexpr const char* CHANNELS[] = {"ph", "turb", "ec"};
constexpr unsigned HISTORY_SIZE = 1440;
constexpr uint64_t SAMPLE_PERIOD = 5000;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
OneWire oneWire(4);
DallasTemperature thermometer(&oneWire);
DeviceAddress temperatureAddress;
WebServer server(80);
Preferences preferences;
Calibration calibrations[3];
Reading readings[4]; // temperatura, pH, turbidez, EC
Minute history[HISTORY_SIZE], bucket;
unsigned historyHead = 0, historyCount = 0;
uint64_t sessionUntil = 0, nextSample = 0, conversionReady = 0, bucketStart = 0, nextDisplay = 0;
bool converting = false, sensorFound = false, displayReady = false, storageReady = false;
unsigned displayPage = 0, outsideStreak = 0;
String serialLine, apPassword, commandReply;
bool serialOverflow = false;
bool demoChannels[3] = {false, false, false};
Settings settings;
Stability stableInputs[4];
CalibrationDraft drafts[3];
float rawTemperature = DEVICE_DISCONNECTED_C;
TrendPoint trend[TREND_SIZE];
unsigned trendHead = 0, trendCount = 0;

bool simulatedChannel(unsigned i) { return i > 0 && i < 4 && demoChannels[i - 1]; }
bool anyDemo() { return demoChannels[0] || demoChannels[1] || demoChannels[2]; }
uint64_t uptime() { return esp_timer_get_time() / 1000ULL; }
bool sessionActive(uint64_t now) { return sessionUntil > now; }
int channelIndex(const char* name) {
  for (unsigned i = 0; i < 3; ++i) if (strcmp(name, CHANNELS[i]) == 0) return i;
  return -1;
}
bool allowedReference(int index, float y) {
  return finite(y) && (index == 0 ? y >= 0 && y <= 14 : index == 1 ? y >= 0 && y <= 100 : y >= 0 && y <= 100000);
}
bool validCalibration(int i, const Calibration& c) {
  return c.version == 1 && validPoints(c.mv1, c.y1, c.mv2, c.y2) &&
    allowedReference(i, c.y1) && allowedReference(i, c.y2) &&
    (!c.limits || (allowedReference(i, c.low) && allowedReference(i, c.high) && c.low < c.high));
}
void reply(const char* text) { commandReply = text; Serial.println(text); }
bool saveSettings(const Settings& next) {
  if (!validSettings(next) || !storageReady || preferences.putBytes("ui_settings", &next, sizeof(next)) != sizeof(next)) {
    reply("ERRO: configuracao nao salva na memoria da placa"); return false;
  }
  settings = next;
  for (unsigned i = 0; i < 3; ++i) demoChannels[i] = (settings.demoMask & (1 << i)) != 0;
  nextDisplay = 0;
  return true;
}
void sendSettings() {
  String json = "{\"dark\":" + String(settings.dark) + ",\"layout\":" + String(settings.layout) +
    ",\"focus\":" + String(settings.focus) + ",\"rotation\":" + String(settings.rotation) +
    ",\"inverse\":" + String(settings.inverse) + ",\"contrast\":" + String(settings.contrast) +
    ",\"demo_mask\":" + String(settings.demoMask) + ",\"temperature_offset\":" + String(settings.temperatureOffset, 3) +
    ",\"oled_ready\":" + String(displayReady ? "true" : "false") + ",\"calibrations\":[";
  for (unsigned i = 0; i < 3; ++i) {
    if (i) json += ',';
    const auto& c = calibrations[i];
    json += "{\"configured\":" + String(validCalibration(i, c) ? "true" : "false") +
      ",\"verified\":" + String(c.verified ? "true" : "false") + ",\"first_captured\":" + String(drafts[i].first && uptime() - drafts[i].stamp <= 1800000ULL ? "true" : "false") +
      ",\"mv1\":" + String(c.mv1, 2) + ",\"ref1\":" + String(c.y1, 2) +
      ",\"mv2\":" + String(c.mv2, 2) + ",\"ref2\":" + String(c.y2, 2) + "}";
  }
  json += "]}";
  server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json", json);
}
bool persist(int i, const Calibration& c) {
  if (!storageReady || preferences.putBytes(CHANNELS[i], &c, sizeof(c)) != sizeof(c)) {
    reply("ERRO: configuracao nao salva; verifique NVS"); return false;
  }
  calibrations[i] = c;
  drafts[i] = CalibrationDraft{};
  readings[i + 1] = Reading{};
  outsideStreak = 0;
  // Não misturar médias calculadas com calibrações/faixas diferentes.
  historyHead = historyCount = 0; bucket = Minute{}; bucketStart = uptime(); trendHead = trendCount = 0;
  reply("Configuracao salva. Historico reiniciado."); return true;
}
float readMillivolts(uint8_t pin) {
  analogReadMilliVolts(pin); // descartar primeira conversão após troca de canal
  uint32_t sum = 0;
  for (unsigned i = 0; i < 16; ++i) { sum += analogReadMilliVolts(pin); delayMicroseconds(100); }
  return sum / 16.0f;
}
bool isCurrent(unsigned i, uint64_t now) {
  return current(now, readings[i].stamp, readings[i].hasValue) &&
    strcmp(readings[i].state, "atual") == 0 &&
    ((i != 1 && i != 3) || sessionActive(now));
}
const char* readingState(unsigned i, uint64_t now) {
  if (simulatedChannel(i) && readings[i].hasValue)
    return current(now, readings[i].stamp, true) ? "simulado" : "simulado_antigo";
  if ((i == 1 || i == 3) && !sessionActive(now))
    return readings[i].hasValue ? "ultima_amostra" : "sem_sessao";
  if (strcmp(readings[i].state, "atual") == 0 && !isCurrent(i, now)) return "leitura_antiga";
  return readings[i].state;
}
const char* assessment(uint64_t now) {
  if (anyDemo()) return "DEMONSTRAÇÃO: canais SIMULADOS usam potenciômetros. Histórico e relatório identificam esses valores; não são medições de água. Temperatura real, sem simulação.";
  for (unsigned i = 0; i < 4; ++i) if (!isCurrent(i, now)) return "Medição indisponível, antiga ou calibração pendente.";
  for (const auto& c : calibrations) if (!c.limits) return "Faixas experimentais pendentes; sem classificação.";
  if (outsideStreak >= 3) return "Alteração persistente detectada — verificar água, sensores e reservatório.";
  if (outsideStreak) return "Alteração em observação; aguardando persistência.";
  return "Sem alteração nos parâmetros medidos — potabilidade não confirmada.";
}
void updateAssessment(uint64_t now) {
  bool outside = false;
  for (unsigned i = 0; i < 4; ++i) if (!isCurrent(i, now)) { outsideStreak = 0; return; }
  for (unsigned i = 0; i < 3; ++i) {
    const auto& c = calibrations[i];
    if (!c.limits) { outsideStreak = 0; return; }
    outside |= !between(readings[i + 1].value, c.low, c.high);
  }
  outsideStreak = outside ? (outsideStreak < 3 ? outsideStreak + 1 : 3) : 0;
}
void closeMinute(uint64_t now) {
  if (now - bucketStart < 60000) return;
  bucket.stamp = now;
  history[historyHead] = bucket;
  historyHead = nextIndex(historyHead, HISTORY_SIZE);
  if (historyCount < HISTORY_SIZE) ++historyCount;
  bucket = Minute{}; bucketStart = now;
}
void collect(uint64_t now) {
  auto& temp = readings[0];
  rawTemperature = sensorFound ? thermometer.getTempC(temperatureAddress) : DEVICE_DISCONNECTED_C;
  const float t = rawTemperature == DEVICE_DISCONNECTED_C ? rawTemperature : rawTemperature + settings.temperatureOffset;
  if (finite(t) && t != DEVICE_DISCONNECTED_C && t >= -55 && t <= 125) {
    stableInputs[0].add(rawTemperature, now);
    temp.value = t; temp.hasValue = true; temp.stamp = now; temp.state = "atual";
  } else { temp.state = "sensor_indisponivel"; stableInputs[0] = Stability{}; }
  for (unsigned i = 0; i < 3; ++i) {
    auto& r = readings[i + 1];
    r.mv = readMillivolts(ANALOG_PINS[i]);
    stableInputs[i + 1].add(r.mv, now);
    if (demoChannels[i]) {
      uint32_t raw = 0;
      for (unsigned n = 0; n < 16; ++n) raw += analogRead(ANALOG_PINS[i]);
      const float maxima[] = {14.0f, 100.0f, 2000.0f};
      r.value = raw * (maxima[i] / (16.0f * 4095.0f));
      r.hasValue = true; r.stamp = now; r.state = "simulado";
      continue;
    }
    const auto& c = calibrations[i];
    if ((i == 0 || i == 2) && !sessionActive(now)) { r.state = "sem_sessao"; continue; }
    if (r.mv < 150 || r.mv > 2300) { r.state = "fora_faixa_adc"; continue; }
    if (!validCalibration(i, c) || !c.verified) { r.state = "calibracao_pendente"; continue; }
    if (!between(r.mv, c.mv1, c.mv2)) { r.state = "fora_faixa_calibrada"; continue; }
    // Sem coeficiente universal para o TDS: EC somente em ensaio a 25 ± 0,5 °C.
    if (i == 2 && (!isCurrent(0, now) || absValue(t - 25) > 0.5f)) {
      r.state = "ec_exige_25C"; continue;
    }
    r.value = interpolate(r.mv, c.mv1, c.y1, c.mv2, c.y2);
    r.hasValue = true; r.stamp = now; r.state = "atual";
  }
  updateAssessment(now);
  TrendPoint point; point.stamp = now;
  for (unsigned i = 0; i < 4; ++i) if (isCurrent(i, now) || (simulatedChannel(i) && current(now, readings[i].stamp, readings[i].hasValue))) {
    point.value[i] = readings[i].value; point.valid |= (1 << i);
    if (simulatedChannel(i)) point.simulated |= (1 << i);
    if (simulatedChannel(i)) bucket.simulatedMask |= (1 << i);
    bucket.sum[i] += readings[i].value; ++bucket.count[i];
  }
  trend[trendHead] = point; trendHead = (trendHead + 1) % TREND_SIZE;
  if (trendCount < TREND_SIZE) ++trendCount;
  Serial.printf("uptime=%llu s | ", (unsigned long long)(now / 1000));
  for (unsigned i = 0; i < 4; ++i) {
    Serial.printf("%u:%s valor=", i, readingState(i, now));
    if (readings[i].hasValue) Serial.print(readings[i].value, 2); else Serial.print("--");
    if (i) Serial.printf(" ADC=%.1fmV", readings[i].mv);
    Serial.print(" | ");
  }
  Serial.println();
}
String numberOrNull(float value, bool valid) { return valid && finite(value) ? String(value, 2) : String("null"); }
void sendReadings() {
  const uint64_t now = uptime();
  String json; json.reserve(1500);
  json = "{\"uptime_ms\":" + String((double)now, 0) + ",\"session_remaining_s\":";
  json += String((unsigned long)(sessionActive(now) ? (sessionUntil - now) / 1000 : 0));
  json += ",\"assessment\":\""; json += assessment(now); json += "\",\"readings\":[";
  for (unsigned i = 0; i < 4; ++i) {
    const auto& r = readings[i]; if (i) json += ',';
    json += "{\"value\":" + numberOrNull(r.value, r.hasValue);
    json += ",\"simulated\":"; json += (simulatedChannel(i)) ? "true" : "false";
    json += ",\"stable\":"; json += stableInputs[i].stable(now, i == 0 ? 0.3f : 10.0f) ? "true" : "false";
    json += ",\"stability_samples\":" + String(stableInputs[i].count);
    json += ",\"stability_span\":" + String(stableInputs[i].spread(), 2);
    json += ",\"mv\":" + String(r.mv, 1) + ",\"age_ms\":";
    json += r.hasValue ? String((double)(now - r.stamp), 0) : String("null");
    json += ",\"state\":\""; json += readingState(i, now); json += "\"}";
  }
  json += "]}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}
void sendHistory() {
  const uint64_t now = uptime();
  server.sendHeader("Cache-Control", "no-store");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", ""); server.sendContent("[");
  bool first = true;
  for (unsigned n = 0; n < historyCount; ++n) {
    const auto& row = history[(historyHead + HISTORY_SIZE - historyCount + n) % HISTORY_SIZE];
    if (now - row.stamp > 86400000ULL) continue;
    String item = first ? "" : ","; first = false;
    item += "{\"age_ms\":" + String((double)(now - row.stamp), 0) + ",\"mean\":[";
    for (unsigned i = 0; i < 4; ++i) { if (i) item += ','; item += numberOrNull(row.count[i] ? row.sum[i] / row.count[i] : 0, row.count[i] != 0); }
    item += "],\"count\":[";
    for (unsigned i = 0; i < 4; ++i) { if (i) item += ','; item += String(row.count[i]); }
    item += "],\"simulated_mask\":" + String(row.simulatedMask) + "}"; server.sendContent(item); yield();
    if (!server.client().connected()) break;
  }
  server.sendContent("]"); server.sendContent("");
}
void sendTrend() {
  const uint64_t now = uptime();
  String json; json.reserve(6500); json = "[";
  for (unsigned n = 0; n < trendCount; ++n) {
    const auto& p = trend[(trendHead + TREND_SIZE - trendCount + n) % TREND_SIZE];
    if (n) json += ',';
    json += "{\"age_ms\":" + String((double)(now - p.stamp), 0) + ",\"values\":[";
    for (unsigned i = 0; i < 4; ++i) { if (i) json += ','; json += numberOrNull(p.value[i], p.valid & (1 << i)); }
    json += "],\"simulated_mask\":" + String(p.simulated) + "}";
  }
  json += ']'; server.sendHeader("Cache-Control", "no-store"); server.send(200, "application/json", json);
}
void drawDisplay(uint64_t now) {
  if (!displayReady || now < nextDisplay) return;
  nextDisplay = now + 1000;
  display.setRotation(settings.rotation); display.invertDisplay(settings.inverse);
  display.ssd1306_command(SSD1306_SETCONTRAST); display.ssd1306_command(settings.contrast);
  display.clearDisplay(); display.setTextSize(1); display.setCursor(0, 0);
  display.println("WATER TANK"); display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
  const char* labels[] = {"Temp C", "pH", "Turb %", "EC uS/cm"};
  auto value = [&](unsigned i) -> String {
    return (isCurrent(i, now) || (simulatedChannel(i) && current(now, readings[i].stamp, readings[i].hasValue))) ? String(readings[i].value, i == 1 ? 2 : 1) : String("--");
  };
  if (settings.layout == 3) {
    unsigned i = settings.focus;
    display.setCursor(0, 13); display.print(labels[i]); display.setCursor(70, 13); display.print(value(i));
    bool found = false; float lo = 0, hi = 0;
    for (unsigned n = 0; n < trendCount; ++n) {
      const auto& p = trend[(trendHead + TREND_SIZE - trendCount + n) % TREND_SIZE];
      if (!(p.valid & (1 << i))) continue;
      if (!found) { lo = hi = p.value[i]; found = true; }
      if (p.value[i] < lo) lo = p.value[i]; if (p.value[i] > hi) hi = p.value[i];
    }
    if (!found) { display.setCursor(0, 32); display.print("Aguardando leituras"); }
    else {
      display.setCursor(0, 25); display.print(hi, absValue(hi) >= 100 ? 0 : 1);
      display.setCursor(0, 44); display.print(lo, absValue(lo) >= 100 ? 0 : 1);
      display.drawLine(34, 24, 34, 52, SSD1306_WHITE); display.drawLine(34, 52, 127, 52, SSD1306_WHITE);
      const auto& first = trend[(trendHead + TREND_SIZE - trendCount) % TREND_SIZE];
      const auto& last = trend[(trendHead + TREND_SIZE - 1) % TREND_SIZE];
      bool connected = false; int px = 0, py = 0; uint64_t prior = 0;
      for (unsigned n = 0; n < trendCount; ++n) {
        const auto& p = trend[(trendHead + TREND_SIZE - trendCount + n) % TREND_SIZE];
        if (!(p.valid & (1 << i))) { connected = false; continue; }
        int x = 35 + (last.stamp > first.stamp ? (p.stamp - first.stamp) * 91 / (last.stamp - first.stamp) : 45);
        int y = hi > lo ? 50 - (p.value[i] - lo) * 24 / (hi - lo) : 38;
        if (connected && p.stamp - prior <= 15000) display.drawLine(px, py, x, y, SSD1306_WHITE);
        else display.drawPixel(x, y, SSD1306_WHITE);
        connected = true; px = x; py = y; prior = p.stamp;
      }
    }
  } else if (settings.layout == 2) {
    unsigned i = settings.focus;
    display.setCursor(0, 15); display.print(labels[i]); if (simulatedChannel(i)) display.print(" SIM");
    display.setCursor(0, 29); display.setTextSize(2); display.println(value(i)); display.setTextSize(1);
  } else if (settings.layout == 1) {
    for (unsigned i = 0; i < 4; ++i) {
      int x = (i % 2) * 65, y = 14 + (i / 2) * 20;
      display.setCursor(x, y); display.print(labels[i]); if (simulatedChannel(i)) display.print("*");
      display.setCursor(x, y + 9); display.print(value(i));
    }
  } else {
    for (unsigned i = 0; i < 4; ++i) {
      display.setCursor(0, 14 + i * 10); display.print(labels[i]);
      display.setCursor(62, 14 + i * 10); display.print(value(i)); if (simulatedChannel(i)) display.print("*");
    }
  }
  display.setCursor(0, 56); display.print(anyDemo() ? "*SIM: nao mede agua" : "Potab. nao confirmada");
  display.display();
}
void help() {
  Serial.println("Comandos (115200 baud, nova linha):");
  Serial.println("ajuda | rede | amostra <30..1800 segundos> | parar");
  Serial.println("demo <ph|turb|ec|todos> <on|off> (persistente)");
  Serial.println("Somente USB: senha <nova senha> | display");
  Serial.println("cal <ph|turb|ec> <mV1> <referencia1> <mV2> <referencia2>");
  Serial.println("validar <ph|turb|ec> (somente apos verificar referencia independente)");
  Serial.println("faixa <ph|turb|ec> <min> <max> (limites experimentais, nao sanitarios)");
  Serial.println("A calibracao usa mV NO GPIO, apos divisor. EC: validar a 25 +/-0,5 C.");
}
void command(const String& line) {
  char name[12] = {}; float a, b, c, d; unsigned seconds; int end = 0;
  if (line.startsWith("ui ")) {
    int dark, layout, focus, rotation, inverse, contrast;
    if (sscanf(line.c_str(), "ui %d %d %d %d %d %d %n", &dark, &layout, &focus, &rotation, &inverse, &contrast, &end) != 6 ||
        end != (int)line.length() || dark < 0 || dark > 1 || layout < 0 || layout > 3 || focus < 0 || focus > 3 ||
        (rotation != 0 && rotation != 2) || inverse < 0 || inverse > 1 || contrast < 32 || contrast > 255) { reply("ERRO: opcoes de tela invalidas"); return; }
    Settings next = settings; next.dark = dark; next.layout = layout; next.focus = focus;
    next.rotation = rotation; next.inverse = inverse; next.contrast = contrast;
    if (saveSettings(next)) reply("Tema e OLED salvos. Permanecem apos reiniciar."); return;
  }
  if (line.startsWith("preparar ")) {
    if (sscanf(line.c_str(), "preparar %11s %n", name, &end) != 1 || end != (int)line.length()) { reply("ERRO: canal invalido"); return; }
    if (strcmp(name, "temp") == 0) { stableInputs[0] = Stability{}; reply("Aguardando temperatura estavel."); return; }
    int i = channelIndex(name);
    if (i < 0 || demoChannels[i]) { reply("ERRO: desligue a simulacao do canal antes de calibrar"); return; }
    stableInputs[i + 1] = Stability{};
    reply("Aguardando quatro leituras estaveis. Coloque a referencia e aguarde pelo menos 20 segundos."); return;
  }
  if (line.startsWith("capturar ")) {
    unsigned point;
    if (sscanf(line.c_str(), "capturar %11s %u %f %n", name, &point, &a, &end) != 3 || end != (int)line.length()) { reply("ERRO: captura invalida"); return; }
    int i = channelIndex(name);
    if (i < 0 || demoChannels[i] || !allowedReference(i, a) || (point != 1 && point != 2)) { reply("ERRO: canal, referencia ou modo invalido"); return; }
    if (!stableInputs[i + 1].stable(uptime(), 10)) { reply("ERRO: aguarde quatro leituras estaveis (variacao ate 10 mV)"); return; }
    if (i == 2 && (!isCurrent(0, uptime()) || absValue(readings[0].value - 25) > 0.5f)) { reply("ERRO: EC exige temperatura real de 25 +/-0.5 C"); return; }
    float mv = stableInputs[i + 1].mean();
    if (!between(mv, 150, 2300)) { reply("ERRO: ADC fora de 150..2300 mV; confira divisor e multimetro"); return; }
    if (point == 1) {
      drafts[i].first = true; drafts[i].mv = mv; drafts[i].reference = a; drafts[i].stamp = uptime();
      stableInputs[i + 1] = Stability{};
      reply("Primeiro ponto capturado. Troque a referencia e prepare o segundo ponto."); return;
    }
    if (!drafts[i].first || uptime() - drafts[i].stamp > 1800000ULL) { reply("ERRO: capture novamente o primeiro ponto (validade 30 min)"); return; }
    Calibration next; next.mv1 = drafts[i].mv; next.y1 = drafts[i].reference; next.mv2 = mv; next.y2 = a;
    if (!validCalibration(i, next)) { reply("ERRO: referencias devem ser distintas e sinais separados por pelo menos 20 mV"); return; }
    if (persist(i, next)) { drafts[i] = CalibrationDraft{}; stableInputs[i + 1] = Stability{}; reply("Curva salva, pendente de verificacao. Use uma terceira referencia independente."); } return;
  }
  if (line.startsWith("verificar ")) {
    if (sscanf(line.c_str(), "verificar %11s %f %f %n", name, &a, &b, &end) != 3 || end != (int)line.length()) { reply("ERRO: verificacao invalida"); return; }
    int i = channelIndex(name);
    if (i < 0 || demoChannels[i] || !validCalibration(i, calibrations[i]) || !stableInputs[i + 1].stable(uptime(), 10)) { reply("ERRO: canal real, curva e sinal estavel necessarios"); return; }
    const auto& cal = calibrations[i]; float mv = stableInputs[i + 1].mean();
    if (!finite(b) || b <= 0 || b > absValue(cal.y2 - cal.y1) * 0.2f || !between(a, cal.y1, cal.y2) ||
        absValue(a - cal.y1) < 0.0001f || absValue(a - cal.y2) < 0.0001f || !between(mv, cal.mv1, cal.mv2)) { reply("ERRO: referencia intermediaria e tolerancia positiva ate 20% do intervalo exigidas"); return; }
    if (i == 2 && (!isCurrent(0, uptime()) || absValue(readings[0].value - 25) > 0.5f)) { reply("ERRO: EC exige 25 +/-0.5 C"); return; }
    float error = absValue(interpolate(mv, cal.mv1, cal.y1, cal.mv2, cal.y2) - a);
    if (error > b) { reply("ERRO: resultado fora da tolerancia. Revise referencias, curva e montagem."); return; }
    auto next = cal; next.verified = true;
    if (persist(i, next)) reply("Verificacao dentro da tolerancia informada. Curva habilitada."); return;
  }
  if (line.startsWith("tempcal ")) {
    if (sscanf(line.c_str(), "tempcal %f %n", &a, &end) != 1 || end != (int)line.length() || !between(a, -10, 85) ||
        !isCurrent(0, uptime()) || !stableInputs[0].stable(uptime(), 0.3f)) { reply("ERRO: temperatura real estavel e referencia -10..85 C necessarias"); return; }
    Settings next = settings; next.temperatureOffset = a - stableInputs[0].mean();
    if (absValue(next.temperatureOffset) > 5) { reply("ERRO: correcao superior a 5 C; confira sensor e referencia"); return; }
    if (saveSettings(next)) {
      readings[0] = Reading{}; historyHead = historyCount = 0; bucket = Minute{}; bucketStart = uptime(); trendHead = trendCount = 0;
      reply("Correcao de temperatura salva. Confira em outra temperatura com termometro de referencia.");
    } return;
  }
  if (line.startsWith("demo ")) {
    char channel[12] = {}, mode[4] = {}; int consumed = 0;
    if (sscanf(line.c_str(), "demo %11s %3s %n", channel, mode, &consumed) != 2 || consumed != (int)line.length() ||
        (strcmp(mode, "on") != 0 && strcmp(mode, "off") != 0)) { reply("ERRO: demo <ph|turb|ec|todos> <on|off>"); return; }
    const int selected = channelIndex(channel);
    const bool all = strcmp(channel, "todos") == 0;
    if (!all && selected < 0) { reply("ERRO: canal de demonstracao invalido"); return; }
    Settings next = settings;
    for (unsigned i = 0; i < 3; ++i) if (all || (int)i == selected) {
      if (strcmp(mode, "on") == 0) next.demoMask |= (1 << i); else next.demoMask &= ~(1 << i);
    }
    if (!saveSettings(next)) return;
    for (unsigned i = 0; i < 3; ++i) if (all || (int)i == selected) {
      readings[i + 1] = Reading{}; stableInputs[i + 1] = Stability{}; drafts[i] = CalibrationDraft{};
    }
    outsideStreak = 0;
    historyHead = historyCount = 0; bucket = Minute{}; bucketStart = uptime(); trendHead = trendCount = 0;
    nextSample = 0; nextDisplay = 0;
    reply("Demo atualizada. Nao mede agua. Calibracao preservada; historico reiniciado.");
    return;
  }
  if (line.startsWith("senha ")) {
    String next = line.substring(6);
    if (next.length() < 8 || next.length() > 63) { reply("ERRO: senha deve ter 8..63 caracteres ASCII"); return; }
    for (unsigned i = 0; i < next.length(); ++i) {
      if (next[i] < 32 || next[i] > 126) { reply("ERRO: use caracteres ASCII imprimiveis"); return; }
    }
    if (!storageReady || preferences.putString("ap_password", next) == 0) { reply("ERRO: senha nao persistida"); return; }
    reply("Senha salva. Reiniciando a placa.");
    delay(200); ESP.restart(); return;
  }
  if (line == "display") {
    Serial.println("I2C: SDA=21 SCL=22");
    for (uint8_t address = 1; address < 127; ++address) {
      Wire.beginTransmission(address);
      if (Wire.endTransmission() == 0) Serial.printf("I2C encontrado: 0x%02X\n", address);
    }
    Serial.println(displayReady ? "OLED inicializado" : "OLED indisponivel; confira conexoes e reinicie");
    return;
  }
  if (line == "ajuda") { help(); return; }
  if (line == "rede") {
    Serial.println("SSID: WaterTankMonitor"); Serial.println("Senha: " + apPassword);
    Serial.print("IP: "); Serial.println(WiFi.softAPIP()); return;
  }
  if (line == "parar") { sessionUntil = 0; outsideStreak = 0; reply("Sessao encerrada"); return; }
  if (sscanf(line.c_str(), "amostra %u %n", &seconds, &end) == 1 && end == (int)line.length() && seconds >= 30 && seconds <= 1800) {
    sessionUntil = uptime() + seconds * 1000ULL; outsideStreak = 0;
    // Impedir que a nova sessão faça uma leitura anterior parecer atual.
    readings[1].state = readings[3].state = "aguardando_nova_amostra";
    reply("Sessao iniciada. Sondas devem estar estabilizadas."); return;
  }
  end = 0;
  if (sscanf(line.c_str(), "cal %11s %f %f %f %f %n", name, &a, &b, &c, &d, &end) == 5 && end == (int)line.length()) {
    int i = channelIndex(name); Calibration next;
    if (i >= 0 && demoChannels[i]) { reply("ERRO: desligue a simulacao antes de calibrar o sensor real"); return; }
    next.mv1 = a; next.y1 = b; next.mv2 = c; next.y2 = d;
    if (i >= 0 && validCalibration(i, next)) persist(i, next);
    else reply("ERRO: pontos invalidos; mV 150..2300, separacao >=20 mV");
    return;
  }
  end = 0;
  if (sscanf(line.c_str(), "validar %11s %n", name, &end) == 1 && end == (int)line.length()) {
    int i = channelIndex(name);
    if (i >= 0 && demoChannels[i]) { reply("ERRO: nao validar calibracao com potenciometro simulado"); return; }
    if (i >= 0 && validCalibration(i, calibrations[i])) { auto next = calibrations[i]; next.verified = true; persist(i, next); }
    else reply("ERRO: definir calibracao antes de validar"); return;
  }
  end = 0;
  if (sscanf(line.c_str(), "faixa %11s %f %f %n", name, &a, &b, &end) == 3 && end == (int)line.length()) {
    int i = channelIndex(name);
    if (i >= 0 && allowedReference(i, a) && allowedReference(i, b) && a < b && validCalibration(i, calibrations[i])) {
      auto next = calibrations[i]; next.low = a; next.high = b; next.limits = true; persist(i, next);
    } else reply("ERRO: faixa ou calibracao invalida"); return;
  }
  reply("Comando invalido. Digite ajuda.");
}
void readCommands() {
  unsigned budget = 64;
  while (Serial.available() && budget--) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (serialOverflow) Serial.println("ERRO: comando muito longo");
      else { serialLine.trim(); if (serialLine.length()) command(serialLine); }
      serialLine = ""; serialOverflow = false;
    } else if (serialLine.length() < 128 && !serialOverflow) serialLine += c;
    else serialOverflow = true;
  }
}
void setup() {
  Serial.begin(115200); serialLine.reserve(128);
  storageReady = preferences.begin("watertank", false);
  Settings loadedSettings;
  if (storageReady && preferences.isKey("ui_settings") && preferences.getBytesLength("ui_settings") == sizeof(loadedSettings) &&
      preferences.getBytes("ui_settings", &loadedSettings, sizeof(loadedSettings)) == sizeof(loadedSettings) && validSettings(loadedSettings)) settings = loadedSettings;
  for (unsigned i = 0; i < 3; ++i) demoChannels[i] = (settings.demoMask & (1 << i)) != 0;
  for (int i = 0; i < 3; ++i) {
    Calibration loaded;
    if (storageReady && preferences.isKey(CHANNELS[i]) && preferences.getBytesLength(CHANNELS[i]) == sizeof(loaded) &&
        preferences.getBytes(CHANNELS[i], &loaded, sizeof(loaded)) == sizeof(loaded) && validCalibration(i, loaded)) calibrations[i] = loaded;
  }
  if (storageReady) apPassword = preferences.getString("ap_password", "");
  if (apPassword.length() < 8 || apPassword.length() > 63) {
    char generated[17]; snprintf(generated, sizeof(generated), "%08lx%08lx", (unsigned long)esp_random(), (unsigned long)esp_random());
    apPassword = generated;
    if (storageReady && preferences.putString("ap_password", apPassword) == 0) Serial.println("ERRO: senha nao persistida");
  }
  if (!storageReady) Serial.println("ERRO NVS: calibracao indisponivel e senha temporaria");
  Wire.begin(21, 22); Wire.setTimeOut(50);
  Wire.beginTransmission(0x3C);
  displayReady = Wire.endTransmission() == 0 && display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (displayReady) { display.setTextSize(1); display.setTextColor(SSD1306_WHITE); display.setTextWrap(false); }
  else Serial.println("OLED indisponivel; aquisicao e web continuam");
  analogReadResolution(12); analogSetAttenuation(ADC_11db);
  thermometer.begin(); thermometer.setWaitForConversion(false);
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP("WaterTankMonitor", apPassword.c_str())) Serial.println("ERRO: Wi-Fi indisponivel");
  server.on("/", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "text/html; charset=utf-8", PANEL); });
  server.on("/report.js", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "application/javascript; charset=utf-8", REPORT_SCRIPT); });
  server.on("/interactions.js", HTTP_GET, []() { server.sendHeader("Cache-Control", "no-store"); server.send_P(200, "application/javascript; charset=utf-8", INTERACTIONS_SCRIPT); });
  server.on("/api/readings", HTTP_GET, sendReadings);
  server.on("/api/history", HTTP_GET, sendHistory);
  server.on("/api/trend", HTTP_GET, sendTrend);
  server.on("/api/settings", HTTP_GET, sendSettings);
  const char* headers[] = {"X-WaterTank"};
  server.collectHeaders(headers, 1);
  server.on("/api/command", HTTP_POST, []() {
    if (server.header("X-WaterTank") != "1" || server.arg("senha") != apPassword) {
      server.send(403, "text/plain; charset=utf-8", "Senha ou origem inválida."); return;
    }
    String line = server.arg("comando"); line.trim();
    if (line.length() > 128 || line.indexOf('\n') >= 0 || line.indexOf('\r') >= 0 ||
        line == "rede" || line == "ajuda" || line == "display" || line.startsWith("senha ")) {
      server.send(400, "text/plain", "Comando invalido para o painel."); return;
    }
    commandReply = ""; command(line);
    server.send(commandReply.startsWith("ERRO:") ? 400 : 200, "text/plain; charset=utf-8", commandReply);
  });
  server.onNotFound([]() { server.send(404, "text/plain", "Nao encontrado"); });
  server.begin(); bucketStart = uptime(); help(); command("rede");
}
void loop() {
  server.handleClient(); readCommands();
  const uint64_t now = uptime(); closeMinute(now);
  if (!converting && now >= nextSample) {
    sensorFound = thermometer.getAddress(temperatureAddress, 0);
    if (sensorFound) { thermometer.setResolution(temperatureAddress, 12); thermometer.requestTemperatures(); }
    converting = true; conversionReady = now + 800; nextSample = now + SAMPLE_PERIOD;
  }
  if (converting && now >= conversionReady) { converting = false; collect(now); }
  drawDisplay(now); delay(1);
}
