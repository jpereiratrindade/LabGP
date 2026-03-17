# Regras de Dominio

## Identidade
- `ResearchProject.id` e chave canonica do projeto.
- `repoPath` e dado de origem, nao chave de negocio.

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
- Inferencia de status no inventario respeita precedencia deterministica.
