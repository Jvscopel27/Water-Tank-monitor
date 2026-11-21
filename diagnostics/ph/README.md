# Diagnóstico temporário do PH-4502C

Sketch independente para ESP32 clássico, **Po no GPIO 35 (ADC1)**, alimentação experimental do módulo em **3V3**, GND comum, serial 115200 baud. GPIO34 permanece reservado à turbidez no projeto principal. Nenhuma conversão para pH é realizada neste teste.

## Análise do projeto antes da implementação

Em `firmware/WaterTankMonitor/WaterTankMonitor.ino`:

- `ANALOG_PINS` define pH=35, turbidez=34, EC=32.
- `readMillivolts()` descarta a primeira conversão e faz média de 16 leituras calibradas em mV.
- `collect()` no modo SIM calcula `raw * 14 / (16 * 4095)` para pH, sendo `raw` a soma de 16 conversões. Logo, ADC=4095 em todas produz exatamente 14. Não há divisão inteira nessa expressão, pois o fator é float. Esse valor representa a escala artificial, não uma calibração física.
- Em modo real, `collect()` usa `interpolate()` de `Measurement.h`, exige sessão, curva verificada, ADC entre 150 e 2300 mV e recusa extrapolação. Não há um clamp que transforme qualquer leitura real em 14.
- `command()` trata `cal`, `capturar`, `verificar` e `validar`; `persist()` grava as curvas. O diagnóstico não chama essas funções nem escreve NVS.
- A saturação observada (pH=14 em SIM, ADC informado ~3124 mV) é compatível com entrada no teto do ADC. A origem elétrica não foi determinada. Alimentação inadequada, offset, ligação ou defeito são hipóteses.

O firmware instalado também anunciou comandos de Telegram ausentes desta cópia do repositório. **Não foi substituído.** O firmware principal e seus headers não são alterados por este teste.

## Limites elétricos e do ensaio

O PH-4502C usual é especificado para 5 V, mas este ensaio solicitado usa apenas 3,3 V. Isso pode impedir a operação correta do circuito; não é validação de funcionamento nessa tensão. **Não alimentar o módulo com 5 V mantendo Po direto no ESP32.** Configurar ADC_11db não protege a entrada contra sobretensão.

O ADC não substitui multímetro para verificar alimentação ou segurança elétrica: uma leitura saturada não revela quanto a tensão real excedeu sua faixa. O teste não mede VCC. Alterações físicas devem ocorrer com alimentação desligada. Confira os rótulos V+, G e Po da unidade. Não usar Do/To como saída de pH.

Não se deve identificar o trimpot por “inferior”, pois isso depende da orientação da placa. Na variante usual, o ajuste de offset fica mais próximo do BNC; o outro ajusta o limiar de Do. Confirme pela documentação/foto da sua revisão. Sem identificação confiável, não ajuste. Se identificado, acompanhe pequenos ajustes com ferramenta isolada, sem tocar ou curto-circuitar a placa energizada.

## Compilar e executar

Arduino IDE: abra `PhDiagnostic/PhDiagnostic.ino` com `DiagnosticStats.h` ao lado; ESP32 Dev Module, core de referência 2.0.17. Sem bibliotecas externas.

PlatformIO (na raiz do repositório):

```bash
python -m platformio run --project-dir diagnostics/ph
```

Este sketch é uma aplicação separada. **Gravá-lo substitui a aplicação em execução**: durante o teste não haverá painel/OLED/Telegram nem aquisição dos demais sensores. Antes de gravar, obtenha a fonte ou backup recuperável da versão instalada com Telegram. A aplicação de diagnóstico não apaga deliberadamente configurações NVS, mas isso não substitui um backup. Não foi feito upload durante a implementação.

## O que aparece a cada 500 ms

- ADC bruto de 12 bits (0–4095).
- `V~` e mV de `analogReadMilliVolts()`.
- MM1…MM10: médias móveis do ADC bruto e dos mV; a janela cresce até dez leituras, depois substitui a mais antiga.
- Mínimos/máximos de ADC e mV desde o início da etapa atual.
- `varFS10 = 100 × (ADC máximo − ADC mínimo) / 4095` na janela recente. É percentual da escala completa, não erro relativo nem precisão, e não divide pela média próxima de zero.
- Estado heurístico: `LIMITE_ALTO` para ADC ≥4055; `PERTO_ZERO` para ADC ≤40; `LIMITE_NA_JANELA` quando houve limite nas últimas dez leituras; `QUASE_CONSTANTE` para janela cheia com amplitude ≤12 contagens e ≤10 mV; ou `VARIANDO`.

`analogRead()` e `analogReadMilliVolts()` fazem **conversões consecutivas diferentes**. Os mV não são calculados a partir do mesmo número bruto impresso. Junto aos limites, os mV não devem ser interpretados como tensão exata. Sinal constante numa solução pode ser normal; sinal constante em diferentes soluções é apenas indício a investigar.

## Procedimento A/B/C

Monitor Serial: 115200 baud, final de linha “Nova linha”. Comandos aceitam maiúsculas ou minúsculas. Os dados são temporários em RAM.

1. **A — Água:** coloque a parte apropriada da sonda na água, envie `a` e aguarde estabilização física (pode levar um minuto ou mais). Examine a janela recente. Envie `salvar` para guardar suas últimas dez leituras. Não assumir pH 7 para água.
2. **B — Resposta a uma solução ácida:** enxágue a sonda conforme o fabricante; use outro recipiente com água e pequena quantidade de vinagre. Não altere offset, alimentação ou montagem. Envie `b`, aguarde estabilização e use `salvar`. Envie `comparar`.
3. **C — Offset:** mantenha a mesma solução de B. Envie `c` antes do ajuste para acompanhar os extremos e o efeito do trimpot de offset identificado. Faça pequenos ajustes, observando o ADC sair ou não do limite; não tente forçar um pH exibido. Espere estabilizar, use `salvar` e depois `comparar`.

Cada `a`, `b` ou `c` reinicia a janela e os extremos da etapa, mas preserva capturas salvas. `salvar` substitui somente a captura da etapa ativa e exige dez leituras. Capturas saturadas/instáveis são guardadas com indicação de resultado inconclusivo. `limpar` apaga as três capturas e reinicia A. `ajuda` lista os comandos. Uma captura salva não se atualiza automaticamente com novas leituras.

Depois de mexer no offset, repita A e B com esse novo ajuste fixo para verificar novamente a resposta entre soluções. Não misture uma captura antiga com outra feita após mudar o offset.

## Interpretação da comparação automática

O resumo compara B−A e C−B em mV. Só classifica a variação quando ambas as capturas contêm dez leituras estáveis e nenhuma amostra perto dos limites. Considera significativa uma diferença absoluta maior que `max(20 mV, 3 × (amplitude A + amplitude B))`. Esse critério é heurístico, não um teste estatístico ou especificação de exatidão.

- **Limite alto:** possível saturação da entrada; não informa a tensão real acima do teto nem prova sensor danificado.
- **Perto de zero:** investigar ligação, alimentação, offset ou falha; entrada desconectada também pode produzir valores plausíveis.
- **A/B com diferença significativa:** há resposta elétrica entre condições, se todas as outras condições ficaram iguais. Não valida pH nem linearidade.
- **A/B sem diferença significativa:** pouca diferença efetiva, falta de equilíbrio, sonda, ligação, offset ou alimentação em 3V3 são possibilidades.
- **B/C com diferença significativa:** há resposta ao ajuste, se a solução ficou igual. Não comprova resposta da sonda ao pH.
- **Sinal responsivo e pH travado no sistema principal:** revisar modo SIM e curva. Sem padrões conhecidos, não é possível diagnosticar nem corrigir quantitativamente o erro de calibração.

Água/vinagre servem apenas para comparação qualitativa. A calibração real requer soluções de pH conhecido e uma terceira referência de verificação. Nenhum valor de pH é inferido desses líquidos no código de teste.

## Testes de software

```bash
g++ -std=c++11 -Wall -Wextra -Werror tests/ph_diagnostic_test.cpp -o /tmp/ph-diagnostic-test
/tmp/ph-diagnostic-test
g++ -std=c++11 -fsyntax-only tests/measurement_compile_test.cpp
```

Cobertura: janela parcial/cheia e substituição circular, médias, extremos, valores zero, limites inclusivos, estabilidade, rejeição de comparações saturadas/instáveis e limiar de diferença com amplitude de ruído. Não substitui ensaio com hardware.

Validação desta implementação (13/11/2026): testes C++ do diagnóstico e os 15 static_assert de Measurement.h aprovados. Build ESP32 aprovado com espressif32 6.12.0 / Arduino Core 2.0.17: 21.856 bytes de RAM estática e 282.401 bytes de aplicação em flash. Nenhum teste A/B/C físico executado e nenhum upload feito. Neste computador, PlatformIO foi preparado num ambiente isolado em `/tmp/wtm-ph-tools`; o comando usado foi `/tmp/wtm-ph-tools/bin/python -m platformio run --project-dir diagnostics/ph`.

## Referências

- [Espressif: ADC, resolução, atenuação e analogReadMilliVolts](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html).
- [Fornecedor LaskaKit: PH-4502C, pinagem, offset e alimentação nominal](https://www.laskakit.cz/en/diy-more-ph-4502c-modul-pro-mereni-ph/). Referência de uma variante, não confirmação da revisão comprada.
- [Guia disponibilizado pela ElectroPeak](https://electropeak.com/pub/media/wysiwyg/files/ph0-14.pdf).
