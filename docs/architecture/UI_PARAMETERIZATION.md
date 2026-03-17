# UI Parameterization - LabGP

## Objetivo

Documentar, de forma objetiva, como cada visao da GUI e parametrizada e como interpretar os dados exibidos.

## Convencao principal

- `Fluxo`: etapa do ciclo de vida do projeto (`Proposta`, `Em avaliacao`, `Aprovado`, `Execucao`, `Analise`, `Publicacao`, `Encerrado`).
- `Qualidade`: scores numericos (`Total`, `Inst`, `Pesq`, `Oper`, `Matur`, `Exec`, `Conf`).

Um projeto pode estar `Aprovado` no fluxo e ainda ter qualidade baixa.

## Parametros globais

- `workspaceRoot`: pasta usada no scan.
- `scorePerspective`:
  - `Consolidado` -> usa `score.total`
  - `Institucional` -> usa `score.institutional`
  - `Pesquisador` -> usa `score.researcher`
- Cores por faixa de qualidade:
  - `>= 80`: verde
  - `>= 60`: amarelo
  - `>= 40`: laranja
  - `< 40`: vermelho

## Lista de Projetos (aba `Projetos`)

### Fonte de dados
- `ResearchProjectStore`.
- Pode conter projetos cadastrados e projetos `INV-*` derivados do inventario.

### Ordenacao
- Ordena por qualidade da visao ativa (`scorePerspective`), descrescente.
- Desempate por `id`.

### Colunas
- `Fluxo (Status)`: etapa do ciclo de vida.
- `Qualid(...)`: score da perspectiva ativa.
- `Total/Inst/Pesq/Oper/Matur/Exec/Conf`: detalhamento da qualidade.

## Kanban (aba `Kanban`)

### Agrupamento
- Sempre por `Fluxo (Status)`:
  - `Proposta`, `Em avaliacao`, `Aprovado`, `Execucao`, `Analise`, `Publicacao`, `Encerrado`.

### Card
- Mostra `Fluxo` e `Qualidade` separadamente.
- `Qualidade` usa a perspectiva ativa.

## Inventario (aba `Inventario`)

### Fonte de dados
- `InventoryScanner::scan(workspaceRoot)`.
- Entrada pode ser:
  - `Git`: pasta com `.git`.
  - `Dossie`: pasta com sinais documentais (`.md`, `.pdf`, `.docx`, `.xlsx`, `.csv`).

### Colunas e significado
- `Origem`: `Git` ou `Dossie`.
- `Fluxo (Status inferido)`: etapa inferida para item de inventario.
- `Total/Inst/Pesq/Oper/Matur/Conf`: qualidade.
- `Inov`, `Ativ`, `ResPrev`: sinais de inovacao, atividades e resultados previstos.

### Inferencia de fluxo no inventario (precedencia)
1. Encerrado
2. Publicacao
3. Analise
4. Execucao
5. Aprovado
6. Em avaliacao
7. Proposta (fallback)

## Grafo (aba `Grafo`)

### Eixos e visuais
- `X`: score `Institucional` (0..100)
- `Y`: score `Pesquisador` (0..100)
- Cor do no: faixa da qualidade da perspectiva ativa.
- Tamanho do no: proporcional a `Execucao`.

### Controles
- `Qualidade minima`: filtra por score da perspectiva ativa.
- `So criticos (<40)`: filtra apenas itens criticos.

### Interacao
- Hover: tooltip com `Fluxo` + principais scores.
- Clique: painel lateral com detalhes completos do projeto.

## Regras de composicao dos scores

Ver implementacao em `src/domain/Scoring.cpp`:
- Base: `operational`, `maturity`, `reliability`, `execution`.
- Perspectivas: `institutional`, `researcher`.
- Consolidado: `total`.

## Regra de mapeamento inventario -> projetos `INV-*`

No carregamento da aplicacao:
- primeiros itens do inventario sao transformados em projetos de tela (`INV-1`, `INV-2`, ...),
- com `status` vindo de `inferredStatus`,
- e atributos derivados dos sinais de inventario para compor os scores.
