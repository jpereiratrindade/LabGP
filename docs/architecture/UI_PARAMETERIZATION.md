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
- Contem os projetos formalizados no sistema.

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
  - `Dossie`: pasta com PDFs, usada como base da Identificacao Embrapa.

### Processamento automatico
- Ao acionar `Reescanear inventario de fontes`, os PDFs sao processados automaticamente.
- O scanner atualiza o corpus, a curadoria e os metadados de identificacao.
- O resultado alimenta a tabela do inventario, o detalhe lateral e a exportacao TSV.

### Regra de leitura de conteudo textual
- Apenas PDFs sao usados para extrair texto de:
  - `Resumo`, `Objetivos`, `Contribuicoes para inovacao`,
  - `Atividades de pesquisa`, `Resultados esperados`, `Equipe`.
- O texto extraido e armazenado em cache local:
  - `<pasta-do-projeto>/.labgp_cache/pdf_text/<sha256>.txt`
- Se o hash do PDF nao muda, o cache e reutilizado.
- Registro persistente dos PDFs interpretados:
  - `<pasta-do-projeto>/.labgp_cache/pdf_text/manifest.tsv`
  - Campos: `file_name`, `file_path`, `curation_tag`, `relevance_score`, `included_in_corpus`, `sha256`, `cache_path`, `used_cache`, `text_bytes`

### Curadoria de documentos (PDF)
- Cada PDF recebe uma `curation_tag`:
  - `nucleo_projeto`: proposta/projeto/submissao/resumo principal
  - `evidencia_execucao`: metodologia, cronograma, atividades, resultados, pareceres
  - `suporte_admin`: edital, chamada, anexo, termo, contrato, orcamento
  - `complementar`: demais documentos
- `included_in_corpus` indica se o documento entrou no texto-base para inferencia.
- Regra de inclusao (fase atual):
  - inclui `nucleo_projeto` e `evidencia_execucao`
  - inclui tambem por relevancia alta
  - garante inclusao minima dos primeiros documentos quando necessario

### Colunas e significado
- `Origem`: `Git` ou `Dossie`.
- `Fluxo sugerido`: etapa sugerida para a fonte, apenas como apoio de curadoria.
- `Total/Inst/Pesq/Oper/Matur/Conf`: qualidade.
- `Inov`, `Ativ`, `ResPrev`: sinais de inovacao, atividades e resultados previstos.
- `Identificacao Embrapa`: bloco expandido no topo do detalhe com:
  - `Titulo do projeto`
  - `Lider/Responsavel`
  - `Instituicao do lider`
  - `Cargo do lider`
  - `Estado da submissao`
  - `Data de impressao`
  - `Codigo SEG`
  - `Contrato vinculado`
  - `Edital/Chamada`
  - `Tipo de projeto`
  - `Data de inicio`
  - `Duracao (meses)`
  - `Data de termino`

### Painel de detalhes (clique na linha)
- `Resumo`
- `Objetivos`
- `Contribuicoes para inovacao`
- `Atividades de pesquisa`
- `Resultados esperados`
- `Equipe`

### Blocos da Identificacao Embrapa
1. `Identificacao basica`: titulo, lider/responsavel, instituicao do lider, cargo do lider e fonte associada.
2. `Submissao e vinculo`: estado da submissao, data de impressao, codigo SEG e contrato vinculado.
3. `Enquadramento`: edital/chamada, tipo de projeto, datas e duracao.

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

## Exportacao TSV

- A exportacao `labgp_inventory_export.tsv` e gerada na raiz do workspace.
- Colunas principais: `repo_name`, `repo_path`, `source`, `title`, `coordinator`, `institution`, `leader_role`, `submission_state`, `submission_print_date`, `code_seg`, `linked_contract`, `call_notice`, `project_type`, `start_date`, `duration_months`, `end_date`, `team_members_count`, `team_members`, `total`, `institutional`, `researcher`, `operational`, `maturity`, `reliability`, `innovation`, `activity`, `planned_results`, `curated`, `status_suggested`.
- O TSV replica os metadados da Identificacao Embrapa que aparecem no topo do detalhe de inventario.
