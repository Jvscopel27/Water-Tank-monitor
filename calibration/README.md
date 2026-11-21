# Calibração experimental do protótipo

Firmware substituído em 20/11/2026.

pH: vinagre 1699,969 mV → 3,1; água filtrada 1425,375 mV → 6,3; bicarbonato 1253,516 mV → 8,3. Referências caseiras estimadas/atribuídas pelo usuário. Interpolação linear por trechos, usando mV no GPIO 35 após divisor de dois resistores de 6,8 kΩ. Não mudar offset ou divisor sem recalibrar.

O firmware migra a estrutura NVS antiga sem perder os extremos e importa o terceiro ponto uma única vez (`ph3_imported`). A consulta Serial `calinfo` mostra os três pontos ativos. Uma nova calibração de dois pontos substitui a curva de três pontos; o terceiro ponto antigo não é reaplicado automaticamente.

O gráfico exibe linha, último valor e amplitude (máximo menos mínimo), sem marcadores ou contagem de pontos. A aquisição é contínua desde a inicialização, sem expirar por inatividade. Os comandos `amostra` e `parar` apenas controlam um marcador de tempo e não interrompem leituras. A turbidez mantém sua calibração anterior. EC requer sua própria calibração; a revisão atual não exige temperatura.

Validação: compilação PlatformIO, testes C++ da interpolação, verificações estáticas anteriores, sintaxe JavaScript e gravação com hash verificado. Logs privados de verificação em `build/replacement-*.log`.

## Turbidez — calibração oficial do projeto

Concluída em 20/11/2026 por autorização expressa para usar a média apesar da oscilação. Água filtrada: média das seis leituras registradas, 1789,250 mV = 0%. Café: média de 12 leituras novas, 1050,567 mV = 100%. Escala linear relativa, não NTU. Registro completo em `turbidity-official.json`. Firmware confirmou gravação e habilitação; após reiniciar, 1051,3 mV resultou em 99,90% (estado atual). A calibração de pH foi preservada.

O café variou de 956,4 a 1110,2 mV durante a coleta. A média define a referência, mas não remove a oscilação. O firmware mantém seu comportamento de indicar fora da faixa calibrada para sinais além dos extremos.

## Aquisição contínua

Atualização de 20/11/2026: removidos os bloqueios por sessão em aquisição, atualidade e estado de pH/EC. O painel explica o acompanhamento contínuo. Sinais fora dos extremos continuam sendo recebidos em mV, com estado fora da faixa calibrada; a próxima leitura válida retorna automaticamente ao gráfico. Nenhum ponto de calibração foi alterado nesta atualização.

## Revisão de 21/11/2026

Temperatura removida do firmware, painel, OLED e PDF. Condutividade permanece em µS/cm, sem dependência nem compensação térmica. As calibrações de pH/turbidez permanecem válidas como referências operacionais da bancada. O importador automático específico desta bancada foi retirado do firmware público; curvas já gravadas são mantidas. Novas placas devem ser calibradas separadamente.
