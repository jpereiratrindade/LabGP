# Next Session Checkpoint

Data: 2026-03-17

## Estado encerrado hoje
- Curadoria de PDFs ativa no inventario:
  - `curation_tag`, `relevance_score`, `included_in_corpus`
  - `manifest.tsv` por projeto em `.labgp_cache/pdf_text/`
- Curadoria visivel na UI (coluna `Curad` + resumo lateral + alerta de PDF sem texto extraivel).
- Extracao de secoes ajustada para reduzir vazamento entre contexto e equipe.
- Guardrails de fluxo no dominio adicionados:
  - `ResearchProjectStore::moveStatus` agora bloqueia transicoes invalidas.
  - Teste de dominio atualizado para validar sequencia de transicao.

## Onde continuar amanha (prioridade)
1. Revisar inferencia de status para dossies da Embrapa com OCR fraco (casos com `text_bytes=0`).
2. Ajustar calibracao de score para reduzir discrepancia entre projetos com dossie semelhante.
3. Implementar plano de refatoracao incremental dos riscos do Socrates:
   - separar responsabilidades de `InventoryScanner`
   - separar abas de `GuiDashboard`
   - centralizar `ScorePolicy` (faixas e thresholds)
4. Avaliar fallback OCR opcional (ex.: pipeline externo) mantendo PDF como fonte oficial.

## Comandos uteis para retomada
- Build: `cmake -S . -B build-ninja -G Ninja && cmake --build build-ninja`
- Testes: `ctest --test-dir build-ninja --output-on-failure`
- Execucao: `./build-ninja/LabGP --workspace /run/media/jpereiratrindade/labeco10T/Embrapa/Projetos`
