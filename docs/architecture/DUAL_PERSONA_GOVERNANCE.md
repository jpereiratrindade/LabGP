# Dual Persona Governance - LabGP

## Objetivo

Evoluir o LabGP para atender dois olhares simultaneos:

- `Persona Institucional`: foco em conformidade, governanca, controle e prestacao.
- `Persona Pesquisador`: foco em execucao cientifica, coordenacao da equipe e resultados.

As duas visoes coexistem e reduzem vies de avaliacao unicamente administrativa ou unicamente tecnica.

## Ciclo de vida recomendado

1. Prospecao e enquadramento
2. Submissao e contratacao
3. Planejamento executivo
4. Execucao cientifica e administrativa
5. Monitoramento e controle
6. Prestacao de contas e encerramento
7. Avaliacao de impacto (ex-post)

## Processos essenciais por persona

### Institucional
- Elegibilidade e aderencia a edital/chamada
- Workflow de aprovacoes
- Controle fisico-financeiro por rubrica
- Gestao de riscos e mudancas
- Trilhas de auditoria e evidencias
- Prestacao parcial/final

### Pesquisador
- Plano metodologico e plano de trabalho
- Cronograma cientifico e marcos
- Organizacao da equipe e revisoes tecnicas
- Gestao de dados e validacao metodologica
- Entregaveis cientificos e resultados previstos
- Captura de licoes aprendidas

## Indicadores de inventario

O inventario agrega sinais detectaveis automaticamente:

- `Inov` (`innovationSignals`): contribuicoes para inovacao
- `Ativ` (`activitySignals`): atividades de pesquisa em andamento
- `ResPrev` (`plannedResultsSignals`): indicios de resultados previstos/esperados
- `Origem`: `Git` ou `Dossie`
- `Status inferido`: proposta, em avaliacao, aprovado, execucao, analise, publicacao, encerrado

## Modelo de score no dominio

`ScoreBreakdown`:

- Base consolidada:
  - `operational`
  - `maturity`
  - `reliability`
  - `execution`
  - `total`
- Perspectivas:
  - `institutional`
  - `researcher`

### Leitura recomendada

- `0-39`: fragil
- `40-64`: operacional minimo
- `65-84`: robusto
- `85-100`: referencia

## Decisoes de produto na UI

- `Projetos`: alternancia de visao de score (`Consolidado`, `Institucional`, `Pesquisador`)
- `Kanban`: exibe score da visao escolhida sem perder `Total`
- `Inventario`: remove coluna `Integrado` e inclui `Origem`, `Inov`, `Ativ`, `ResPrev`, `Status`
- `Grafo`: posiciona projetos em `Institucional x Pesquisador` com cor por qualidade
- Separacao explicita em todas as telas: `Fluxo` (etapa) vs `Qualidade` (score)

## Proximas evolucoes recomendadas

1. Tornar pesos de score configuraveis por perfil de organizacao.
2. Adicionar regras de status por metadados de submissao (ex.: protocolo, data, parecer).
3. Expandir inventario para capturar:
   - equipe e papeis
   - cronograma detalhado
   - riscos e mitigacoes
   - resultados realizados vs previstos
4. Exportar score por persona em CSV/JSON para comites e prestacao.
