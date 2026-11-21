# Revisão do repositório e plano de replicação — 21/11/2026

## Estado encontrado

A revisão usou `origin/main` atualizado do GitHub, cuja ponta era `2f4625b`. Já existiam pastas separadas para firmware, documentação, hardware, simulação e mecânica. Entretanto, as mudanças testadas na bancada estavam somente locais; os guias públicos ainda exigiam temperatura e sessões. O teste de navegador dependia de um caminho de Chrome no Windows. Não havia fluxo de CI. Dois arquivos STEP estavam soltos na raiz.

## Implementado nesta atualização

- Publicação do firmware e dos registros experimentais, sem backups ou credenciais.
- Guias atuais para iniciar, montar, operar e resolver falhas, com caminho de leitura no README.
- Separação explícita entre sensores reais, simulação histórica e programas temporários de diagnóstico.
- Unidade de condutividade preservada e ausência de compensação térmica documentada.
- Testes portáveis de navegador/PDF, sem caminho fixo de Chrome; CI para compilação e testes.
- Arquivos STEP agrupados na pasta mecânica; inventário das peças.

## Próximas melhorias, em ordem

1. **Identificar cada módulo real:** fotos frente/verso, nome/revisão, pinagem e fonte de dados. O modelo exato de turbidez/TDS ainda não está comprovado pela documentação.
2. **Publicar o circuito final:** substituir desenhos históricos por esquema que corresponda fio a fio à bancada atual, incluindo divisores efetivamente usados em turbidez e TDS. Não marcar como confirmado um resistor que não foi verificado.
3. **Adicionar fotos anotadas da montagem:** alimentação, GND comum, ordem dos conectores e separação entre área molhada e eletrônica. Incluir checklist com resultado esperado em cada etapa.
4. **Criar release reproduzível:** tag, binário compilado, checksum e notas com placa/core/bibliotecas. Fazer isso após confirmar a montagem final e o novo firmware na bancada.
5. **Registrar receitas e ensaios:** massas/volumes, origem das referências, condições da amostra, leituras e dispersão. Separar referência operacional de valor físico certificado.
6. **Facilitar manutenção:** templates de issue e PR, glossário com ADC/GPIO/GND/NVS e tutorial em vídeo seguindo exatamente o guia escrito.

A documentação agora orienta o uso do software atual. A replicação elétrica completa ainda depende de identificar os módulos e atualizar o esquema; isso não pode ser inferido apenas do código.
