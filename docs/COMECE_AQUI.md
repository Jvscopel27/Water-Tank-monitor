# Comece aqui: da instalação à primeira leitura

Este guia monta primeiro a **bancada com três potenciômetros**, que já permite testar o site e o OLED. Depois explica a passagem para os sensores reais. Um potenciômetro é um botão giratório que muda uma tensão; ele representa uma medição, mas não mede água.

## 1. Separe os materiais

- ESP32-WROOM-32 DevKit e cabo USB **de dados**.
- OLED SSD1306 I²C de 128 × 64 pixels, com quatro pinos.
- Três potenciômetros, protoboard e fios.
- Opcional: DS18B20 de três fios e resistor de 4,7 kΩ para temperatura.

A bancada atual usa potenciômetros para pH, turbidez e EC. **Temperatura não é simulada:** sem DS18B20 conectado, aparecerá indisponível. EC significa condutividade elétrica.

## 2. Baixe o projeto e a Arduino IDE

No [repositório](https://github.com/Jvscopel27/Water-Tank-monitor), clique em **Code → Download ZIP** e extraia a pasta. Não é necessário instalar Git para esse caminho.

Se já usa Git, a alternativa é:

```bash
git clone https://github.com/Jvscopel27/Water-Tank-monitor.git
cd Water-Tank-monitor
```

Baixe a **Arduino IDE 2.x** no [site oficial](https://www.arduino.cc/en/software), escolha seu sistema e instale. Ela é o programa que abre, compila e envia o código à placa. Compilar significa transformar o texto do programa em instruções que o ESP32 executa. [Guia oficial de instalação](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).

## 3. Instale o suporte ao ESP32

1. Abra **Arquivo → Preferências** na IDE (os nomes podem variar com o idioma).
2. Em **URLs adicionais para Gerenciadores de Placas**, adicione esta URL, preservando outras que já existirem:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. Abra o **Gerenciador de Placas**, procure `esp32` e instale **esp32 by Espressif Systems**, versão **2.0.17**.
4. Selecione **ESP32 Dev Module** para a DevKit ESP32 clássica usada neste projeto.

A versão 2.0.17 é a referência testada, não uma indicação de versão mais recente. Placas ESP32-C3/S3 e outros modelos precisam de revisão de pinagem e configuração. [Instruções da Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).

## 4. Instale as bibliotecas

Bibliotecas são códigos prontos que nosso programa usa para conversar com componentes. No **Gerenciador de Bibliotecas**, procure e instale:

| Nome | Versão de referência | Para que serve |
|---|---|---|
| Adafruit GFX Library | 1.12.6 | Desenha letras, linhas e gráficos. |
| Adafruit SSD1306 | 2.5.16 | Envia a imagem para o OLED. |
| Adafruit BusIO | 1.17.4 | Apoia a comunicação das bibliotecas Adafruit. |
| OneWire | 2.3.8 | Comunica com o sensor de temperatura por um fio de dados. |
| DallasTemperature | 4.0.6 | Solicita e interpreta as temperaturas do DS18B20. |

Aceite as dependências solicitadas e confira as versões. `Wire`, `WiFi`, `WebServer` e `Preferences` já vêm no pacote ESP32; não precisam de instalação separada. Não é necessário instalar Node.js, React, Python ou um servidor web para usar o equipamento.

## 5. Grave o firmware

1. Conecte primeiro somente a DevKit ao USB.
2. Abra `firmware/WaterTankMonitor/WaterTankMonitor.ino` na IDE. Mantenha **todos os arquivos `.h` dessa pasta junto do `.ino`**. Copiar apenas o sketch causa erro de arquivos ausentes.
3. Em **Ferramentas → Porta**, escolha a porta que aparece ao conectar a placa. O número COM varia entre computadores.
4. Clique em **Verificar** (✓). Depois clique em **Carregar** (→) e aguarde o término.
5. Abra o **Monitor Serial**, selecione **115200 baud** e final de linha **Nova linha**. Pressione EN/RESET na placa se precisar ver a inicialização.
6. Envie `rede` para consultar o nome da rede, a senha e o IP. Na primeira inicialização, o programa gera uma senha e a salva na placa.

Se não aparecer porta, verifique o cabo de dados e o conversor USB da sua placa. A unidade usada na bancada tem CH9102; o fabricante disponibiliza o [driver CH343SER para a família CH910X](https://www.wch.cn/downloads/CH343SER_EXE.html). Outras DevKits podem ter outro conversor e exigir outro driver.

Se o upload ficar em `Connecting...`, tente segurar BOOT ao iniciar o envio e soltar quando a gravação começar. Feche outros programas que estejam usando a porta serial.

## 6. Monte a bancada com a alimentação desligada

Desconecte o USB antes de mexer nos fios. Nesta montagem simples, alimente a DevKit pelo USB e os controles/OLED pelo pino **3V3**. Todas as conexões GND devem compartilhar o mesmo terra.

| Componente | Ligação ao ESP32 |
|---|---|
| OLED VCC / GND | 3V3 / GND |
| OLED SDA / SCL | GPIO 21 / GPIO 22 |
| Potenciômetro de pH: terminal central | GPIO 35 |
| Potenciômetro de turbidez: terminal central | GPIO 34 |
| Potenciômetro de EC: terminal central | GPIO 32 |
| Cada potenciômetro: terminais laterais | Um em 3V3, outro em GND |
| DS18B20, se instalado: DATA | GPIO 4, com resistor de 4,7 kΩ entre DATA e 3V3 |
| DS18B20: alimentação | VDD em 3V3 e GND em GND; confira a identificação dos fios |

Trocar as duas laterais do potenciômetro apenas inverte o sentido da variação. Não deduza os fios do DS18B20 só pela cor. O OLED esperado usa endereço I²C `0x3C`; o comando serial `display` ajuda a conferir a detecção.

**Nunca aplique 5 V nos GPIOs do ESP32.** A montagem com sensores de 5 V exige os divisores e cuidados do [guia elétrico](../hardware/README.md), não as ligações diretas dos potenciômetros. Não combine USB e alimentação externa sem revisar o circuito de alimentação.

## 7. Abra o site e teste

1. Ligue a placa. No celular ou computador, conecte-se à rede **WaterTankMonitor**, com a senha obtida no Serial.
2. Se aparecer “rede sem internet”, mantenha a conexão: o site está dentro do ESP32.
3. Abra **http://192.168.4.1** no navegador, ou o IP informado por `rede`.
4. Informe a senha em **Acesso às configurações** para habilitar alterações.
5. Na área **Potenciômetros**, habilite os três canais. Também é possível enviar pelo Serial:

   ```text
   demo todos on
   ```

6. Gire um controle por vez e aguarde a próxima leitura, aproximadamente cinco segundos. pH varia de 0 a 14; turbidez de 0 a 100%; EC de 0 a 2000 µS/cm. Os valores ficam marcados como **simulados/SIM**.
7. Em **Tela OLED**, escolha lista, grade, destaque ou gráfico. Escolha o parâmetro do gráfico e clique em **Salvar tema e tela**. A prévia não é uma gravação.
8. Escolha tema claro/escuro, brilho, inversão e giro conforme a preferência. O gráfico reúne até 48 leituras recentes, aproximadamente quatro minutos.
9. Espere pelo menos um minuto e baixe o **relatório PDF** em Histórico e relatório. Ele funciona sem internet; os dados JSON são usados internamente pelo site.

O gráfico do OLED tem escala automática. Ele mostra a evolução do parâmetro escolhido, não uma fotografia da água. Sem valores válidos, não há curva; ausência de temperatura não vira zero.

## 8. Confira o que permanece ao desligar

| Permanece na placa | Reinicia vazio/inativo |
|---|---|
| Senha, tema, formato e parâmetro do OLED, brilho, giro e inversão | Histórico e pontos dos gráficos |
| Canais com simulação habilitada | Sessão de amostra em andamento |
| Calibrações, faixas e correção de temperatura | Captura automática e primeiro ponto ainda não concluído |

Faça uma escolha, salve, reinicie e confira. O histórico usa RAM para evitar gravações contínuas na flash. Exporte o PDF antes de desligar se precisar guardar os resultados. Alterar modo de medição ou calibração também limpa as séries para não misturar critérios.

## 9. Passe para sensores reais

O [guia de hardware](../hardware/README.md) explica PH4502C, turbidez de 5 V, módulo TDS V1.0 e DS18B20. Confira a pinagem e as tensões da unidade recebida com multímetro antes de conectar. O módulo TDS será usado numa **estimativa experimental de EC**, que precisa de referências próprias.

1. Desligue, substitua os controles pelos circuitos apropriados e confira a montagem.
2. Desative a simulação do canal no painel. Potenciômetros não calibram sondas.
3. Abra o tutorial de calibração do sensor no site, prepare uma referência conhecida e confirme a preparação física.
4. Informe o valor da primeira referência e use **Capturar automaticamente**. O programa espera estabilidade; não precisa acertar o momento de clicar.
5. Enxágue/prepare a sonda conforme seu procedimento, troque para outra referência e repita para o segundo ponto. Conclua em até 30 minutos ou recapture o primeiro.
6. Verifique uma terceira referência independente e intermediária. A curva fica pendente até ser verificada. Para EC, o sistema exige temperatura real de **25 ±0,5 °C**.
7. Para pH e EC reais, inicie uma sessão com `amostra 300` (cinco minutos), pelo painel ou Serial. `parar` encerra. Sem sessão, essas leituras não são tratadas como atuais.

A automação verifica estabilidade e calcula a curva; não prepara soluções nem confirma a concentração real delas. Use o [manual de operação](OPERACAO.md) para o procedimento detalhado e registre os ensaios. Para temperatura, é necessário um termômetro de referência; o ajuste é uma correção constante, limitada a ±5 °C.

## Problemas frequentes

| Sintoma | O que conferir |
|---|---|
| Erro de biblioteca ou arquivo `.h` ausente | Instalação das cinco bibliotecas e pasta completa do sketch. |
| OLED apagado | Alimentação, GND, SDA/SCL, modelo SSD1306 e endereço; use `display` no Serial. |
| Site não abre | Rede WaterTankMonitor ativa, IP correto e `http://`; desative troca automática para outra rede durante o teste. |
| Controles não mudam a leitura | Terminal central no GPIO correto, GND comum, simulação habilitada e espera de cinco segundos. |
| Temperatura mostra `--` | DS18B20 ausente, conexão/pull-up incorretos ou leitura inválida; não há potenciômetro de temperatura. |
| Sensor real mostra pendência | Calibração/verificação, sessão de amostra, tensão e condições térmicas. |
| Captura automática cancelou | Aba em segundo plano, mudança de referência, perda de conexão ou reinício; confira o estado e inicie novamente. |
| Histórico sumiu | Reinício ou alteração de critérios; esse armazenamento é temporário. |

## Próxima leitura

Leia [como o código funciona](COMO_O_CODIGO_FUNCIONA.md) para entender a lógica, [o painel](PAINEL.md) para os controles e [os testes](TESTES.md) para separar o que já foi conferido do que ainda depende de ensaio físico.
