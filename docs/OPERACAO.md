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

Comandos de operação/calibração podem ser enviados pela área “Configuração de bancada” do painel, usando a senha da rede. A página remove a senha do campo após cada envio e não a armazena em `localStorage`. Também há suporte pelo Monitor Serial quando a configuração de alimentação permitir usá-lo com segurança.

| Comando | Efeito |
|---|---|
| `ajuda` | Lista comandos no Monitor Serial. |
| `rede` | Exibe credenciais e IP somente no Monitor Serial. |
| `amostra 300` | Inicia uma sessão de cinco minutos para pH e EC. O prazo aceito vai de 30 a 1.800 segundos. |
| `parar` | Encerra a sessão imediatamente. |
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

No OLED, valores não atuais são substituídos por `--`. A tela alternada mostra a idade da última leitura; detalhes do estado ficam no painel. No painel, o último valor pode continuar visível, sempre acompanhado de estado e idade. Ao perder a conexão, a página identifica os dados como antigos.

Não existe diagnóstico de potabilidade. A comparação de faixas só é habilitada quando todas as leituras estão atuais e as três faixas analógicas foram configuradas. Três aquisições consecutivas fora da faixa geram indicação de alteração persistente; uma leitura válida dentro da faixa reinicia a contagem.

## Histórico, memória e rede

- Médias de um minuto, com contagem de leituras válidas por canal. Um canal sem valores válidos naquele minuto fica `null`, não zero.
- Até 1.440 entradas. Registros mais antigos que 24 horas não são exportados.
- A página mostra as 60 médias mais recentes; o download JSON contém todo o histórico disponível.
- Reinicialização e alteração de calibração/faixas apagam o histórico. Calibração, faixas e senha sobrevivem à perda de energia.
- O tempo vem do relógio monotônico da placa; não há data de calendário, NTP ou RTC externo.
- O painel funciona sem CDN, fontes externas ou internet. Consultas são HTTP local. A senha da rede também protege os comandos de alteração; não exponha o equipamento diretamente à internet.

O servidor é síncrono e foi escolhido pela simplicidade do protótipo. Downloads de histórico e clientes lentos podem atrasar temporariamente o atendimento e a aquisição; esse comportamento deve ser medido nos ensaios. Se o atraso for relevante, a próxima revisão deve limitar/paginar a exportação ou adotar execução separada após avaliação.

## Ainda depende de ensaio

Ruído com Wi-Fi ativo, margem do regulador, cabo real do DS18B20, precisão das curvas, reconexão dos sensores, persistência após desligamento, comportamento de clientes lentos e estabilidade de 24 h. Uma saída analógica em falha pode assumir tensão plausível: os testes de faixa não detectam todas as falhas.
