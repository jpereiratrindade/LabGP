# ADR-0006 - Versionamento de Evidencias com Hash

## Status
Proposta

## Contexto
Evidencias de projeto (arquivos e artefatos) precisam cadeia de custodia verificavel.

## Decisao
Versionar evidencias com metadados minimos:
- `version`
- `sha256`
- `uploaded_at`
- `uploaded_by`
- `supersedes`

## Consequencias
- Melhora confiabilidade para auditoria e prestacao.
- Introduz custo de padronizacao de upload em iteracao futura.
