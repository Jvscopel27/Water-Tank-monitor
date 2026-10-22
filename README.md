<div align="center">

# 💧 Water Tank Monitor

**Monitoramento de parâmetros da água para apoiar o cuidado com reservatórios residenciais.**

![Status: em desenvolvimento](https://img.shields.io/badge/status-em_desenvolvimento-0284C7?style=flat-square)
![Projeto acadêmico](https://img.shields.io/badge/projeto-acad%C3%AAmico-0F766E?style=flat-square)
![Escopo: protótipo de bancada](https://img.shields.io/badge/escopo-prot%C3%B3tipo_de_bancada-475569?style=flat-square)
[![Licença: MIT](https://img.shields.io/badge/licen%C3%A7a-MIT-16A34A?style=flat-square)](LICENSE)

[Sobre](#sobre-o-projeto) · [Funcionalidades](#funcionalidades-planejadas) · [Primeiros passos](#primeiros-passos) · [Contribuição](#como-contribuir) · [Equipe](#equipe)

</div>

---

## Sobre o projeto

O **Water Tank Monitor** é um projeto acadêmico de um dispositivo de baixo custo para acompanhar **turbidez, pH, condutividade elétrica e temperatura** da água. A proposta é disponibilizar as leituras em um display local e em uma interface acessível pelo navegador do celular, via Wi-Fi.

O projeto busca apoiar a inspeção de caixas d'água residenciais e a limpeza preventiva, tornando alterações nos parâmetros medidos mais fáceis de acompanhar.

A entrega prevista é um **protótipo de bancada**. A instalação permanente em uma residência dependerá da validação dos sensores, dos materiais, da vedação e das condições de operação.

> **Em desenvolvimento:** este README descreve o escopo planejado. As funcionalidades ainda deverão ser implementadas e validadas; as tecnologias e os procedimentos de execução serão documentados conforme o projeto avançar.

## O que será monitorado

| Parâmetro | O que permite acompanhar | Como será apresentado |
| :--- | :--- | :--- |
| **Turbidez** | Alterações relacionadas à presença de partículas em suspensão. | Indicador relativo, comparado com uma referência de ensaio. |
| **pH** | Acidez ou alcalinidade da água. | Leitura após calibração e verificação da sonda. |
| **Condutividade elétrica** | Resposta relacionada aos íons dissolvidos na água. | Valor estimado em µS/cm a 25 °C, após calibração e verificação. |
| **Temperatura** | Condição térmica da amostra. | Informação auxiliar para interpretar as medições. |

> **Limite do projeto:** o dispositivo fará uma triagem dos parâmetros medidos e **não confirmará a potabilidade da água**. Leituras sem alterações não significam que a água esteja segura para beber. Essa limitação faz parte do escopo definido no documento do projeto.

## Funcionalidades planejadas

- **Leitura de quatro parâmetros:** turbidez, pH, condutividade elétrica e temperatura.
- **Painel web local:** consulta das últimas leituras pelo navegador do celular.
- **Rede Wi-Fi própria:** acesso à rede `WaterTankMonitor`, protegida por senha.
- **Display no equipamento:** exibição alternada das leituras, do estado do sistema e do tempo desde a medição.
- **Operação independente do celular:** a coleta e a exibição continuarão enquanto o equipamento estiver alimentado, mesmo com o navegador fechado.
- **Histórico temporário:** médias de um minuto, limitadas às últimas 24 horas, armazenadas em memória RAM.
- **Indicação de atualização:** identificação da idade das leituras e de resultados referentes à última amostra.
- **Tratamento de falhas:** sinalização de medição indisponível, calibração pendente ou alteração detectada.
- **Persistência de configurações:** armazenamento separado da calibração e das configurações em memória não volátil.

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

**A preencher durante a implementação.**

Esta seção reunirá as escolhas efetivamente utilizadas e validadas pela equipe.

| Categoria | Definição |
| :--- | :--- |
| Linguagens | A definir. |
| Bibliotecas e frameworks | A definir. |
| Ambiente de desenvolvimento | A documentar. |
| Placa e componentes utilizados | A confirmar na montagem. |
| Ferramentas de teste | A definir. |

Os badges de tecnologias serão adicionados após a confirmação dessas escolhas.

## Primeiros passos

### Demonstração no Wokwi

![Circuito virtual do Water Tank Monitor no Wokwi](hardware/Circuito%20%282%29.png)

*Montagem no Wokwi: ESP32, OLED, DS18B20 e três controles que representam turbidez, pH e condutividade.*

Na montagem virtual, os controles permitem representar diferentes leituras dos sensores. O botão vermelho foi previsto para demonstrar uma falha de medição. Esta imagem apresenta as conexões.

### Pré-requisitos

Para obter uma cópia do repositório, tenha o **Git** instalado.

Os demais pré-requisitos — ambiente de desenvolvimento, dependências, drivers, componentes e alimentação — serão documentados quando a implementação estiver disponível.

### 1. Clonar o repositório

```bash
git clone https://github.com/Jvscopel27/Water-Tank-monitor.git
cd Water-Tank-monitor
```

### 2. Preparar o ambiente

**Pendente de implementação.** Serão incluídos aqui os comandos de instalação, as versões necessárias e as configurações do ambiente.

### 3. Montar e configurar o protótipo

**Pendente de validação.** Esta etapa receberá o esquema de ligação revisado, as instruções de montagem e os procedimentos de calibração.

### 4. Executar e acessar o painel

**Pendente de implementação.** Serão documentados o procedimento de gravação e inicialização, a configuração da rede e o endereço de acesso.

Por enquanto, a clonagem disponibiliza a documentação existente; ainda não há um procedimento executável de instalação ou inicialização.

## Testes e validação

**Testes automatizados:** ainda não disponíveis. Os comandos serão adicionados quando houver implementação e ferramentas de teste definidas.

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
- [ ] Implementar o painel acessível por Wi-Fi.
- [ ] Calibrar os sensores e integrar leituras reais.
- [ ] Implementar histórico, estados do sistema e tratamento de falhas.
- [ ] Validar a operação independente do celular e o comportamento após reinício.
- [ ] Finalizar a montagem de bancada e registrar os ensaios.
- [ ] Entregar código, manual, relatório de calibração e demonstração.

**Período previsto no planejamento:** 5 de outubro a 1º de dezembro de 2026.

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
