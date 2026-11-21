# Comece aqui: primeira montagem

## 1. O que montar

Use ESP32 DevKit clássico, cabo USB de dados, OLED SSD1306 I²C, PH-4502C com sonda, módulo analógico de turbidez e módulo TDS/condutividade. **Não há sensor de temperatura nesta versão.** Consulte a [lista de materiais](../hardware/bom.csv) e a [tabela de fios](../hardware/README.md) antes de alimentar.

Monte com a alimentação desligada. A sonda pode ir na amostra; placas eletrônicas e conectores devem permanecer secos. Não conecte alimentação externa e USB em paralelo sem verificar a placa.

## 2. Baixar e instalar

1. No GitHub, clique **Code → Download ZIP** e extraia tudo.
2. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
3. Em Preferências, adicione a URL de placas ESP32: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
4. Abra o Gerenciador de Placas, procure **esp32 by Espressif Systems** e instale **2.0.17**.
5. No Gerenciador de Bibliotecas instale Adafruit GFX **1.12.6**, SSD1306 **2.5.16** e BusIO **1.17.4**.
6. Abra `firmware/WaterTankMonitor/WaterTankMonitor.ino`. Não copie somente o `.ino`: os arquivos `.h` contêm partes do programa e o site.

## 3. Gravar

Conecte o cabo USB. Selecione **ESP32 Dev Module** e a porta que aparece ao conectar a placa (Windows: COM; Linux: `/dev/ttyACM0` ou `/dev/ttyUSB0`). Clique em **Verificar**, depois **Carregar**. Feche outros programas que estejam usando a porta.

Se não conectar durante o upload, mantenha BOOT pressionado quando aparecer “Connecting” e solte após iniciar a gravação. Isso não substitui conferir cabo, porta e modelo da placa.

## 4. Abrir o site

1. Monitor Serial: **115200 baud**, final de linha **Nova linha**.
2. Envie `rede`. Anote a senha exibida; não publique esse log.
3. No celular ou PC, conecte-se a **WaterTankMonitor**. Aceite permanecer conectado sem internet.
4. Abra **http://192.168.4.1** no navegador.
5. Informe a senha na área de configurações para salvar alterações.

Devem aparecer **pH, turbidez e condutividade**, sem cartão de temperatura. As leituras se repetem a cada 5 segundos, mesmo sem navegador aberto. Os botões de período são apenas marcadores; não ligam/desligam a aquisição.

## 5. Primeira leitura não é calibração

Placa nova pode mostrar “calibração pendente”. Isso é esperado: confira primeiro se o campo **Entrada (mV)** muda quando a amostra muda. A faixa aceita pelo firmware é 150–2300 mV; isso é um critério do programa, não a especificação elétrica máxima do GPIO.

Envie `demo todos off` para garantir sensores reais. Não use `validar` antes de cadastrar e conferir a curva. Siga [Operação](OPERACAO.md). Os pontos registrados em `calibration/` pertencem à bancada do proprietário e não são aplicados automaticamente em novas placas.

## 6. Critério de conclusão

- Site abre e mostra três canais.
- Tensões correspondem às entradas conectadas.
- Simulação desligada para medições reais.
- Curvas calibradas na sua montagem, com referências registradas.
- Após reiniciar, calibrações continuam salvas; histórico recomeça vazio.

Se algo falhar, consulte [Solução de problemas](SOLUCAO_DE_PROBLEMAS.md).
