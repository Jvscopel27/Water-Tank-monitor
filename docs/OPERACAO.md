# Operação e calibração

## Antes de medir

A aquisição é contínua a cada 5 segundos; não depende de temperatura, sessão ou navegador. Os estados distinguem sinal fora da faixa ADC, fora da curva e calibração pendente. Sinais inválidos não são convertidos em medidas válidas; ao voltar à faixa, a próxima leitura válida é exibida automaticamente.

Use `demo todos off` com sensores reais. O canal `ec` permanece em **µS/cm**, sem compensação térmica. Nenhuma curva de EC existente é convertida para porcentagem.

## Calibrar pelo site

1. Escolha o sensor e confirme a preparação da referência.
2. Coloque a primeira referência, informe seu valor e clique em capturar ponto 1.
3. O assistente espera 20 segundos e quatro leituras com variação de até 10 mV.
4. Enxágue quando aplicável, coloque a segunda referência e capture ponto 2. O primeiro ponto fica na RAM por 30 minutos; reiniciar o ESP32 o apaga.
5. A curva é persistida, inicialmente pendente. Confira uma referência intermediária e a tolerância para habilitá-la.

A terceira referência do assistente verifica a curva de dois pontos; não adiciona um terceiro nó. O firmware também lê curvas de pH com três pontos já salvas, como a bancada do proprietário. Uma nova calibração de dois pontos substitui essa curva.

## Referências e unidades

- **pH:** usar referências com valores conhecidos fornece uma calibração rastreável. Os valores caseiros estimados/atribuídos da bancada estão documentados separadamente.
- **Turbidez:** defina água de referência como 0% e uma amostra reproduzível como 100%. É percentual relativo; não NTU.
- **Condutividade:** referências precisam ter valor atribuído em µS/cm. Água filtrada e água com sal podem testar resposta, mas não determinam sozinhas valores exatos nessa unidade. Sem sensor de temperatura, não há correção térmica: compare em condições semelhantes e registre as condições conhecidas.

## Comandos Serial (115200 baud, nova linha)

| Comando | Função |
|---|---|
| `rede` | Mostra rede, senha e IP; manter privado |
| `demo todos off` | Ativa os três canais reais |
| `preparar ec` | Reinicia a janela de estabilidade para condutividade |
| `capturar ec 1 1413` | Exemplo: ponto 1, somente se a referência usada realmente for 1413 µS/cm |
| `capturar ec 2 VALOR` | Ponto 2; substituir VALOR pela referência real |
| `verificar ec REFERENCIA TOLERANCIA` | Verifica referência intermediária |
| `cal turb MV_ZERO 0 MV_MAXIMO 100` | Define diretamente os dois pontos em mV no GPIO |
| `validar turb` | Habilita manualmente uma curva já cadastrada, por decisão do operador |
| `calinfo` | Consulta os três pontos de pH atualmente salvos |
| `amostra 300` / `parar` | Inicia/encerra apenas um marcador; aquisição continua |

A definição direta permite usar médias registradas mesmo quando a captura automática recusa instabilidade. Documente as leituras, média e dispersão: salvar uma média não elimina a oscilação. O projeto exige pontos entre 150 e 2300 mV, separados por pelo menos 20 mV e com referências diferentes.

## Persistência

Calibrações e preferências ficam na NVS. O histórico e os pontos recentes ficam em RAM: reinício, troca de modo ou calibração os limpam. Atualizar o firmware não deve incluir apagar toda a flash sem backup. Nunca publique backups de flash/NVS: podem conter senhas.
