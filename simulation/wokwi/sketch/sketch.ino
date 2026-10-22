// Water Tank Monitor - demonstracao EXCLUSIVA para o Wokwi.
// Os potenciometros representam grandezas simuladas, nao sondas reais.
// Nao gravar este codigo no prototipo esperando leituras calibradas.
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

constexpr uint8_t PIN_TURBIDITY = 34;
constexpr uint8_t PIN_PH = 35;
constexpr uint8_t PIN_EC = 32;
constexpr uint8_t PIN_TEMPERATURE = 4;
constexpr uint8_t PIN_FAILURE = 27;
constexpr uint8_t PIN_SDA = 21;
constexpr uint8_t PIN_SCL = 22;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
OneWire oneWire(PIN_TEMPERATURE);
DallasTemperature thermometer(&oneWire);
bool displayReady = false;

float readFraction(uint8_t pin) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < 16; ++i) {
    sum += analogRead(pin);
    delayMicroseconds(100);
  }
  return sum / (16.0f * 4095.0f);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_FAILURE, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  Wire.begin(PIN_SDA, PIN_SCL);
  displayReady = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  thermometer.begin();
  thermometer.setResolution(10);
  Serial.println("WATER TANK MONITOR - SIMULACAO");
  Serial.println("Controles: turbidez relativa, pH e EC sinteticos.");
  Serial.println("DS18B20: clique no sensor para alterar a temperatura.");
  Serial.println("Botao vermelho: mantenha pressionado para simular falha.");
  Serial.println("Estas leituras nao confirmam potabilidade.");
  if (!displayReady) Serial.println("ERRO: display indisponivel");
  else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.setCursor(0, 0);
    display.println("WATER TANK MONITOR");
    display.println("SIMULACAO WOKWI");
    display.display();
    Serial.println("OLED_OK");
  }
}

void loop() {
  const uint32_t started = millis();
  const bool forcedFailure = digitalRead(PIN_FAILURE) == LOW;
  thermometer.requestTemperatures();
  const float temperature = thermometer.getTempCByIndex(0);
  const bool valid = !forcedFailure && temperature != DEVICE_DISCONNECTED_C;
  const float ph = readFraction(PIN_PH) * 14.0f;
  const float turbidity = readFraction(PIN_TURBIDITY) * 100.0f;
  const float ec = readFraction(PIN_EC) * 2000.0f;

  if (displayReady) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WATER TANK | SIMULADO");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 14);
    if (valid) {
      display.print("Temp: "); display.print(temperature, 1); display.println(" C");
      display.print("pH:   "); display.println(ph, 2);
      display.print("Turb: "); display.print(turbidity, 1); display.println(" % rel");
      display.print("EC:   "); display.print(ec, 0); display.println(" uS/cm");
      display.println("Leitura atualizada");
    } else {
      display.println("MEDICAO INDISPONIVEL");
      display.println();
      display.println(forcedFailure ? "Falha simulada" : "DS18B20 desconectado");
      display.println("Verifique o sensor");
    }
    display.setCursor(0, 56);
    display.print("Sem laudo potabilidade");
    display.display();
  }

  if (valid) {
    Serial.printf("LEITURA_OK t=%.2f ph=%.2f turb_rel=%.1f ec_sim=%.0f tempo_s=%lu\n",
                  temperature, ph, turbidity, ec, millis() / 1000UL);
  } else {
    Serial.printf("MEDICAO_INDISPONIVEL motivo=%s\n",
                  forcedFailure ? "falha_simulada" : "sensor_temperatura");
  }
  const uint32_t elapsed = millis() - started;
  if (elapsed < 1000) delay(1000 - elapsed);
}
