# RafCI experiment-specific closures

**Area:** execution-governance / scientific evidence federation  
**Authority:** `rafaelmeloreisnovo/RafPolimata`  
**Lifecycle:** `ACTIVE / fail-closed contracts`  
**Claim state:** `claim_allowed=false`  
**Closure binding:** `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY`

This directory contains experiment-specific cross-repository closure contracts. A closure pins the exact producer revisions participating in one scientific claim path. It does not replace `rafci/graph.v1.json`, transfer implementation authority, or convert historical evidence into current evidence.

## SGPT Synaptic Closure V1

Canonical source:

- `sgpt_synaptic_closure_v1.json`

Validator:

- `../tools/verify_sgpt_synaptic_closure_v1.py`

CI:

- `../../.github/workflows/sgpt-synaptic-closure-v1.yml`

The closure binds the same Strong-Gravity Photonic Transport hypothesis across:

```text
RLL science producer
    -> WORLD69 mathematical proof boundary
    -> theorem/hypothesis classification
    -> scholarly manuscript
    -> adversarial testbed
    -> GitHub Actions execution surface
    -> RafCI evidence/claim authority
```

`rafaelmeloreisnovo/actions` is pinned only as `ACTION_PROVIDER`. Its provider identity never makes it an epistemic authority and the SGPT closure does not invoke irrelevant Gradle behavior merely to claim use of that repository.

## Fire gates

```text
G0 identity
G1 mathematics
G2 determinism
G3 adversarial negative controls
G4 physical/scientific baselines
G5 reproduction
G6 held-out prediction
G7 claim promotion
```

The source contract deliberately keeps G0-G7 at `TOKEN_VAZIO`. CI may prove the closure syntax/topology and reject illegal mutations, yielding only `PASS_CONTRACT_ONLY`. Scientific gate states belong to revision-bound receipts produced by their own executions.

The first adversarial set is fixed before result inspection:

- remove `Theta`;
- remove `M_f`;
- remove `R_ram`;
- flip the sign of `Theta`;
- shuffle predictor variables;
- permute temporal targets;
- run the baseline without the SGPT extension.

A useful candidate must survive these negative controls and then compete with all mandatory GRMHD/GRRMHD/GRPIC/GRRT baselines. A green contract CI is not evidence that it did so.

## Truth boundary

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
IMPLEMENTED_UNTESTED != PASS
PASS_CONTRACT_ONLY != SCIENTIFIC_VALIDATION
```

Rollback is additive: revert or supersede the closure in RafPolimata. Never rewrite producer history or delete contradictory receipts.
