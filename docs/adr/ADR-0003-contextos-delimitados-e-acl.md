# ADR-0003 - Contextos Delimitados e ACL

## Status
Proposta

## Contexto
Regras de inventario, fluxo e score estavam evoluindo no mesmo ritmo e com acoplamento crescente.

## Decisao
Formalizar 4 contextos:
- `PortfolioResearch`
- `GovernanceScoring`
- `InventoryIntelligence`
- `ReportingView`

Adicionar ACL para fontes heterogeneas (`Git` e `Dossie`) antes de entrar no dominio.

## Consequencias
- Menor acoplamento entre leitura de fonte e regra de negocio.
- Facilita evolucao independente por contexto.
