# DAI LabGP

## Papel no projeto
`DAI` em LabGP significa `Decision`, `Action` e `Impediment`.
Ele existe como conceito de governanca leve para registrar operacao, acompanhamento e bloqueios
sem substituir os artefatos estruturais do projeto.

## Quando usar
- registrar decisoes operacionais de curta ou media duracao
- explicitar acoes em andamento e impedimentos relevantes
- conectar execucao corrente com evidencias verificaveis

## Como se relaciona com outros docs
- `ADR`: decisoes arquiteturais e estruturais
- `DDD`: regras e conceitos de dominio
- `testes` e `commits`: evidencias de implementacao

## Regras minimas
- mudanca estrutural deve referenciar um `ADR`
- mudanca de regra de dominio deve vir com teste e alinhamento ao `DDD`
- impedimento critico precisa de responsavel e proximo passo claro

## Evidencias esperadas
- build e testes executados quando aplicavel
- documentacao atualizada somente quando sustenta o comportamento real do sistema
- rastreabilidade suficiente para entender a decisao tomada
