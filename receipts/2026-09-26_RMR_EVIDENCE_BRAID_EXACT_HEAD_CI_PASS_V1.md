# Receipt — RMR Evidence Braid V1 Exact-Head CI PASS — 2026-09-26

**Kind:** `STATE_SUCCESSOR / EXACT_HEAD_CI_PASS / APPEND_ONLY`  
**Parent:** `receipts/2026-09-26_RMR_EVIDENCE_BRAID_CI_RECONCILIATION_V1.md`  
**Governance:** `CLOSURE_L1`  
**claim_allowed:** `false`

## Why this successor exists

The parent receipt was committed before provider CI finished and therefore
correctly recorded:

`successor exact-head CI = TOKEN_VAZIO pending provider observation`.

That historical state is preserved. This successor records the later provider
observation and does not rewrite the parent.

## Exact repository lineage

- original implementation PR: `#354`
- original merge: `fa37660ec937f72da9cb91e6dd8ce373e06bf4e5`
- reconciliation PR: `#355`
- exact tested reconciliation head:
  `0745e2b78c5d0413f8e3adc78e44baf61c9eeb37`
- authorized merge commit:
  `fe61af712b2eda2adb4ca2348befe47e975454b5`

## Exact provider evidence

For tested head `0745e2b78c5d0413f8e3adc78e44baf61c9eeb37`:

| Gate | Run | State |
|---|---:|---|
| CI | `36276258122` | PASS |
| Formal Science Orchestrator | `36276258160` | PASS |
| Workflow Graph Audit | `36276258065` | PASS |
| Internal Custody Ledger | `36276258069` | PASS |
| M063 language completion freestanding contract | `36276258130` | PASS |

The canonical CI run reports the single job
`Semantic coherence and falsifiability gates` as PASS.

Observed CI artifacts include:

- `token-vazio-validation-36276258122` — artifact `10917497278`;
- `apkc-proofs-and-hotfix-36276258122` — artifact `10917616780`;
- `apkc-proof-runs` — artifact `10917183307`;
- `watt-proxy-report` — artifact `10917263150`.

## Closed gate

The reconciliation source and canonical CI now jointly establish, at the tested
head:

- TOKEN_VAZIO governance validation: PASS;
- hosted Evidence Braid selftest/KAT path: PASS;
- freestanding x86-64 production object gate: PASS;
- freestanding ARMv7 production cross-object gate: PASS;
- freestanding AArch64 production cross-object gate: PASS;
- undefined-symbol requirement for the production objects: PASS within the
  canonical CI contract.

Therefore:

`SOURCE_PRESENT + EXACT_HEAD_CI_PASS`

is now evidence-backed for this bounded scope.

## Boundaries that remain open

This receipt does **not** promote:

- physical Android ARM32 execution;
- physical Android ARM64 execution;
- calibrated performance or energy;
- BLAKE3 producer binding;
- ApkC/ZIPRAF Evidence Braid adapter;
- external legal identity, trusted timestamp, certification or
  non-repudiation.

Those remain `TOKEN_VAZIO` or separately governed.

`SOURCE_PRESENT != CI_PASS != DEVICE_PASS != LEGAL_QUALIFICATION`

## R3

**F_ok:** the historical fail-closed governance error is repaired and the exact
successor head has provider CI evidence across the canonical gates.

**F_gap:** device execution and downstream custody adapters remain open.

**F_next:** bind the Evidence Braid generation root into the ApkC/ZIPRAF receipt
path as a separate adapter delta without replacing independent external
verification.

claim_allowed=false
