# DAI LabGP

## Objetivo
Formalizar `Decision`, `Action` e `Impediment` para garantir governanca leve e rastreavel.

## Regras
- Todo impedimento critico precisa de dono e prazo.
- Toda decisao estrutural referencia ADR.
- Toda mudanca de regra de dominio referencia DDD + teste.

## Estados
- `Aberto`
- `Em execucao`
- `Resolvido`
- `Escalado`

## Minimo de registro
- contexto
- decisao
- acoes com responsavel
- impedimentos
- evidencias (commit/teste/doc)

## DAI Ativo - Curadoria PDF no Inventario

### Contexto
O LabGP passou a ingerir dossies PDF como fonte principal de conteudo no inventario.
Precisamos garantir qualidade de inferencia, rastreabilidade e governanca da curadoria.

### Decisao
- Aplicar curadoria automatica por `curation_tag` + `relevance_score`.
- Registrar resultado por documento em `manifest.tsv`.
- Manter cache por hash para reproducibilidade e desempenho.
- Referencia arquitetural: `ADR-0007-curadoria-documentos-pdf.md`.

### Actions
- [ ] Calibrar heuristicas de curadoria em amostra real de projetos (owner: Produto/Dominio, SLA: 5 dias uteis).
- [ ] Definir limiar de qualidade minimo para status inferido (owner: Dominio, SLA: 5 dias uteis).
- [ ] Incluir filtros por `curation_tag` na UI do Inventario (owner: UI, SLA: proxima iteracao).
- [ ] Especificar estrategia de migracao de `manifest.tsv` para futuras versoes (owner: Arquitetura, SLA: 5 dias uteis).

### Impediments
- [ ] Base de dossies ainda pequena para medir falso positivo/falso negativo com confianca.
- [ ] Ausencia de curadoria semantica por conteudo (fase 2).

### Evidencias esperadas
- Build e testes verdes.
- `manifest.tsv` gerado por projeto com trilha completa dos PDFs.
- Atualizacao de `DDD.md`, `UI_PARAMETERIZATION.md` e ADR.
