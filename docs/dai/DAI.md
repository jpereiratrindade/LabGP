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
