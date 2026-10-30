# Painel e OLED — revisão de 30/10/2026

## Uso

1. Conecte ao Wi-Fi `WaterTankMonitor` e abra `http://192.168.4.1`.
2. Para modificar a placa, informe a senha atual em **Acesso às configurações**. A senha não é incluída no código nem no PDF.
3. Em **Tela OLED**, escolha tema claro/escuro e formato fixo: lista, grade, destaque ou **gráfico de um parâmetro**. O seletor de parâmetro controla o destaque e o gráfico. Configure brilho, inversão e giro se necessário. Clique em **Salvar tema e tela**. A prévia sozinha não altera a placa; escolhas não salvas são preservadas ao usar outros controles. “Restaurar escolhas salvas” descarta apenas a prévia.
4. Em **Potenciômetros**, habilite pH (GPIO 35), turbidez (34) e EC (32). A seleção é persistente. Temperatura continua real no DS18B20 porque a montagem informada não tem um quarto potenciômetro.
5. Em **Histórico e relatório**, aguarde pelo menos um minuto para formar uma média. O botão de PDF baixa um arquivo com resumo, identificação de simulação, gráficos e todas as médias. Sem histórico, o relatório contém apenas o resumo atual.

O histórico é RAM e reinicia com a placa. Tema, OLED, modos de simulação, calibrações e senha são armazenados na flash/NVS e não dependem do navegador permanecer aberto. Ao alterar calibração ou modo, o histórico é limpo para não misturar critérios.

## Gráficos no painel e no OLED

A seção **Gráficos** e o formato de gráfico do OLED usam a mesma série de até 48 leituras, com uma nova leitura a cada cinco segundos (aproximadamente quatro minutos). A série fica na placa, portanto não depende de manter uma página aberta. A escala vertical se ajusta aos valores e exibe limites; lacunas ou intervalos maiores que 15 segundos não são unidos por uma linha. Temperatura sem sensor fica sem traço; não é substituída por zero.

A série rápida é independente das médias de um minuto do histórico/PDF. Ambas são apagadas ao reiniciar ou trocar modo/calibração. A escolha do formato de gráfico e do parâmetro permanece em NVS. Dados simulados continuam identificados e não classificam a qualidade da água.

## Calibração guiada

O tutorial específico de cada sensor fica no próprio painel e funciona offline. Desligue a simulação do canal, confirme a preparação física e use referências conhecidas. A captura exige quatro leituras estáveis, média no intervalo elétrico permitido e referências distintas. EC também exige temperatura real de 25 ±0,5 °C.

Agora basta informar a referência e iniciar **Capturar automaticamente**. O assistente limpa a janela de amostras, aguarda no mínimo 20 segundos e envia a captura uma única vez quando estabilidade e condições elétricas/térmicas forem satisfeitas. O progresso mostra quantidade de amostras e variação observada. Após três minutos sem estabilidade, a tentativa termina com orientação de revisão. O tempo físico de equilíbrio da sonda pode ser maior.

O botão **Cancelar captura**, a troca de referência/canal, a perda de conexão, a reinicialização do ESP32 e a página em segundo plano cancelam a tentativa. A fila é temporária no navegador e não volta automaticamente após recarregar. A curva já salva permanece na placa. Após erro de comunicação na gravação, confira o estado antes de repetir: o servidor pode ter concluído a operação mesmo sem a resposta chegar ao navegador.

A confirmação automática por estabilidade se inspira no fluxo descrito pela [Hanna Instruments](https://knowledge.hannainst.com/en/knowledge/generalized-calibration-ph-electrode-meter). Ela não substitui referências frescas, enxágue e preparação da sonda. Não tentamos identificar automaticamente o valor de tampões com um módulo PH4502C ainda sem calibração.

O primeiro ponto fica temporariamente em RAM (até 30 minutos). O segundo salva a curva em NVS, ainda pendente. Uma terceira referência intermediária pode ser comparada automaticamente com a tolerância experimental informada; a curva é habilitada se essa comparação passar. Isso não comprova a identidade da solução, a precisão do instrumento de referência ou a instalação da sonda. Esses passos continuam físicos.

Para temperatura, o painel calcula uma correção constante do DS18B20 em relação ao termômetro de referência, limitada a ±5 °C. Depois é necessário conferir outra temperatura. Este procedimento não constitui certificação metrológica.

## Ferramentas e decisões desta revisão

- **CSS com variáveis:** permite os temas sem framework e sem downloads externos.
- **Adafruit GFX/SSD1306:** as mesmas bibliotecas desenham uma tela fixa. Não foi adicionada biblioteca de display.
- **Preferences/NVS:** um registro versionado guarda as preferências; elas só são aplicadas se a gravação e a validação forem bem-sucedidas. Não há gravação de leituras a cada atualização na flash.
- **PDF vetorial em JavaScript:** texto, caixas, gráficos e tabelas são montados no navegador, com fontes PDF padrão. O ESP32 só fornece a página e os dados. Dispensa CDN, serviço externo e uma biblioteca grande na flash.
- **Playwright e PyMuPDF/pypdf, somente para desenvolvimento:** verificam o painel em navegador e renderizam/conferem relatórios. Não são instalados no ESP32 e não alteram o ambiente principal Arduino IDE.

O JSON continua disponível internamente para o painel; a ação de download apresentada ao usuário agora é PDF. Os registros simulados levam uma máscara de origem por canal e a marca SIM no relatório. Não participam da classificação da água.

Os testes de bancada e de navegador estão registrados em [TESTES.md](TESTES.md). Para reproduzir a instalação, siga [Comece aqui](COMECE_AQUI.md).
