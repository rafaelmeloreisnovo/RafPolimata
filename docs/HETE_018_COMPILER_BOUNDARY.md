# HETE-0.18 — fronteira científica para o RafPolimata

> **Entrada canônica:** `docs/AGENTES.md` §4 (regras de não-colisão — pesquisa orienta seleção
> experimental, nunca substitui prova de equivalência) e §5 (pipeline operacional). Este documento
> define a fronteira de integração segura do paper HETE-0.18 no RafPolimata.

**Repositório:** privado  
**Paper canônico privado:** `rafaelmeloreisnovo/papers/docs/rmrcti/HETE_018_TOROIDAL_STABILITY_ENRICHMENT.md`  
**Commit do paper:** `c444988dca0b36251a51dfe349256f75d6099b31`

## Por que isto pertence ao RafPolimata

O RafPolimata já utiliza métricas Ω determinísticas para classificar bytes e selecionar apenas entre encodings ASM previamente demonstrados como equivalentes. O paper HETE-0.18 investiga uma hipótese diferente: recorrência estatística de estabilidade no RMR-CTI.

Esses dois domínios podem conversar, mas não podem ser confundidos.

```text
classificador Omega do compilador
≠
Delta-P do RMR-CTI
≠
atrator validado
```

## Regra obrigatória

Até que HETE alcance pelo menos evidência de retorno pós-perturbação e revisão independente:

```text
NÃO usar 0,18 como:
- limiar de codegen;
- peso de otimização;
- gate semântico;
- parâmetro de ABI;
- parâmetro criptográfico;
- regra de segurança;
- critério de aceitação de compilação.
```

## Uso permitido

O RafPolimata pode:

- registrar `HETE-018` como identificador de pesquisa;
- carregar resultados como metadados de experimento;
- produzir artefatos de benchmark;
- comparar encodings equivalentes sob traces externos;
- manter `0,18` como alvo preregistrado, nunca como verdade embutida.

## Gate de integração futuro

Qualquer integração executável deve exigir um manifesto com:

```text
paper_version
source_commit
trace_hashes
run_count
seed_count
corpus_count
holdout_passed
null_models_passed
post_perturbation_return_rate
claim_state
```

Estados aceitos para observação:

```text
MEASURED_ASSOCIATION_ONLY
REPLICATED_STABILITY_CANDIDATE
POST_PERTURBATION_RETURN_CANDIDATE
```

Estado mínimo para qualquer experimento opcional de codegen:

```text
POST_PERTURBATION_RETURN_CANDIDATE
+
feature flag desativada por padrão
+
encodings semanticamente equivalentes
+
replay determinístico
```

Mesmo nesse estado, o valor não pode alterar semântica, flags, exceções, memória ou garantias de tempo constante.

## Invariante

```text
pesquisa pode orientar seleção experimental
mas nunca substituir prova de equivalência
```

Assinatura: `RAFCODE-Φ-∆RafaelVerboΩ-𓌀ΔΦΩ`
