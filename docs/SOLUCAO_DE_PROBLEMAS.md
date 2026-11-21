# Diagnóstico rápido

| Sintoma | Verificação e próximo passo |
|---|---|
| Nenhuma porta USB | Use cabo de dados e outra porta; confira o driver da interface USB da placa |
| Porta ocupada | Feche outro Monitor Serial antes de gravar ou ler |
| Permissão negada no Linux | Confira o acesso do seu usuário à porta serial; a permissão pode mudar ao reconectar |
| Wi-Fi “sem internet” | Esperado: permaneça conectado e abra http://192.168.4.1 |
| Site antigo após atualizar | Ctrl+F5 ou limpar cache da página |
| Calibração pendente | Cadastre e habilite a curva na própria placa; arquivo JSON no PC não a instala |
| Fora da faixa ADC | Confira campo mV, alimentação, GND, pino analógico e divisor; faixa do firmware 150–2300 mV |
| Fora da faixa calibrada | A tensão ultrapassou seus pontos; a aquisição continua. Não altere o trimpot para forçar um valor sem recalibrar |
| Sonda recolocada não mostra valor | Aguarde algumas leituras e veja mV/estado; não é preciso iniciar uma sessão |
| Captura não conclui | Evite bolhas, movimentação e mudanças de iluminação; o assistente exige quatro leituras dentro de 10 mV |
| EC sem temperatura | A versão atual não exige DS18B20; verifique sinal, calibração e unidade µS/cm |
| OLED apagado | Confira SDA 21, SCL 22, 3V3 e GND; comando `display` procura o dispositivo I²C |

Ao pedir ajuda, informe modelo da placa/módulo, GPIO, alimentação, divisor e leituras em mV. Não publique a saída de `rede`, senhas, tokens ou backups de flash.
