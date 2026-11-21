# Como o código funciona

`setup()` carrega preferências e calibrações da NVS, configura ADC de 12 bits/11 dB, OLED e Wi-Fi AP. `loop()` atende HTTP e comandos Serial, coleta a cada 5 segundos e atualiza o OLED. Não há OneWire, DS18B20 ou espera de conversão de temperatura.

| Arquivo | Responsabilidade |
|---|---|
| `WaterTankMonitor.ino` | Hardware, comandos, validação, API HTTP e aquisição |
| `Measurement.h` | Estruturas, faixa válida, interpolação de dois/três pontos |
| `Settings.h` | Preferências persistentes e janela de quatro leituras |
| `Trend.h` | Estrutura das amostras recentes |
| `Panel.h` | HTML/CSS e interação principal do site |
| `Interactions.h` | Gráficos e captura automática |
| `Report.h` | PDF gerado no navegador |

Por compatibilidade dos índices, os vetores da API/histórico ainda têm quatro posições: **0 reservado**, 1 pH, 2 turbidez, 3 EC. A posição 0 não é adquirida, exibida ou exigida para avaliar os demais canais. O campo legado de offset térmico permanece somente no layout das preferências NVS, para não perder configurações antigas.

EC mantém o nome `ec`, unidade µS/cm, referências entre 0 e 100000 e a mesma chave NVS. Não há compensação térmica. O modo simulado continua 0–2000 µS/cm; é um valor sintético de teste.

Curvas antigas são carregadas pelo tamanho da estrutura legada. A curva estendida pode conter terceiro ponto intermediário de pH; a conversão é linear por trechos. O firmware público não injeta os pontos particulares da bancada em outras placas.

Estados inválidos não suspendem ADC nem atendimento HTTP. O próximo sinal válido renova o valor e a idade. A média por minuto e o gráfico incluem apenas amostras válidas (ou simuladas identificadas).

Testes e comandos: [TESTES.md](TESTES.md).
