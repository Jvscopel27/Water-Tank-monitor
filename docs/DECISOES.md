# Decisões propostas para aprovação

## Ferramentas e motivos

| Escolha | Como funciona | Por que usar neste projeto |
|---|---|---|
| C/C++ com Arduino Core | O sketch é compilado em código executável pelo ESP32; `setup()` inicializa e `loop()` mantém o equipamento funcionando. | Mantém a base do código existente e atende à escolha da equipe. |
| Arduino IDE 2.x | Editor, gerenciadores de placas/bibliotecas, compilação, upload e Monitor Serial. | Será o ambiente principal da equipe. Não é necessário manter um computador conectado após gravar. |
| esp32 by Espressif Systems | Inclui compilador, suporte à placa e APIs de hardware/rede. | É o suporte oficial para usar a API Arduino no ESP32. Referência de compilação adotada: 2.0.17, já disponível no ambiente de verificação. Não é uma afirmação de que seja a versão mais recente; atualização para 3.x exige repetir a validação. |
| Adafruit SSD1306 + GFX + BusIO | O driver envia ao display a imagem desenhada por GFX; BusIO dá suporte à comunicação das bibliotecas Adafruit. | Mantém as bibliotecas do código já utilizado e separa desenho da tela de comunicação elétrica. |
| OneWire + DallasTemperature | OneWire realiza o protocolo de um fio; DallasTemperature pede e interpreta as conversões de temperatura. | Reutiliza o código existente e permite aguardar a conversão sem travar o programa por 750 ms. |
| Wire | Implementa comunicação I²C, em SDA 21/SCL 22. | Já vem no pacote ESP32 e é usada pelo OLED. |
| WiFi + WebServer | A placa cria a rede e responde às requisições HTTP do celular. | Resolve a consulta local sem servidor externo nem aplicativo. Bibliotecas incluídas no pacote ESP32. |
| HTML/CSS/JavaScript embarcados | O ESP32 entrega a página ao navegador; JavaScript consulta JSON a cada cinco segundos. | O painel precisa de uma interface de navegador. São arquivos/textos embarcados no firmware, sem Node.js, React ou processo separado. |
| Preferences / NVS | Armazena pequenos registros na memória flash e recupera-os após reiniciar. | Adequado para calibração, faixas e senha. Evita gravar leituras continuamente na flash. |
| Buffer circular em RAM | Guarda até 1.440 médias de um minuto; ao completar, substitui a mais antiga. | Atende às últimas 24 horas sem cartão SD ou banco de dados. Reiniciar apaga o histórico. |
| Wokwi | Executa o firmware de demonstração e modelos virtuais de componentes. | Permite demonstrar entradas, OLED e falhas antes dos ensaios; não valida sensores analógicos reais. |
| Monitor Serial e multímetro | O primeiro exibe diagnósticos do software; o segundo mede as tensões físicas. | Permitem conferir aquisição e alimentação de forma complementar. |
| PlatformIO, opcional | Compila por comando usando versões fixas do core e das bibliotecas. | Foi usado como ferramenta auxiliar de verificação, aproveitando o compilador instalado. Não substitui a Arduino IDE escolhida para o trabalho diário. |
| Git/GitHub | Mantêm o histórico de alterações e a documentação compartilhada. | Cada conjunto de mudanças terá explicação e aprovação antes de commit/push. |

## O que muda no código

O sketch em `simulation/wokwi/sketch/sketch.ino` é preservado como demonstração. Corrigimos o caminho de compilação e a seleção dos arquivos no ambiente opcional do PlatformIO, pois o sketch foi organizado numa subpasta compatível com Arduino IDE.

O novo sketch `firmware/WaterTankMonitor/WaterTankMonitor.ino` reaproveita a pinagem e as bibliotecas de aquisição/display, mas implementa o comportamento da placa real:

- Conversão para milivolts pelo ADC, sem tratar potenciômetros como sondas calibradas.
- Calibração local por dois pontos, confirmação de verificação e bloqueio de extrapolação.
- Sessão de amostras com prazo; resultados antigos preservados com idade e estado explícitos.
- Aquisição a cada cinco segundos, conversão de temperatura sem espera bloqueante e funcionamento independente da página aberta.
- Painel local, senha gerada na primeira inicialização e configurações preservadas em NVS.
- Histórico de médias válidas, contagem de amostras por canal e exportação JSON.
- Faixas experimentais definidas pelo operador e alerta após três leituras consecutivas fora delas.
- Medições antigas não entram nas novas médias. Alterar calibração/faixas limpa o histórico para não misturar critérios.


Fontes: [instalação oficial do ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html), [ADC](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html), [Wi-Fi](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html), [Preferences](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html), [SSD1306](https://github.com/adafruit/Adafruit_SSD1306), [DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library).
