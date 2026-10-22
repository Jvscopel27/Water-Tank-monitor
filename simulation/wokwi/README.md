# Simulação do Water Tank Monitor no Wokwi

![Montagem do circuito virtual no Wokwi](../../hardware/Circuito%20%282%29.png)

*Imagem da montagem fornecida pela equipe. Os três controles representam os sensores analógicos; a imagem não registra a execução do programa.*

O arquivo `diagram.json` já contém os componentes e todas as conexões. O programa `sketch/sketch.ino` lê as entradas e atualiza o OLED e o monitor serial a cada aproximadamente um segundo. Abra esse arquivo na Arduino IDE; a pasta e o sketch têm o mesmo nome, conforme a organização usada pela IDE.

**Estado de validação:** diagrama aprovado pelo `wokwi-cli lint` sem erros ou avisos, com duas mensagens informativas sobre os modelos de ESP32 e DS18B20. Compilação local aprovada com Arduino Core 2.0.17 e as versões fixadas em `libraries.txt`. A equipe forneceu a captura abaixo da demonstração em execução; o cenário automatizado completo ainda não foi executado nesta sessão.

![OLED com leituras simuladas](../../hardware/Tela%20OLED.png)

## O que está montado

| Componente | Ligação | Como interagir |
|---|---|---|
| ESP32 DevKit V1 virtual | Controlador | Iniciar a simulação. |
| OLED SSD1306, 128×64 | SDA 21, SCL 22, 3V3 e GND | Exibe as leituras. |
| DS18B20 virtual | DQ 4, 3V3, GND e pull-up de 4,7 kΩ | Clicar no sensor e alterar a temperatura. |
| Controle de turbidez relativa | SIG 34, 3V3 e GND | Arrastar entre 0 e 100%. |
| Controle de pH sintético | SIG 35, 3V3 e GND | Arrastar entre 0 e 14. |
| Controle de EC sintética | SIG 32, 3V3 e GND | Arrastar entre 0 e 2000 µS/cm. |
| Botão vermelho | GPIO27 e GND, pull-up interno | Manter pressionado por pelo menos 2 segundos para simular indisponibilidade; soltar para recuperar. |

Os valores iniciais configurados são 25 °C, pH próximo de 7, turbidez relativa próxima de 10% e EC sintética próxima de 500 µS/cm. Os valores dos controles dependem da conversão ADC do simulador.

## Abrir no editor web

1. No [Wokwi](https://wokwi.com/), crie um projeto **ESP32 com Arduino**.
2. Substitua o conteúdo da aba `sketch.ino` pelo [programa da demonstração](sketch/sketch.ino).
3. Substitua o conteúdo de `diagram.json` pelo [diagrama desta pasta](diagram.json). A montagem inteira é definida nesse arquivo; não é necessário adicionar os componentes um a um.
4. Adicione o arquivo [libraries.txt](libraries.txt), preservando esse nome, ou instale as cinco bibliotecas listadas nele pelo **Library Manager**.
5. Inicie a simulação no botão ▶. O OLED deve mostrar `WATER TANK | SIMULADO` e o monitor serial deve registrar `OLED_OK` e `LEITURA_OK`.
6. Mova os três controles, altere a temperatura e teste o botão vermelho.
7. Salve o projeto na sua conta para obter o link compartilhável.

O endereço de um projeto salvo só será documentado após sua criação no Wokwi. O link do repositório não equivale a uma simulação publicada.

## Compilar localmente

Com PlatformIO Core instalado, execute nesta pasta:

```bash
python -m platformio run
```

O arquivo `platformio.ini` descreve o ambiente local. O editor web usa `sketch.ino`, `diagram.json` e `libraries.txt`.

## Validar no Wokwi CLI

Com a compilação concluída, Wokwi CLI instalado e a variável `WOKWI_CLI_TOKEN` configurada localmente:

```bash
wokwi-cli lint
wokwi-cli . --scenario simulation.test.yaml --timeout 30000 --serial-log-file serial.log
```

O cenário verifica inicialização do OLED, aquisição, mudança de temperatura, resposta dos três controles, indicação de falha e recuperação. Também solicita uma captura do OLED. A criação do cenário não significa que ele tenha sido executado.

## Diferença em relação ao circuito físico

Esta é uma demonstração de aquisição e exibição, com dados sintéticos. Não implementa o firmware definitivo, calibração das sondas, Wi-Fi, histórico ou análise da água.

Os módulos PH4502C, TDS V1.0 e turbidez foram substituídos por controles do simulador. A escala de EC é sintética e não resulta de conversão de TDS.

Os divisores e filtros do circuito físico **não foram inseridos entre os controles e o ESP32**: o Wokwi não suporta simulação analógica completa e sua documentação informa limitações no uso de resistores com potenciômetros. Por isso o diagrama desta pasta não deve ser copiado para ligar as sondas reais. Consulte o [projeto elétrico de bancada](../../hardware/README.md).

### Referências

- [Formato do diagrama Wokwi](https://docs.wokwi.com/diagram-format).
- [Potenciômetros e limitações analógicas](https://docs.wokwi.com/parts/wokwi-potentiometer).
- [Resistores no simulador](https://docs.wokwi.com/parts/wokwi-resistor).
- [Sensor DS18B20](https://docs.wokwi.com/parts/wokwi-ds18b20).
- [Display SSD1306](https://docs.wokwi.com/parts/board-ssd1306).
- [Bibliotecas no editor web](https://docs.wokwi.com/guides/libraries).
- [Cenários de teste](https://docs.wokwi.com/wokwi-ci/automation-scenarios).
