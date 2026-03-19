# ADR-0007 - Curadoria de Documentos PDF no Inventario

## Status
Aceita

## Contexto
O LabGP passou a usar dossies PDF como fonte de Identificacao Embrapa e apoio a
curadoria automatica no reescan. Sem curadoria explicita, documentos administrativos
e anexos podem contaminar a leitura das fontes e a extracao de campos
(resumo, objetivos, atividades, resultados, metadados de identificacao).

Tambem havia necessidade de rastreabilidade: quais documentos foram interpretados,
quais entraram no corpus, e sob qual criterio.

## Decisao
Adotar curadoria automatica de PDFs no contexto `InventoryIntelligence`, com:

1. Classificacao por tag (`curation_tag`) no nome do arquivo:
- `nucleo_projeto`
- `evidencia_execucao`
- `suporte_admin`
- `complementar`

2. Score de relevancia (`relevance_score`) por heuristica de nome.

3. Regra de inclusao no corpus textual (`included_in_corpus`):
- prioriza `nucleo_projeto` e `evidencia_execucao`
- inclui por relevancia alta
- garante inclusao minima para evitar corpus vazio

4. Cache de texto extraido por hash do PDF (`sha256`) em:
- `.labgp_cache/pdf_text/<sha256>.txt`

5. Registro persistente da curadoria em:
- `.labgp_cache/pdf_text/manifest.tsv`
- campos: `file_name`, `file_path`, `curation_tag`, `relevance_score`,
  `included_in_corpus`, `sha256`, `cache_path`, `used_cache`, `text_bytes`

## Consequencias

### Positivas
- Melhor qualidade de leitura das fontes e da Identificacao Embrapa.
- Rastreabilidade auditavel do processo de ingestao documental.
- Menor custo de reprocessamento por reuso de cache por hash.

### Custos e riscos
- Heuristicas por nome podem gerar falso positivo/falso negativo.
- Exige calibracao periodica das regras de curadoria.

## Plano de adocao
1. Medir taxa de acerto de `curation_tag` em amostra real.
2. Ajustar heuristicas por dominio institucional quando necessario.
3. Evoluir para fase 2 com curadoria semantica (conteudo), mantendo compatibilidade com `manifest.tsv` e com a exportacao TSV de identificacao.
