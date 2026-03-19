# Context Map

## Contextos
- `PortfolioResearch`: ciclo de vida de projetos de pesquisa.
- `GovernanceScoring`: calculo de qualidade (`Total`, `Inst`, `Pesq`).
- `InventoryIntelligence`: leitura de repositorios/dossies, curadoria de fontes e identificacao Embrapa.
- `ReportingView`: visoes de UI (`Projetos`, `Kanban`, `Grafo`, `Inventario`) e exportacao TSV.

## Relacoes
- `InventoryIntelligence -> PortfolioResearch`: identificacao e sinais alimentam a leitura do projeto.
- `PortfolioResearch -> GovernanceScoring`: fatos do projeto alimentam score.
- `GovernanceScoring -> ReportingView`: leitura para visualizacao.
- `ReportingView` nao altera regras de dominio.

## ACL (Anti-Corruption Layer)
- Adaptador `Git`: normaliza repositorio para `InventoryEntry`.
- Adaptador `Dossie`: normaliza documentos para `InventoryEntry`.
