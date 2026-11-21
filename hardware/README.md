# Montagem elétrica atual

O firmware atual utiliza três entradas ADC1. **Não conectar DS18B20: temperatura foi retirada.** Faça as ligações com a alimentação desligada e confira a identificação impressa no módulo; cores dos fios não garantem função.

| Parte | Ligação | Estado da informação |
|---|---|---|
| OLED SSD1306 | SDA → GPIO21; SCL → GPIO22; VCC → 3V3; GND → GND | Usado na bancada |
| PH-4502C | V+ → 5 V; G → GND comum; Po → divisor → GPIO35 | Montagem confirmada pelo proprietário |
| Divisor pH | Po → 6,8 kΩ → nó GPIO35 → 6,8 kΩ → GND | Dois resistores iguais; tensão no ADC nominalmente metade de Po |
| Turbidez | Saída analógica → adequação de tensão → GPIO34 | GPIO confirmado; conferir modelo e divisor efetivos |
| TDS/condutividade | Saída analógica → adequação de tensão → GPIO32 | GPIO confirmado; conferir modelo, alimentação e tensão máxima |

```text
PH-4502C Po ── 6,8 kΩ ──┬── GPIO35
                        └── 6,8 kΩ ── GND
PH-4502C G ───────────────────────── GND ESP32
```

**Nunca aplique 5 V diretamente aos GPIOs.** Po é a saída analógica do pH; Do não é a leitura analógica. Não suponha que todos os módulos de turbidez/TDS tenham a mesma alimentação ou saída. Confira as especificações da unidade antes de conectá-la; se faltarem, identifique o módulo primeiro.

A faixa aceita para calibração no programa é **150–2300 mV no GPIO**, depois do divisor. Dois resistores iguais podem produzir 2500 mV com entrada de 5 V: isso está fora da faixa de calibração do programa. Não confunda essa faixa de software com uma garantia de proteção elétrica. A bancada de pH foi ajustada para trabalhar dentro dela.

Alimente a DevKit conforme sua especificação. Não interligue fonte externa e USB sem verificar a forma de alimentação da placa. Mantenha eletrônica seca, sem respingos; somente as partes indicadas pelo fabricante podem ser submersas.

## Materiais e desenhos

[Lista de materiais](bom.csv). `circuito.svg`, `circuito.png` e `Circuito (2).png` são **desenhos históricos**, não o esquema confirmado desta revisão. O [projeto elétrico histórico](PROJETO_ELETRICO_HISTORICO.md) foi mantido para rastreabilidade e inclui temperatura. Falta publicar o esquema final dos módulos de turbidez e TDS; veja [o plano](../docs/ORGANIZACAO.md).
