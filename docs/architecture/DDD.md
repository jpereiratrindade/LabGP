# DDD LabGP

## Contextos Delimitados
- `PortfolioResearch`: ciclo de vida do projeto cientifico.
- `GovernanceScoring`: score consolidado e por persona.
- `InventoryIntelligence`: leitura de repositorios/dossies, curadoria de fontes e identificacao Embrapa.
- `ReportingView`: projecoes para lista, kanban, grafo, inventario e exportacao TSV.

## Agregados e Entidades
- Agregado raiz: `ResearchProject` (estado de fluxo e dados de execucao).
- Entidade de apoio: `InventoryEntry` (sinais detectados por fonte).
- Entidade de apoio: `InterpretedDocument` (rastreabilidade da ingestao PDF).
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
  - `repoPath` como referencia da fonte selecionada
  - metadados de identificacao: `submissionState`, `submissionPrintDate`, `leaderRole`, `codeSeg`, `linkedContract`
  - `innovationSignals`, `activitySignals`, `plannedResultsSignals`
  - `inferredStatus`
  - campos textuais extraidos: `summary`, `objectives`, `innovationContributions`,
    `researchActivities`, `expectedResults`, `teamMembers`
- `InterpretedDocument` contem:
  - identidade e trilha: `fileName`, `filePath`, `sha256`, `cachePath`
  - curadoria: `curationTag`, `relevanceScore`, `includedInCorpus`
  - execucao tecnica: `usedCache`, `textBytes`
- Regra: somente PDFs alimentam o corpus textual do dossie.
- Regra: o corpus inclui documentos por curadoria/relevancia com fallback minimo.
- Regra: o registro persistente da ingestao fica em `.labgp_cache/pdf_text/manifest.tsv`.
- Regra: o reescan processa PDFs automaticamente e atualiza o inventario.
- Regra: os metadados de identificacao aparecem no topo do detalhe de inventario e na exportacao TSV.

## Referencias
- `CONTEXT_MAP.md`
- `DOMAIN_RULES.md`
- `../adr/ADR-0007-curadoria-documentos-pdf.md`
