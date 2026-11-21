# Testes reproduzíveis

## Ambiente

Firmware: PlatformIO, plataforma `espressif32@6.12.0`, Arduino ESP32 2.0.17, placa `esp32dev`. Dependências de firmware em `firmware/platformio.ini`; ferramentas de teste em `tests/requirements.txt`.

Em Linux com Python 3.12, g++ e ambiente virtual:

```sh
python3 -m venv .venv
. .venv/bin/activate
pip install -r tests/requirements.txt
python -m playwright install --with-deps chromium
mkdir -p build
g++ -std=c++11 -c tests/measurement_compile_test.cpp -o build/measurement.o
g++ -std=c++11 tests/ph_three_point_test.cpp -o build/ph_test
./build/ph_test
g++ -std=c++11 tests/ph_diagnostic_test.cpp -o build/ph_diagnostic_test
./build/ph_diagnostic_test
python tests/continuous_reading_test.py
python tests/analog_acquisition_test.py
pio run -d firmware
python tests/dashboard_test.py --output build/dashboard
```

A instalação de dependências do navegador pode solicitar privilégios administrativos. O teste usa o Chromium instalado pelo Playwright; não depende de um caminho do Windows.

## Cobertura

- Faixas e interpolação; curva pH de três pontos; estruturas legadas.
- Aquisição contínua e retorno após leitura inválida.
- EC em µS/cm sem termômetro: referências superiores a 100 aceitas, preservação da escala e retomada do sinal.
- Site com três cartões, ausência dos controles de temperatura, captura automática EC sem temperatura.
- Quatro formatos OLED, preferências, tema, reconexão, tela móvel, gráficos e PDF sem coluna de temperatura.
- PDF vazio e histórico de 1440 minutos.

O fluxo `.github/workflows/check.yml` executa esses testes em push/PR. Consulte a aba Actions para o resultado remoto; criar o fluxo não significa que já passou no GitHub.

## Bancada

As calibrações experimentais de pH e turbidez estão em `calibration/`. Testes de navegador usam dados sintéticos, não validam sensores. Confirmar hardware exige placa real, tensões, referências e condições registradas. A compilação por si só não comprova funcionamento elétrico ou precisão.

Arquivos gerados e backups ficam em `build/`, ignorado pelo Git. Não publicar logs contendo senha da rede ou cópias da flash/NVS.
