# Operação e calibração de bancada

## Preparação

1. Instale as versões do pacote ESP32 e bibliotecas indicadas no README.
2. Abra `firmware/WaterTankMonitor/WaterTankMonitor.ino` na Arduino IDE.
3. Selecione `ESP32 Dev Module` para a DevKit compatível, escolha a porta e compile.
4. Com a DevKit isolada do circuito e alimentada somente pelo USB, grave o programa.
5. Abra o Monitor Serial a 115200 baud, com fim de linha. Digite `rede` para consultar senha e IP. A senha é gerada automaticamente e preservada em NVS; não a publique no GitHub.
6. Desconecte o USB. Com tudo desligado, conecte a montagem conforme o projeto elétrico. Energize pela fonte externa.
7. No celular, conecte-se à rede `WaterTankMonitor` com a senha registrada. Abra o IP da placa. Se o celular avisar que a rede não tem internet, mantenha a conexão local.

Não é necessário instalar bibliotecas `WiFi`, `WebServer`, `Preferences` ou `Wire` de terceiros: elas vêm no pacote ESP32. O endereço I²C do OLED no código é `0x3C`; se a unidade for `0x3D`, ajuste as duas ocorrências antes de gravar.

## Comandos

O painel oferece controles de tela, demonstração e calibração guiada. Informe a senha da rede em “Acesso às configurações”; ela permanece apenas na página aberta, sem `localStorage`, e pode ser apagada pelo botão “Limpar senha”. Os comandos abaixo continuam disponíveis em “Comandos avançados” e pelo Monitor Serial, quando a alimentação permitir o uso seguro do USB.

| Comando | Efeito |
|---|---|
| `ajuda` | Lista comandos no Monitor Serial. |
| `rede` | Exibe credenciais e IP somente no Monitor Serial. |
| `senha NOVA_SENHA` | Somente por USB/Serial: salva uma senha de 8 a 63 caracteres ASCII imprimíveis e reinicia a placa. Também altera a senha de comandos do painel. Atualize a senha da rede nos dispositivos conectados. Espaços nas extremidades são removidos. |
| `display` | Somente por USB/Serial: lista endereços I²C que respondem em SDA 21/SCL 22 e informa se o OLED foi inicializado. Não confirma visualmente os pixels. Após mudar a ligação com a placa desligada, reinicie-a. |
| `amostra 300` | Inicia uma sessão de cinco minutos para pH e EC. O prazo aceito vai de 30 a 1.800 segundos. |
| `parar` | Encerra a sessão imediatamente. |
| `demo CANAL on` / `demo CANAL off` | CANAL: `ph`, `turb`, `ec` ou `todos`. GPIO 35, 34 e 32, respectivamente. Escalas sintéticas: pH 0–14, turbidez 0–100%, EC 0–2.000 µS/cm. O modo é salvo em NVS e volta após reiniciar. Cada troca limpa o histórico; médias simuladas são registradas com máscara de origem, identificadas como SIM no painel/PDF e excluídas da classificação da água. Temperatura permanece real. |
| `ui TEMA FORMATO FOCO GIRO INVERSAO BRILHO` | Configurações salvas juntas em NVS. Tema 0/1, formato 0 lista/1 grade/2 destaque/3 gráfico, foco 0 temperatura/1 pH/2 turbidez/3 EC, giro 0/2, inversão 0/1, brilho 32–255. Prefira os controles visuais do painel. |
| `preparar CANAL` | Reinicia a janela de estabilidade do canal real antes da captura (`ph`, `turb`, `ec` ou `temp`). O assistente automático envia este comando ao iniciar cada etapa. |
| `capturar CANAL PONTO REFERENCIA` | PONTO 1 ou 2. Média de quatro leituras com variação até 10 mV, ADC 150–2.300 mV. O ponto 1 expira em 30 minutos e é apenas RAM. O ponto 2 salva uma curva ainda não verificada. EC exige temperatura real de 25 ±0,5 °C. |
| `verificar CANAL REFERENCIA TOLERANCIA` | Compara a curva com uma terceira referência intermediária, diferente dos pontos de ajuste. A tolerância experimental deve ser positiva e até 20% do intervalo de referências. Habilita a curva somente se o erro ficar dentro da tolerância. |
| `tempcal REFERENCIA` | Ajusta uma correção constante do DS18B20, limitada a ±5 °C, a partir de quatro amostras reais estáveis (variação até 0,3 °C). Referência entre −10 e 85 °C. Salva em NVS e limpa o histórico. Conferir depois em outra temperatura; não é certificação metrológica. |
| `cal CANAL mV1 REF1 mV2 REF2` | Salva dois pontos de calibração. CANAL pode ser `ph`, `turb` ou `ec`. A leitura permanece pendente de verificação. |
| `validar CANAL` | Registra que o operador verificou a calibração com uma referência independente. Não realiza essa verificação automaticamente. |
| `faixa CANAL MIN MAX` | Define a faixa experimental de comparação. Não é um critério sanitário. |

Substitua os nomes em maiúsculas por valores medidos. Use ponto decimal, espaços entre argumentos e uma linha por comando. Os milivolts são medidos **no ADC, depois do divisor**, não na saída original do módulo.

Calibrações e faixas são persistidas apenas quando alteradas. Cada alteração reinicia o histórico em RAM para não misturar resultados calculados com critérios diferentes. Se a gravação em NVS falhar, o comando informa erro e mantém a configuração anterior.

## Procedimento de calibração

1. Verifique a alimentação e compare a tensão exibida no painel com o multímetro nos pontos TP1–TP3. Não conecte saídas de 5 V diretamente aos GPIOs.
2. Registre a tensão estabilizada de cada uma de duas referências, repetindo cada condição pelo menos três vezes.
3. Cadastre os pontos com `cal`. O firmware exige 150–2.300 mV e separação mínima de 20 mV, para evitar uma curva baseada em diferenças pequenas demais.
4. Verifique uma terceira referência independente, na faixa delimitada pelos dois pontos, registrando erro e repetibilidade. Só depois envie `validar`.
5. Defina as faixas experimentais aprovadas para o ensaio com `faixa`.
6. Aguarde a estabilização das sondas, inicie a sessão de amostras e registre os resultados.

**pH:** use tampões que abranjam a faixa de trabalho. A interpolação inclui o conjunto sonda/módulo/divisor/ADC e aceita inclinação positiva ou negativa. Não há compensação automática de temperatura do pH.

**Turbidez:** duas referências definem uma escala relativa de 0 a 100, com unidades `% rel.`. Não são NTU. Escolha e documente as condições das referências, inclusive iluminação, geometria e limpeza da sonda.

**EC:** a aproximação linear é válida apenas no intervalo que foi verificado. Use referências adequadas ao módulo e mantenha a amostra em 25 ± 0,5 °C. Não é aplicado fator universal de TDS para EC nem coeficiente de compensação térmica presumido. Fora da temperatura permitida, o canal fica indisponível. Se a verificação indicar não linearidade relevante, não confirme a calibração: revise a curva ou o módulo.

O DS18B20 deve representar a temperatura da amostra medida; a posição do sensor faz parte do procedimento experimental. pH e EC usam porções separadas da mesma coleta, conforme o planejamento. O software não detecta se o operador retirou uma sonda da água nem confirma fisicamente que uma amostra foi estabilizada.

## Estados de leitura

| Estado | Significado |
|---|---|
| `atual` | Leitura válida dentro das condições configuradas. |
| `sem_sessao` | Não há sessão ativa para pH/EC nem amostra anterior disponível. |
| `ultima_amostra` | Sessão encerrada; o valor preservado não é apresentado como atual. |
| `calibracao_pendente` | Não há pontos válidos ou a verificação ainda não foi confirmada. |
| `fora_faixa_adc` | Tensão fora da janela escolhida para aquisição. Não prova a causa da falha. |
| `fora_faixa_calibrada` | Resultado exigiria extrapolar os pontos; o firmware recusa fazê-lo. |
| `ec_exige_25C` | Temperatura incompatível ou indisponível para esta estimativa de EC. |
| `sensor_indisponivel` | Não foi obtida temperatura válida. |
| `leitura_antiga` | Não há atualização válida nos últimos 15 segundos. |

O OLED usa uma tela fixa, sem alternância: lista, grade, destaque ou gráfico, selecionados no site. Também é possível escolher foco, brilho, giro de 180° e inversão. Valores indisponíveis ficam `--`; `*` indica simulação. O site mostra estado e idade de cada leitura e identifica perda de conexão. O tema claro/escuro e o formato do OLED são compartilhados pela placa e persistem após reiniciar; a prévia só é persistida ao clicar em “Salvar tema e tela”.

Não existe diagnóstico de potabilidade. A comparação de faixas só é habilitada quando todas as leituras estão atuais e as três faixas analógicas foram configuradas. Três aquisições consecutivas fora da faixa geram indicação de alteração persistente; uma leitura válida dentro da faixa reinicia a contagem.

## Histórico, memória e rede

- Médias de um minuto, com contagem de leituras válidas por canal. Um canal sem valores válidos naquele minuto fica `null`, não zero.
- Até 1.440 entradas. Registros mais antigos que 24 horas não são exportados.
- A página mostra as 60 médias mais recentes. “Baixar relatório PDF” gera um arquivo vetorial offline no navegador, com leituras atuais, estado, identificação SIM/REAL, min/max, minigráficos e todas as médias em páginas de tabela. Sem histórico, gera um resumo atual sem inventar registros. JSON permanece apenas como API interna.
- Cada média contém a origem simulada por canal. Valores simulados nunca são usados para classificar a água.
- Reinicialização e alteração de calibração/faixas/modos apagam o histórico em RAM. Calibração, faixas, senha, tema, formato do OLED, correção de temperatura e modos simulados sobrevivem à perda de energia.
- O tempo vem do relógio monotônico da placa; não há data de calendário, NTP ou RTC externo.
- A data de emissão do PDF vem do navegador e é identificada como tal. As médias usam idade relativa à consulta; não são carimbos de data do ESP32.
- O painel funciona sem CDN, fontes externas ou internet. Consultas são HTTP local. A senha da rede também protege os comandos de alteração; não exponha o equipamento diretamente à internet.

O servidor é síncrono e foi escolhido pela simplicidade do protótipo. Downloads de histórico e clientes lentos podem atrasar temporariamente o atendimento e a aquisição; esse comportamento deve ser medido nos ensaios. Se o atraso for relevante, a próxima revisão deve limitar/paginar a exportação ou adotar execução separada após avaliação.

## Ainda depende de ensaio

Ruído com Wi-Fi ativo, margem do regulador, cabo real do DS18B20, precisão das curvas, reconexão dos sensores, persistência após desligamento, comportamento de clientes lentos e estabilidade de 24 h. Uma saída analógica em falha pode assumir tensão plausível: os testes de faixa não detectam todas as falhas.
