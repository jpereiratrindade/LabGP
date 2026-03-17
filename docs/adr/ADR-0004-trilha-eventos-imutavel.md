# ADR-0004 - Trilha de Eventos Imutavel

## Status
Proposta

## Contexto
Mudancas de fluxo e score pedem auditabilidade forte para governanca de projetos.

## Decisao
Adotar trilha append-only para eventos de:
- transicao de status
- recálculo de score
- mudancas de metadados criticos

## Consequencias
- Melhor rastreabilidade historica.
- Exige camada de persistencia de eventos em iteracao futura.
