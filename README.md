<div align="center">

# 💧 Water Tank Monitor

**Monitoramento de parâmetros da água para apoiar o cuidado com reservatórios residenciais.**

![Status: em desenvolvimento](https://img.shields.io/badge/status-em_desenvolvimento-0284C7?style=flat-square)
![Projeto acadêmico](https://img.shields.io/badge/projeto-acad%C3%AAmico-0F766E?style=flat-square)
![Escopo: protótipo de bancada](https://img.shields.io/badge/escopo-prot%C3%B3tipo_de_bancada-475569?style=flat-square)
![C/C++](https://img.shields.io/badge/firmware-C%2FC%2B%2B-00599C?style=flat-square&logo=cplusplus)
![Arduino IDE](https://img.shields.io/badge/ambiente-Arduino_IDE-00878F?style=flat-square&logo=arduino)
![ESP32](https://img.shields.io/badge/placa-ESP32--WROOM--32-E7352C?style=flat-square&logo=espressif)
[![Licença: MIT](https://img.shields.io/badge/licen%C3%A7a-MIT-16A34A?style=flat-square)](LICENSE)

[Sobre](#sobre-o-projeto) · [Funcionalidades](#funcionalidades-planejadas) · [Primeiros passos](#primeiros-passos)· [Frame](#frame) · [Contribuição](#como-contribuir) · [Equipe](#equipe)

</div>

---

## Sobre o projeto

O **Water Tank Monitor** é um projeto acadêmico de um dispositivo de baixo custo para acompanhar **turbidez, pH, condutividade elétrica e temperatura** da água. A proposta é disponibilizar as leituras em um display local e em uma interface acessível pelo navegador do celular, via Wi-Fi.

O projeto busca apoiar a inspeção de caixas d'água residenciais e a limpeza preventiva, tornando alterações nos parâmetros medidos mais fáceis de acompanhar.

A entrega prevista é um **protótipo de bancada**. A instalação permanente em uma residência dependerá da validação dos sensores, dos materiais, da vedação e das condições de operação.

> **Em desenvolvimento:** há uma demonstração para Wokwi e uma versão de firmware para ensaios com a placa real. Compilar o código não equivale a validar as sondas: calibração, precisão e operação em bancada ainda precisam ser verificadas.

## O que será monitorado

| Parâmetro | O que permite acompanhar | Como será apresentado |
| :--- | :--- | :--- |
| **Turbidez** | Alterações relacionadas à presença de partículas em suspensão. | Indicador relativo, comparado com uma referência de ensaio. |
| **pH** | Acidez ou alcalinidade da água. | Leitura após calibração e verificação da sonda. |
| **Condutividade elétrica** | Resposta relacionada aos íons dissolvidos na água. | Valor estimado em µS/cm a 25 °C, após calibração e verificação. |
| **Temperatura** | Condição térmica da amostra. | Informação auxiliar para interpretar as medições. |

> **Limite do projeto:** o dispositivo fará uma triagem dos parâmetros medidos e **não confirmará a potabilidade da água**. Leituras sem alterações não significam que a água esteja segura para beber. Essa limitação faz parte do escopo definido no documento do projeto.

## Funcionalidades e estado do projeto

- **Leitura de quatro parâmetros:** turbidez, pH, condutividade elétrica e temperatura.
- **Painel web local:** consulta das últimas leituras pelo navegador do celular.
- **Rede Wi-Fi própria:** acesso à rede `WaterTankMonitor`, protegida por senha.
- **Display no equipamento:** exibição alternada das leituras, do estado do sistema e do tempo desde a medição.
- **Operação independente do celular:** a coleta e a exibição continuarão enquanto o equipamento estiver alimentado, mesmo com o navegador fechado.
- **Histórico temporário:** médias de um minuto, limitadas às últimas 24 horas, armazenadas em memória RAM.
- **Indicação de atualização:** identificação da idade das leituras e de resultados referentes à última amostra.
- **Tratamento de falhas:** sinalização de medição indisponível, calibração pendente ou alteração detectada.
- **Persistência de configurações:** armazenamento separado da calibração e das configurações em memória não volátil.

O [firmware de bancada](firmware/WaterTankMonitor/WaterTankMonitor.ino) implementa aquisição, OLED, rede local, painel, sessões de amostra, calibração por dois pontos, faixas experimentais e histórico. A [demonstração Wokwi](simulation/wokwi/README.md) usa controles sintéticos e permanece separada. A integração física ainda está em validação; não se trata de equipamento pronto para instalação residencial.

### Como deverá funcionar

```text
                     Amostras de água
                            │
                            ▼
           Turbidez · pH · Condutividade · Temperatura
                            │
                            ▼
                 Processamento das leituras
                            │
              ┌─────────────┼─────────────┐
              ▼             ▼             ▼
         Display local   Painel web    Histórico
                         via Wi-Fi    temporário
```

1. O equipamento será alimentado e iniciará a rotina de aquisição.
2. As leituras válidas serão exibidas no display e disponibilizadas no painel.
3. Para consultar pelo celular, o usuário se conectará à rede `WaterTankMonitor` e abrirá o endereço IP do equipamento no navegador.
4. Ao encerrar a consulta, poderá desconectar o celular sem interromper o funcionamento do sistema.

A meta dos ensaios é atualizar o painel e o display a cada **5 segundos**, respeitando o tempo de estabilização de cada sonda.

### Condições e limites de operação

| Aspecto | Comportamento previsto |
| :--- | :--- |
| **Acesso** | Local, pela rede Wi-Fi do equipamento; sem acesso remoto pela internet. |
| **Conectividade** | A rede do equipamento não fornecerá internet nem enviará notificações ao celular desconectado. |
| **Energia** | Sem alimentação elétrica, não haverá monitoramento. |
| **Histórico** | Será apagado ao reiniciar; calibração e configurações serão preservadas separadamente. |
| **Referência de tempo** | Tempo decorrido, como “há 10 min”, sem data ou hora de calendário. |
| **Medições de pH e condutividade** | Inicialmente realizadas em porções separadas da mesma amostra, para evitar interferência elétrica. |
| **Resultados antigos** | Serão identificados como “última amostra” ou “medição indisponível”, conforme a situação. |
| **Imersão permanente** | Dependerá de validação específica das sondas e da montagem. |

## Tecnologias e componentes

O [projeto elétrico de referência](hardware/README.md) reúne o mapa de conexões, a pinagem, o dimensionamento e a lista de materiais do protótipo. A revisão é preliminar e depende da confirmação dos módulos comprados e de ensaios de bancada.

| Categoria | Definição |
| :--- | :--- |
| Linguagens | **C/C++**, com sketches Arduino compilados como C++. O painel embarcado usa HTML, CSS e JavaScript. |
| Ambiente principal | **Arduino IDE 2.x**, com o pacote **esp32 by Espressif Systems**. |
| Plataforma | **Arduino Core para ESP32**; versão de referência dos testes de compilação: **2.0.17**. |
| Controlador | **ESP32-WROOM-32 em placa DevKit**, seleção `ESP32 Dev Module` para a placa compatível. |
| Comunicação | Wi-Fi local, HTTP/JSON, I²C para o OLED e 1-Wire para temperatura. |
| Armazenamento | Calibração e senha em **NVS**, via `Preferences`; médias em buffer circular na **RAM**. |
| Verificação | Compilação, Wokwi, Monitor Serial, multímetro e ensaios com referências. PlatformIO e Wokwi CLI são auxiliares opcionais. |
| Versionamento | **Git e GitHub**, com revisão e aprovação da equipe antes de publicar alterações. |

### Bibliotecas

| Biblioteca | Versão de referência | Função |
| :--- | :--- | :--- |
| Adafruit GFX Library | 1.12.6 | Desenha textos e elementos gráficos. |
| Adafruit SSD1306 | 2.5.16 | Controla o display OLED. |
| Adafruit BusIO | 1.17.4 | Dependência das bibliotecas Adafruit para comunicação com dispositivos. |
| OneWire | 2.3.8 | Implementa o barramento de dados do DS18B20. |
| DallasTemperature | 4.0.6 | Solicita conversões e interpreta a temperatura do DS18B20. |
| Wire | Incluída no pacote ESP32 | Comunicação I²C com o display. |
| WiFi | Incluída no pacote ESP32 | Cria a rede local `WaterTankMonitor`. |
| WebServer | Incluída no pacote ESP32 | Serve o painel e a API HTTP diretamente na placa. |
| Preferences | Incluída no pacote ESP32 | Preserva configurações após desligamento. |

As cinco bibliotecas externas já são usadas na demonstração. As três últimas são usadas no firmware real e vêm com o suporte oficial ao ESP32. As dependências mantêm suas próprias licenças.

### Componentes do protótipo

| Componente | Utilização |
| :--- | :--- |
| ESP32-WROOM-32 DevKit | Processamento, ADC e Wi-Fi. |
| Kit analógico/digital de turbidez, 5 V | Entrada analógica de turbidez relativa; a faixa anunciada em NTU exige calibração específica. |
| PH4502C com eletrodo BNC | Medição de pH após calibração e verificação. |
| TDS V1.0 com sonda | Entrada usada para estimativa experimental de EC; não fornece EC calibrada automaticamente. |
| DS18B20 encapsulado, três fios | Temperatura da amostra. |
| OLED SSD1306 I²C, 0,96", 128×64 | Consulta local. |
| Fonte isolada de 5 V / 3 A | Alimentação da montagem. |
| Divisores, filtros, pull-up e desacoplamento | Adequação dos sinais e alimentação; valores na [lista de materiais](hardware/bom.csv). |
| Protoboards, cabos, conectores e caixa | Montagem e proteção mecânica de bancada. |

Os títulos dos anúncios identificam as famílias dos módulos; a ordem dos pinos e a faixa elétrica da unidade recebida ainda devem ser conferidas. Isso é uma etapa de montagem, não uma escolha de tecnologia pendente.

## Primeiros passos

### Demonstração no Wokwi

![Circuito virtual do Water Tank Monitor no Wokwi](hardware/Circuito%20%282%29.png)

*Montagem no Wokwi: ESP32, OLED, DS18B20 e três controles que representam turbidez, pH e condutividade.*

Na montagem virtual, os controles permitem representar diferentes leituras dos sensores. O botão vermelho foi previsto para demonstrar uma falha de medição. Esta imagem apresenta as conexões.

[Abrir o guia da simulação](simulation/wokwi/README.md). A captura abaixo, fornecida pela equipe, mostra a demonstração exibindo valores simulados; não substitui os testes da versão física.

![Display da demonstração no Wokwi](hardware/Tela%20OLED.png)

### Pré-requisitos

- Git para clonar, ou download do ZIP pelo GitHub.
- Arduino IDE 2.x e suporte oficial ao ESP32.
- As cinco bibliotecas externas da tabela acima.
- Para a placa real: cabo USB de dados e driver compatível com o conversor USB da DevKit, se o sistema não reconhecer a porta.
- Para ensaios: componentes, multímetro e padrões de calibração adequados.

### 1. Clonar o repositório

```bash
git clone https://github.com/Jvscopel27/Water-Tank-monitor.git
cd Water-Tank-monitor
```

### 2. Preparar o ambiente

Na Arduino IDE, adicione esta URL às URLs adicionais do Gerenciador de Placas:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Instale **esp32 by Espressif Systems**, selecionando **2.0.17** para reproduzir o ambiente de referência. Instale as cinco bibliotecas externas pelo Gerenciador de Bibliotecas, com as versões listadas. Para uma atualização do pacote ESP32, repita os testes antes de adotá-la. [Instalação oficial](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)

Abra `firmware/WaterTankMonitor/WaterTankMonitor.ino`, selecione **ESP32 Dev Module** e a porta da placa. Clique em **Verificar** para compilar.

### 3. Montar e configurar o protótipo

Siga o [guia elétrico](hardware/README.md). GPIOs: temperatura **4**, pH **35**, turbidez **34**, TDS **32**, SDA **21** e SCL **22**. Confira os divisores e as tensões antes de conectar as entradas analógicas.

Grave inicialmente a DevKit isolada, alimentada somente pelo USB. No Monitor Serial a **115200 baud**, registre a senha gerada para a rede e o IP. Desconecte o USB antes de montar e alimentar o circuito pela fonte externa.

### 4. Executar e acessar o painel

Conecte o celular à rede **WaterTankMonitor**, usando a senha obtida durante a preparação. Abra no navegador o IP informado pela placa — por padrão, `http://192.168.4.1`. Não é necessário instalar aplicativo nem ter internet.

No painel, consulte leituras, idade dos dados e histórico. A área de configuração exige a senha da rede. Depois da estabilização das sondas, o comando `amostra 300` inicia uma sessão de cinco minutos; `parar` encerra a sessão. Sem sessão, pH e EC ficam identificados como última amostra ou indisponíveis.

Antes de usar os resultados, siga o [manual de calibração e operação](docs/OPERACAO.md). Sem calibração verificada, o firmware mostra os milivolts do ADC e sinaliza a pendência. Não há coeficientes de sensores inventados no código.

## Testes e validação

Consulte o [plano e registro de validação](docs/TESTES.md), que separa compilação, verificações de lógica, simulação e testes físicos. Há um cenário Wokwi preparado para a demonstração e verificações de interpolação, limites e idade das amostras.

A validação de bancada prevista no documento do projeto inclui:

- Testar cada sensor e o display individualmente.
- Calibrar e verificar as medições, registrando pelo menos três repetições por condição.
- Conferir o acesso ao painel e a desconexão e reconexão do celular sem interromper o equipamento.
- Verificar a indicação de leituras antigas, alterações e falhas de sensores.
- Testar a perda de energia, a preservação da calibração e o reinício do histórico vazio.
- Executar um ensaio de 24 horas da eletrônica e do display sem celular conectado, registrando separadamente as sessões com sondas.

Os resultados deverão registrar as condições de ensaio, as referências utilizadas e as limitações observadas. O ensaio de 24 horas não comprova a adequação das sondas à imersão permanente.

## Etapas do projeto

Sequência planejada de entregas, sem indicação de conclusão:

- [ ] Consolidar requisitos e revisar o esquema elétrico.
- [ ] Montar e testar sensores e display individualmente.
- [x] Preparar o código do painel acessível por Wi-Fi.
- [ ] Calibrar os sensores e integrar leituras reais.
- [x] Preparar histórico, estados do sistema e tratamento de falhas no firmware.
- [ ] Validar a operação independente do celular e o comportamento após reinício.
- [ ] Finalizar a montagem de bancada e registrar os ensaios.
- [ ] Entregar código, manual, relatório de calibração e demonstração.

**Período previsto no planejamento:** 5 de outubro a 1º de dezembro de 2026.

## Frame
![Carcaça inicial do modelo do produto](FRame/prototipo.png)
![Desenho técnico do modelo 3D do produto](FRame/Prototipo1.pdf)

## Como contribuir

Contribuições para a documentação, implementação e validação são bem-vindas.

1. Abra uma [issue](https://github.com/Jvscopel27/Water-Tank-monitor/issues) descrevendo a proposta ou o problema encontrado.
2. Faça um fork do repositório e crie uma branch para a alteração:

   ```bash
   git switch -c melhoria/descricao-da-alteracao
   ```

3. Faça as alterações e atualize a documentação relacionada.
4. Verifique o que for aplicável. Em mudanças de hardware ou medições, informe a montagem, as condições e os resultados dos ensaios.
5. Abra um pull request com o objetivo da mudança, o que foi alterado e como foi verificado.

Ao registrar resultados, identifique claramente dados simulados, leituras reais e medições ainda não validadas.

## Equipe

| Integrante | Contato |
| :--- | :--- |
| João Vitor Scopel | [GitHub — @Jvscopel27](https://github.com/Jvscopel27) |
| Isabela Freitas | [GitHub — @isabelafreitas123-ufes](https://github.com/isabelafreitas123-ufes) |
| Davi Saad | [GitHub — @davisaad-bot](https://github.com/davisaad-bot) |

Para dúvidas sobre o projeto, utilize as [issues do repositório](https://github.com/Jvscopel27/Water-Tank-monitor/issues).

## Licença

Este projeto está licenciado sob a **Licença MIT**. Consulte o arquivo [LICENSE](LICENSE) para conhecer os termos.
