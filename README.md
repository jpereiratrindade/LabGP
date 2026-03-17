# LabGP

Gestao de Projetos de Pesquisa em C++.

## Objetivo

LabGP nasce como um projeto-irmao do LabGestao, com foco em ciclo de vida de projetos de pesquisa:
- Proposta, aprovacao, execucao, analise, publicacao, encerramento
- DAI (Decision, Action, Impediment)
- Maturidade de engenharia (ADR, DDD, governanca)
- Confiabilidade tecnica (checks de CI, analise estatica, sanitizers)

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Estrutura

- `include/` contratos
- `src/` implementacao
- `tests/` testes de dominio
- `docs/adr` decisoes arquiteturais
- `docs/architecture` contexto de dominio
- `docs/dai` decisoes operacionais

## Roadmap curto

1. CRUD de projetos de pesquisa
2. Kanban por fase
3. Inventario e score de confiabilidade
4. Grafo de relacoes entre projetos/repositorios
