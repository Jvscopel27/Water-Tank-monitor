# Verificação do software e do protótipo

## Revisão da documentação e publicação — 30/10/2026

- Reexecutado `tests/dashboard_test.py`: aprovados quatro formatos do OLED, gráficos, preservação de escolhas não salvas, captura automática uma única vez, cancelamento, recuperação de conexão, largura de celular e paginação do PDF.
- Conferidos os 47 links locais do README e dos guias em `docs/`, além da ausência de marcadores de conflito.
- Comparados com a versão remota integrada: arquivos de `frame/` e conteúdo da seção “Frame” do README preservados integralmente.
- Esta revisão não altera a aquisição do firmware nem acrescenta validação física das sondas. Os registros abaixo descrevem os ensaios anteriores.

## Gráficos e captura automática — 30/10/2026

- Compilação aprovada com 105.600 bytes de RAM estática (32,2%) e 915.089 bytes de flash (69,8%). Gravação na placa ESP32-D0WD concluída com hash verificado.
- API `/api/trend` testada na placa: leituras recentes de cinco segundos, máscara de simulação preservada e temperatura indisponível representada por `null`.
- Formato OLED 3 (gráfico) selecionado pelo site, com parâmetro turbidez. Após reset do microcontrolador, as preferências foram recuperadas sem alteração. A prévia do OLED e o gráfico web foram inspecionados visualmente usando os dados da placa.
- Reproduzido e corrigido: iniciar/encerrar uma sessão descartava uma escolha de formato ainda não salva. A regressão agora verifica que comandos não relacionados e reconexões preservam a prévia; o botão de restauração recupera apenas as preferências salvas.
- Testes de navegador com dados controlados: captura automática não grava antes de estabilidade; após o prazo mínimo e estabilidade, envia o ponto uma única vez. Alterar referência ou perder conexão cancela a tentativa. A recuperação da conexão preserva escolhas não salvas. Os formatos de OLED, página de celular e exportação PDF continuam passando.
- Testes na placa: formato inválido rejeitado, tentativa de calibração em canal simulado bloqueada, gráfico sem traço para temperatura ausente e preferência de gráfico persistente após reiniciar.

As verificações automáticas da calibração exercitam o fluxo e as proteções de software. Não foram usadas soluções de referência nem calibradas sondas reais nesta revisão. O comportamento do traço visível no display físico deve ser conferido pelo operador; a imagem inspecionada é a prévia do painel.

## Atualização de bancada — 29/10/2026

- Compilação final aprovada: 104.056 bytes de RAM estática (31,8%) e 900.573 bytes de flash de aplicação (68,7%). Upload concluído no ESP32-D0WD pela COM6; integridade da gravação conferida pelo esptool.
- OLED respondeu no endereço I²C 0x3C. Os três formatos fixos foram selecionados pelo navegador com resposta confirmada pela API; a aparência física deve ser conferida pelo operador.
- Página real carregada em Chromium: tema escuro, controles de OLED, autenticação e simulações. Parâmetros inválidos, senha incorreta, captura/calibração em canal simulado e ajuste de temperatura sem sensor foram recusados.
- Após a equipe encerrar alterações simultâneas no site, o reset do microcontrolador preservou exatamente tema, formato, foco, orientação, inversão, brilho, correção de temperatura e os três modos simulados. O relógio de atividade reiniciou e os potenciômetros voltaram ativos sem novo comando. Não foi simulado um corte de alimentação durante gravação NVS.
- Histórico real da bancada gerou médias com máscara de simulação 14 (pH/turbidez/EC). Temperatura indisponível permaneceu `null`. O PDF foi baixado pelo painel do ESP32, aberto e renderizado para revisão visual.
- `tests/dashboard_test.py`: testes com dados artificiais passaram para tema, recarga, três formatos de OLED, bloqueios visuais de calibração, tela de 390 px e download PDF. Históricos com 0, 60 e 1.440 registros geraram respectivamente 1, 4 e 55 páginas válidas. Foram inspecionados layout, margens, textos, tabelas e identificação SIM/REAL.
- As 15 verificações C++ de interpolação, limites, idade e índice circular continuam aprovadas.

**Limites:** a calibração com sondas reais, referências rastreáveis, exatidão, interferência, desconexão de sensores e estabilidade de 24 horas continuam pendentes. Bloqueios de software não equivalem a calibração física. O histórico continua em RAM, embora configurações persistam.

Para reproduzir os testes de navegador no PC de desenvolvimento (Chrome instalado):

```powershell
python -m pip install playwright pymupdf pypdf
python tests/dashboard_test.py --output build\qa-dashboard
```

O script usa o caminho padrão do Chrome no Windows; ajuste-o para outro ambiente. Os dados do teste são artificiais. Essas ferramentas não são dependências do ESP32 nem da Arduino IDE. Não versione PDFs de teste, capturas com credenciais ou diretórios de compilação.

## Registro inicial — 22/10/2026

| Verificação | Estado |
|---|---|
| Diagrama Wokwi | Validação anterior sem erros; duas mensagens informativas do catálogo para os modelos ESP32/DS18B20. |
| Imagem de execução da demonstração | A equipe forneceu `hardware/Tela OLED.png`, com leituras sintéticas exibidas no OLED. |
| Verificações de interpolação/faixas/idade/índice circular | 15 verificações `static_assert` aprovadas pelo compilador C++ do ESP32. |
| Compilação da demonstração | Aprovada com PlatformIO 6.2.0, plataforma espressif32 6.12.0 e Arduino Core 2.0.17. |
| Compilação do firmware de bancada | Aprovada no mesmo ambiente; não foi realizado upload para hardware. |
| JavaScript do painel | Sintaxe aprovada pelo parser Esprima; não equivale a teste de interação no navegador. |
| Cenário de interação no Wokwi CLI | Preparado, ainda não executado nesta sessão; requer acesso ao simulador/credencial local. |
| Testes físicos e ensaio de 24 horas | Pendentes. |

O arquivo `tests/measurement_compile_test.cpp` verifica a lógica compartilhada em `Measurement.h`. As verificações rodam em tempo de compilação e não provam operação dos periféricos ou persistência em flash.

O compilador emitiu dois avisos em diretivas `#undef` da dependência OneWire 2.3.8; não houve erro de compilação. A biblioteca externa não foi alterada. As versões foram fixadas para tornar a revisão reproduzível.

## Reproduzir a compilação

Na Arduino IDE, compile cada sketch usando o pacote ESP32 e versões das bibliotecas informados no README:

- `simulation/wokwi/sketch/sketch.ino` — demonstração.
- `firmware/WaterTankMonitor/WaterTankMonitor.ino` — bancada real.

Opcionalmente, com Python e PlatformIO Core instalados:

```bash
python -m platformio run --project-dir simulation/wokwi
python -m platformio run --project-dir firmware
```

Os arquivos `platformio.ini` fixam as mesmas versões de bibliotecas e o Arduino Core 2.0.17. A versão 6.12.0 nesses arquivos é a plataforma do PlatformIO, não a versão do Arduino Core.

Para as verificações de lógica, execute da raiz do repositório com um compilador C++11 ou superior:

```bash
g++ -std=c++11 -fsyntax-only tests/measurement_compile_test.cpp
```

Nesta revisão foi usado `xtensa-esp32-elf-g++` com os mesmos argumentos. Erros de `static_assert` interrompem a compilação.

## Simulação

O cenário em `simulation/wokwi/simulation.test.yaml` verifica a inicialização do OLED, mudança de temperatura, resposta dos três controles, falha e recuperação. Consulte o [guia Wokwi](../simulation/wokwi/README.md) para execução. O firmware real não deve ser testado com o diagrama de potenciômetros como se eles fossem sondas calibradas.

## Roteiro de aceitação em bancada

1. **Inicialização sem calibração:** sensores analógicos devem indicar pendência, com tensão bruta disponível.
2. **Entrada inválida:** tentar pontos iguais/próximos, tensão fora da faixa e comando longo. A configuração válida anterior deve permanecer.
3. **Calibração:** registrar três repetições por condição e verificar uma terceira referência. Confirmar `validar` somente após o ensaio.
4. **Limites:** verificar leitura nos pontos de calibração e recusa de extrapolação.
5. **Sessão:** iniciar, encerrar e deixar expirar. O valor anterior deve ficar marcado como última amostra e não entrar em médias futuras.
6. **EC:** testar a faixa de temperatura aceita e fora dela; não exibir EC atual quando a temperatura for inválida.
7. **Falha de temperatura:** desconectar/reconectar entre ensaios com a alimentação desligada. Registrar indisponibilidade e recuperação.
8. **OLED:** iniciar sem display e verificar que o painel e a coleta continuam disponíveis.
9. **Rede:** fechar a página e desconectar o celular; confirmar que o histórico continua. Reconectar e conferir idade dos dados.
10. **Senha:** comando do painel com senha errada deve ser recusado, sem alterar sessão/calibração.
11. **Persistência:** reiniciar e conferir calibração/faixas/senha, sessão encerrada e histórico vazio.
12. **Histórico:** conferir médias e contagens por canal, lacunas como `null`, reinício ao recalibrar e substituição circular após 1.440 minutos.
13. **Carga:** exportar histórico enquanto coleta; medir atraso, memória livre e estabilidade com cliente lento.
14. **Faixas:** confirmar que só três leituras consecutivas fora das faixas experimentais geram alteração persistente.
15. **24 horas:** registrar resets, falhas, consumo e condições. Separar a operação da eletrônica das sessões de uso das sondas.

Os resultados devem conter revisão do código, identificação dos módulos, instrumentos, condições, valores de referência e critérios de aceitação definidos pela equipe. Nenhum teste é marcado como concluído apenas porque foi escrito neste roteiro.
