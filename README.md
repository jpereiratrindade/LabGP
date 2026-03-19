# LabGP

Gestao de Projetos de Pesquisa lato sensu (com ou sem software).

## Objetivo

LabGP foca no ciclo de vida de projetos de pesquisa:
- Proposta, aprovacao, execucao, analise, publicacao, encerramento
- DAI (Decision, Action, Impediment)
- Gestao de execucao real (metodologia, plano de trabalho, cronograma, orcamento, entregas, acompanhamento)
- Maturidade de engenharia e governanca (ADR, DDD, DAI)
- Confiabilidade tecnica para projetos intensivos em software (CI, analise estatica, sanitizers)

Exemplo de uso direto:
- Projetos em avaliacao de agencias/instituicoes
- Projetos de pesquisa com forte componente territorial, metodologico e de politica publica

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Execucao

- `./build/LabGP` abre GUI (SDL2 + ImGui), com fallback para console.
- `./build/LabGP --console` força modo texto.
- `./build/LabGP --gui` força tentativa de GUI.
- Na GUI, selecione a pasta de trabalho pelo menu: `Arquivo > Selecionar pasta de trabalho...`.
- Na GUI, use `Arquivo > Reescanear inventario de fontes` para atualizar apenas a leitura das fontes.
- Na aba `Inventario`, selecione uma fonte e use `Criar Projeto a Partir Desta Fonte` para registrar o projeto manualmente.
- `./build/LabGP --workspace /caminho/para/repositorios` seleciona a pasta de trabalho a analisar como inventario de fontes.
- `./build/LabGP --workspace=/caminho/para/repositorios` alternativa equivalente.
- `LABGP_WORKSPACE=/caminho/para/repositorios ./build/LabGP` define workspace por variavel de ambiente.
- `./build/LabGP --pick-workspace` tenta abrir seletor de pasta do sistema (kdialog/zenity/osascript).
- Para dossies, o parser considera PDFs como fonte textual de apoio e usa cache por hash em `.labgp_cache/pdf_text/`.
- O sistema registra curadoria dos PDFs em `.labgp_cache/pdf_text/manifest.tsv` (tag, relevancia e inclusao no corpus) para apoiar a selecao manual de fontes.

## Modelo de avaliacao

- `Operacional` (base tecnica ou base documental, conforme tipo do projeto)
- `Maturidade` (ADR/DDD/DAI/Governanca)
- `Confiabilidade` (aplicavel a projetos intensivos em software)
- `Execucao` (metodologia, plano, cronograma, orcamento, entregas e impedimentos)
- Formula atual: `Total = 0.30*Oper + 0.25*Matur + 0.20*Confiab + 0.25*Exec`
- `Confiab` pode ser `N/A` para projetos nao intensivos em software (sem penalizacao indevida)

## Personas de gestao

LabGP agora trabalha com duas perspectivas complementares:

- `Institucional`: compliance, governanca, rastreabilidade, controle fisico-financeiro e risco.
- `Pesquisador`: planejamento cientifico, execucao da equipe, validacao e resultados previstos.

### Scores por persona

- `Score Institucional` (0-100): mede robustez de governanca e controle.
- `Score Pesquisador` (0-100): mede capacidade de execucao cientifica e de equipe.
- `Score Total` (0-100): mantem o consolidado tecnico-operacional para comparacao historica.

Na GUI:
- A aba `Projetos` e a aba `Kanban` permitem trocar a visao de score (`Consolidado`, `Institucional`, `Pesquisador`).
- A aba `Inventario` mostra os tres scores, os sinais de pesquisa (`Inov`, `Ativ`, `ResPrev`) e a acao de criar projeto manual a partir da fonte selecionada.
- A aba `Grafo` plota projetos por `Institucional x Pesquisador`, com cor por qualidade e tamanho por execucao.

## Parametrizacao das visoes

As regras de parametrizacao de:
- `Lista de Projetos`
- `Kanban`
- `Inventario`
- `Grafo`

estao documentadas em:

- `docs/architecture/UI_PARAMETERIZATION.md`

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
