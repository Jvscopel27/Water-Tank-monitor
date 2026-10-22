# Verificação do software e do protótipo

## Registro desta revisão

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
