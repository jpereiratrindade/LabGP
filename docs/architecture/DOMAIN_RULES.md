# Regras de Dominio

## Identidade
- `ResearchProject.id` e chave canonica do projeto.
- `repoPath` e dado de origem, nao chave de negocio.
- `InventoryEntry` agrega a Identificacao Embrapa e os sinais de apoio.

## Fluxo
- Ordem canonica de etapas:
  - `Proposal -> InReview -> Approved -> Execution -> Analysis -> Publication -> Closed`
- Status representa etapa do fluxo, nao qualidade.

## Qualidade
- Score de qualidade e separado do fluxo.
- Perspectivas:
  - `institutional`
  - `researcher`
- `total` permanece para comparacao consolidada.

## Integridade
- `deliveredDeliverables <= plannedDeliverables` no calculo de execucao.
- `reliabilityApplicable=false` para projetos nao software-intensive.
- O reescan do workspace processa PDFs automaticamente e atualiza a Identificacao Embrapa.
- A inferencia de status no inventario respeita precedencia deterministica e serve como apoio de leitura.
