# ADR-0002 - Score por duas personas

## Status
Aceita

## Contexto

A avaliacao anterior do LabGP enfatizava dimensoes tecnicas/consolidadas.
Para gestao de projetos de pesquisa, isso era insuficiente para diferenciar:

- necessidades institucionais (compliance, prestacao, governanca)
- necessidades do pesquisador lider (execucao cientifica e equipe)

## Decisao

Adicionar no dominio dois scores explicitos:

- `institutional`
- `researcher`

Mantendo `total` para continuidade historica e comparabilidade.

Tambem foi decidido:

- remover coluna `Integrado` do inventario (baixo valor discriminatorio)
- incluir sinais `Inov`, `Ativ`, `ResPrev` e `Origem`
- permitir alternancia de visao de score na GUI

## Consequencias

### Positivas
- Melhor alinhamento com realidade de projetos cientificos.
- Transparencia entre desempenho administrativo e desempenho de execucao.
- Priorizacao mais precisa no Kanban e no inventario.

### Custos
- Mais variaveis de leitura para usuario final.
- Necessidade de calibracao periodica dos pesos por contexto institucional.
