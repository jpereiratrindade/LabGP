# ADR-0005 - Identidade Unica de Projeto

## Status
Proposta

## Contexto
Projetos podem aparecer por nomes/paths diferentes quando vindo de fontes distintas.

## Decisao
`ResearchProject.id` passa a ser a identidade canonica.
Mapeamentos externos (`repoPath`, aliases) sao correlacoes e nao chave de negocio.

## Consequencias
- Evita duplicidade de estado no kanban/lista/grafo.
- Requer reconciliacao de aliases em evolucao futura.
