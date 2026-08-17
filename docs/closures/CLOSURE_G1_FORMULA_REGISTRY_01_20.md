# CLOSURE_G1 — Formula Registry 01–20 Governance

**Estado:** `IMPLEMENTED_GOVERNANCE / MATERIAL_GAPS_REMAIN_OPEN`  
**Escopo:** `research/formula_registry_01_20/**`, seus validadores e referências executáveis  
**Registry:** `research/formula_registry_01_20/registry.v1.json`  
**Gate:** `scripts/validate_formula_registry_01_20.py` + `tests/test_formula_registry_01_20.py`  
**Âncora:** `CLOSURE_G1`

## Significado

`CLOSURE_G1` fecha somente a **ausência de governança e rastreabilidade das lacunas** introduzidas pelo registry F01–F20. Ela não fecha as lacunas matemáticas, físicas, biológicas, experimentais ou semânticas registradas individualmente.

```text
CLOSURE_G1 PASS
!=
F01..F20 validated claims
```

Toda ocorrência de `TOKEN_VAZIO` vinculada a esta closure continua materialmente aberta até que seu próprio domínio, unidades, dados, falsificador, evidência e replicação sejam satisfeitos.

## Invariantes

1. `concept != implementation != execution != evidence != validated_claim`.
2. `TOKEN_VAZIO != PASS`.
3. `CONTRADICTION_ORIGINAL != PASS`.
4. `BLOCKED_AS_WRITTEN != PASS`.
5. `claim_allowed=false` permanece obrigatório para todos os 20 registros nesta versão.
6. F06 permanece bloqueada até consistência dimensional e definição observável.
7. F09 preserva explicitamente a divergência da expressão original; somente a variante corrigida pode ser testada como convergente.
8. F11 permanece bloqueada até tipagem escalar/tensorial e lei de conservação declaradas.
9. F07 usa 42 como base/indexação finita; esta closure não reabre nem promove a antiga alegação de 42 atratores fixos.
10. A passagem para `SIMULATED`, `EVIDENCE_LINKED`, `REPLICATED` ou `CLAIM_ALLOWED` exige alteração separada, evidência versionada e gates correspondentes.

## Prioridade operacional

- **P0:** F01, F04, F06, F09, F11, F13, F16 — defeitos de tipagem, dimensão, interpretação, convergência ou probabilidade.
- **P1:** F03, F05, F07, F08, F10, F12, F14, F20 — núcleo formal/computacional tratável, ainda condicionado a domínio/adaptador.
- **P2:** F02, F15, F17, F18, F19 — modelos semânticos/fenomenológicos que exigem operacionalização antes de claims externos.

## Critério de fechamento desta governance closure

A governança de F01–F20 é considerada implementada quando, no mesmo commit/PR:

- o registry contém exatamente F01..F20 em ordem;
- cada registro contém `primary_gap`, `correction`, `falsifier`, `dimensional_gate`, `priority`, `state` e `claim_allowed=false`;
- F06/F09/F11 preservam seus estados críticos fail-closed;
- referências executáveis não afirmam validação externa;
- testes determinísticos do subset implementado passam;
- o `Formal Science Orchestrator` valida o boundary do registry;
- o gate global de TOKEN_VAZIO reconhece `CLOSURE_G1` como vínculo de custódia.

## Critério de fechamento material por fórmula

Cada fórmula mantém fechamento próprio. O mínimo é:

```text
domínio explícito
+ unidades/nondimensionalização
+ condições iniciais/de contorno quando aplicáveis
+ observável ou saída mensurável
+ falsificador executável
+ dataset/fixture com proveniência quando necessário
+ comparação com baseline
+ incerteza/erro
+ receipt do gate
```

A ausência de qualquer item permanece estado válido e auditável; não autoriza inferência substitutiva.

## Relações

- `CLOSURE_L9_T7_CONVERGENCE.md` continua autoridade para a fronteira de claims T⁷/42.
- `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.md` continua autoridade geral para governança de lacunas operacionais do repositório.
- `docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md` continua autoridade para promoção científica.

## F_ok / F_gap / F_next

```text
F_ok   = registry versionado + validador fail-closed + testes + CI boundary
F_gap  = evidência externa/experimental e adapters específicos continuam abertos por fórmula
F_next = promover somente uma fórmula por vez quando seu conjunto próprio de gates e receipts estiver completo
```
