// Teste isolado: nao calcula pH nem grava calibracoes/NVS.
#include <Arduino.h>
#include "DiagnosticStats.h"
using namespace phdiag;
constexpr uint8_t PH_PIN = 35; // ADC1; GPIO34 pertence a turbidez no projeto.
constexpr uint32_t PERIOD_MS = 500;
Window samples;
Snapshot saved[3];
bool hasSaved[3] = {};
char active = 'A';
uint32_t lastSample = 0, started = 0;
uint16_t stageMinRaw = 4095, stageMaxRaw = 0;
uint32_t stageMinMv = UINT32_MAX, stageMaxMv = 0;
String input;
bool overflow = false;

void help() {
  Serial.println("\nDIAGNOSTICO pH / GPIO35 / 12 bits / ADC_11db / 500 ms");
  Serial.println("Alimentacao experimental: 3V3. Nao ligar Po direto com modulo em 5V.");
  Serial.println("ADC nao verifica sobretensao nem confirma alimentacao adequada.");
  Serial.println("a = agua | b = agua com pouco vinagre | c = acompanhar offset");
  Serial.println("Cada etapa reinicia janela/extremos; capturas anteriores ficam em RAM.");
  Serial.println("salvar = guardar ultimas 10 leituras da etapa (substitui captura anterior)");
  Serial.println("comparar = resumo A/B/C | limpar = apagar capturas | ajuda");
  Serial.println("Comandos com Nova linha, 115200 baud. Salvar so apos estabilizacao fisica.");
  Serial.println("Nao produz pH calibrado. Agua/vinagre nao sao padroes de calibracao.");
}
void beginStage(char stage) {
  active = stage; samples = Window{}; started = millis();
  stageMinRaw = 4095; stageMaxRaw = 0;
  stageMinMv = UINT32_MAX; stageMaxMv = 0;
  Serial.printf("\nETAPA %c iniciada. Captura salva anterior desta etapa foi preservada.\n", active);
  if (stage == 'C') Serial.println("Use a mesma solucao de B. Identifique o trimpot de OFFSET perto do BNC; nao por 'inferior'.");
}
void comparePair(unsigned a, unsigned b) {
  if (!hasSaved[a] || !hasSaved[b]) {
    Serial.printf("%c->%c: faltam capturas.\n", 'A'+a, 'A'+b); return;
  }
  const Comparison c = compare(saved[a], saved[b]);
  Serial.printf("%c->%c: delta=%+.1f mV; limiar heuristico=%.1f mV. ", 'A'+a, 'A'+b, c.deltaMv, c.thresholdMv);
  if (!c.valid) Serial.println("INCONCLUSIVO: janela instavel ou ADC perto dos limites.");
  else if (c.significant) Serial.println("Variacao significativa pelo criterio do teste; nao comprova exatidao de pH.");
  else Serial.println("Sem variacao significativa: pode haver pouca diferenca, falta de equilibrio, offset, sonda/ligacao ou alimentacao inadequada.");
}
void report() {
  Serial.println("\nCAPTURAS: medias das ultimas 10 leituras no instante de salvar.");
  for (unsigned i=0; i<3; ++i) {
    if (!hasSaved[i]) { Serial.printf("%c: nao salva\n", 'A'+i); continue; }
    const auto& s = saved[i];
    Serial.printf("%c: ADC=%.1f [%u..%u] | mV=%.1f [%lu..%lu] | varFS=%.3f%% | baixo/alto=%u/%u | estavel=%s\n",
      'A'+i, s.rawMean, s.rawMin, s.rawMax, s.mvMean,
      (unsigned long)s.mvMin, (unsigned long)s.mvMax, s.variationPercent(), s.low, s.high, s.constant()?"sim":"nao");
  }
  comparePair(0,1); comparePair(1,2);
  Serial.println("A/B compara solucoes somente se offset e montagem ficaram iguais.");
  Serial.println("B/C compara offset somente se a solucao ficou igual. Nao equivale a calibracao.");
  Serial.println("Hipotese eletrica: PH4502C usual requer 5V; em 3V3 pode operar inadequadamente. ADC nao confirma a causa.");
  Serial.println("Hipotese de conversao: sinal responsivo com pH travado pede revisar modo SIM/curva. No projeto, ADC maximo em SIM produz 14.");
}
void command(String line) {
  line.trim(); line.toLowerCase();
  if (line=="a" || line=="b" || line=="c") { beginStage(line[0]-'a'+'A'); return; }
  if (line=="ajuda") { help(); return; }
  if (line=="comparar") { report(); return; }
  if (line=="limpar") {
    for (unsigned i=0;i<3;++i) { saved[i]=Snapshot{};hasSaved[i]=false; }
    beginStage('A'); Serial.println("Capturas apagadas."); return;
  }
  if (line=="salvar") {
    const auto s = samples.snapshot();
    if (s.count<WINDOW) { Serial.println("Aguarde 10 leituras antes de salvar."); return; }
    saved[active-'A']=s;hasSaved[active-'A']=true;
    Serial.printf("Captura %c salva em RAM%s.\n", active, s.usable()?"":" (inconclusiva: limite ou instabilidade)");
    return;
  }
  if (line.length()) Serial.println("Comando desconhecido. Use ajuda.");
}
void readCommands() {
  unsigned budget=64;
  while (Serial.available() && budget--) {
    const char c=Serial.read();
    if (c=='\r') continue;
    if (c=='\n') {
      if (overflow) Serial.println("Comando longo demais."); else command(input);
      input="";overflow=false;
    } else if (!overflow && input.length()<32) input+=c;
    else overflow=true;
  }
}
void sample() {
  // Conversoes consecutivas distintas: mV nao e conversao do mesmo raw.
  const uint16_t raw=analogRead(PH_PIN);
  const uint32_t mv=analogReadMilliVolts(PH_PIN);
  samples.add(raw,mv);
  if (raw<stageMinRaw) stageMinRaw=raw;
  if (raw>stageMaxRaw) stageMaxRaw=raw;
  if (mv<stageMinMv) stageMinMv=mv;
  if (mv>stageMaxMv) stageMaxMv=mv;
  const auto s=samples.snapshot();
  const char* state=raw>=HIGH_ADC?"LIMITE_ALTO":raw<=LOW_ADC?"PERTO_ZERO":s.high||s.low?"LIMITE_NA_JANELA":
    s.count<WINDOW?"FORMANDO_JANELA":s.constant()?"QUASE_CONSTANTE":"VARIANDO";
  Serial.printf("[%c +%lus] ADC=%u/4095 | V~%.3f (%lumV) | MM%u: ADC=%.1f mV=%.1f | etapa ADC[%u..%u] mV[%lu..%lu] | varFS10=%.3f%% | %s\n",
    active,(unsigned long)((millis()-started)/1000),raw,mv/1000.0,(unsigned long)mv,s.count,s.rawMean,s.mvMean,
    stageMinRaw,stageMaxRaw,(unsigned long)stageMinMv,(unsigned long)stageMaxMv,s.variationPercent(),state);
}
void setup() {
  Serial.begin(115200);input.reserve(32);
  pinMode(PH_PIN,INPUT);analogReadResolution(12);
  analogSetPinAttenuation(PH_PIN,ADC_11db);
  help();beginStage('A');lastSample=millis();
}
void loop() {
  readCommands();
  const uint32_t now=millis();
  if (uint32_t(now-lastSample)>=PERIOD_MS) { lastSample=now;sample(); }
  delay(1);
}
