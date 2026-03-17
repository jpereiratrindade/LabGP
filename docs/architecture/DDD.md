# DDD LabGP

## Contextos Delimitados
- `PortfolioResearch`: cadastro e ciclo de vida do projeto cientifico.
- `GovernanceScoring`: score consolidado e por persona.
- `InventoryIntelligence`: leitura de repositorios/dossies e inferencia de sinais.
- `ReportingView`: projecoes para lista, kanban, grafo e inventario.

## Agregados e Entidades
- Agregado raiz: `ResearchProject` (estado de fluxo e dados de execucao).
- Entidade de apoio: `InventoryEntry` (sinais detectados por fonte).
- Value Object: `ScoreBreakdown` (dimensoes de qualidade).

## Regras de Dominio
- Fluxo e qualidade sao eixos diferentes:
  - `status` = etapa do processo
  - `score` = robustez/qualidade
- `ScoreBreakdown` contem:
  - base: `operational`, `maturity`, `reliability`, `execution`
  - perspectivas: `institutional`, `researcher`
  - consolidado: `total`
- `InventoryEntry` contem:
  - `source` (`Git` ou `Dossie`)
  - `innovationSignals`, `activitySignals`, `plannedResultsSignals`
  - `inferredStatus`

## Referencias
- `CONTEXT_MAP.md`
- `DOMAIN_RULES.md`
