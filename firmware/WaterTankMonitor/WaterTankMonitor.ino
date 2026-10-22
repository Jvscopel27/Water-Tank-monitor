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
bool persist(int i, const Calibration& c) {
  if (!storageReady || preferences.putBytes(CHANNELS[i], &c, sizeof(c)) != sizeof(c)) {
    reply("ERRO: configuracao nao salva; verifique NVS"); return false;
  }
  calibrations[i] = c;
  readings[i + 1] = Reading{};
  outsideStreak = 0;
  // Não misturar médias calculadas com calibrações/faixas diferentes.
  historyHead = historyCount = 0; bucket = Minute{}; bucketStart = uptime();
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
  if ((i == 1 || i == 3) && !sessionActive(now))
    return readings[i].hasValue ? "ultima_amostra" : "sem_sessao";
  if (strcmp(readings[i].state, "atual") == 0 && !isCurrent(i, now)) return "leitura_antiga";
  return readings[i].state;
}
const char* assessment(uint64_t now) {
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
  const float t = sensorFound ? thermometer.getTempC(temperatureAddress) : DEVICE_DISCONNECTED_C;
  if (finite(t) && t != DEVICE_DISCONNECTED_C && t >= -55 && t <= 125) {
    temp.value = t; temp.hasValue = true; temp.stamp = now; temp.state = "atual";
  } else temp.state = "sensor_indisponivel";
  for (unsigned i = 0; i < 3; ++i) {
    auto& r = readings[i + 1];
    r.mv = readMillivolts(ANALOG_PINS[i]);
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
  for (unsigned i = 0; i < 4; ++i) if (isCurrent(i, now)) {
    bucket.sum[i] += readings[i].value; ++bucket.count[i];
  }
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
    item += "]}"; server.sendContent(item); yield();
    if (!server.client().connected()) break;
  }
  server.sendContent("]"); server.sendContent("");
}
void drawDisplay(uint64_t now) {
  if (!displayReady || now < nextDisplay) return;
  nextDisplay = now + 5000;
  display.clearDisplay(); display.setCursor(0, 0);
  if ((displayPage++ % 2) == 0) {
    display.println("WATER TANK | BANCADA");
    const char* names[] = {"T C: ", "pH: ", "Turb %: ", "EC uS/cm: "};
    for (unsigned i = 0; i < 4; ++i) {
      display.print(names[i]);
      if (isCurrent(i, now)) display.println(readings[i].value, 1); else display.println("--");
    }
    display.println(sessionActive(now) ? "Sessao ativa" : "Sem sessao pH/EC");
    display.println("Potab. nao confirmada");
  } else {
    display.println("IDADE / ESTADO");
    const char* names[] = {"T ", "pH ", "Tu ", "EC "};
    for (unsigned i = 0; i < 4; ++i) {
      display.print(names[i]);
      if (!readings[i].hasValue) display.println("indisp./cal.pendente");
      else {
        display.print(isCurrent(i, now) ? "atual " : "ultima ");
        display.print((unsigned long)((now - readings[i].stamp) / 1000)); display.println("s");
      }
    }
    display.println(WiFi.softAPIP());
    display.println(outsideStreak >= 3 ? "ALTERACAO DETECTADA" : "Estado detalhado: web");
  }
  display.display();
}
void help() {
  Serial.println("Comandos (115200 baud, nova linha):");
  Serial.println("ajuda | rede | amostra <30..1800 segundos> | parar");
  Serial.println("cal <ph|turb|ec> <mV1> <referencia1> <mV2> <referencia2>");
  Serial.println("validar <ph|turb|ec> (somente apos verificar referencia independente)");
  Serial.println("faixa <ph|turb|ec> <min> <max> (limites experimentais, nao sanitarios)");
  Serial.println("A calibracao usa mV NO GPIO, apos divisor. EC: validar a 25 +/-0,5 C.");
}
void command(const String& line) {
  char name[12] = {}; float a, b, c, d; unsigned seconds; int end = 0;
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
    next.mv1 = a; next.y1 = b; next.mv2 = c; next.y2 = d;
    if (i >= 0 && validCalibration(i, next)) persist(i, next);
    else reply("ERRO: pontos invalidos; mV 150..2300, separacao >=20 mV");
    return;
  }
  end = 0;
  if (sscanf(line.c_str(), "validar %11s %n", name, &end) == 1 && end == (int)line.length()) {
    int i = channelIndex(name);
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
  for (int i = 0; i < 3; ++i) {
    Calibration loaded;
    if (storageReady && preferences.getBytesLength(CHANNELS[i]) == sizeof(loaded) &&
        preferences.getBytes(CHANNELS[i], &loaded, sizeof(loaded)) == sizeof(loaded) && validCalibration(i, loaded)) calibrations[i] = loaded;
  }
  if (storageReady) apPassword = preferences.getString("ap_password", "");
  if (apPassword.length() < 12 || apPassword.length() > 63) {
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
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html; charset=utf-8", PANEL); });
  server.on("/api/readings", HTTP_GET, sendReadings);
  server.on("/api/history", HTTP_GET, sendHistory);
  const char* headers[] = {"X-WaterTank"};
  server.collectHeaders(headers, 1);
  server.on("/api/command", HTTP_POST, []() {
    if (server.header("X-WaterTank") != "1" || server.arg("senha") != apPassword) {
      server.send(403, "text/plain; charset=utf-8", "Senha ou origem inválida."); return;
    }
    String line = server.arg("comando"); line.trim();
    if (line.length() > 128 || line.indexOf('\n') >= 0 || line.indexOf('\r') >= 0 ||
        line == "rede" || line == "ajuda") {
      server.send(400, "text/plain", "Comando invalido para o painel."); return;
    }
    commandReply = ""; command(line);
    server.send(200, "text/plain; charset=utf-8", commandReply);
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
