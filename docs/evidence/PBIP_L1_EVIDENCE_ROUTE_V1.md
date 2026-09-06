# PBIP-L1 — Evidence route

- `FEDERATION_ID`: `PBIP-L1-FED-V1`
- `FORMULA_ID`: `PBIP-L1`
- `role`: `EVIDENCE_ORCHESTRATION`
- `state`: `ROUTE_DEFINED / EXECUTION_PENDING`
- `claim_allowed`: `false`
- `date`: `2026-09-06`
- `governance_closure`: `CLOSURE_L1`

Governance anchor: `CLOSURE_L1`. This closure binds missing provenance/toolchain/hash/runtime evidence to governance without promoting the material gap.

## Purpose

Define the finite evidence path for the formal identity

```math
q^2=r^2-d_\perp^2,
\qquad
\Delta_B=4(r^2-d_\perp^2)=4q^2
```

without conflating documentation, runtime and scientific claim.

## Required receipt fields

```yaml
federation_id: PBIP-L1-FED-V1
formula_id: PBIP-L1
implementation_repo: TOKEN_VAZIO
implementation_commit: TOKEN_VAZIO
implementation_path: TOKEN_VAZIO
compiler_or_runtime: TOKEN_VAZIO
platform: TOKEN_VAZIO
cases:
  secant: TOKEN_VAZIO
  tangent: TOKEN_VAZIO
  no_real_intersection: TOKEN_VAZIO
numeric_tolerance: TOKEN_VAZIO
stdout_sha256: TOKEN_VAZIO
artifact_sha256: TOKEN_VAZIO
source_observed: true
build_proven: false
runtime_proven: false
device_proven: false
reproduced: false
claim_allowed: false
```

`TOKEN_VAZIO` is not zero, false, or failure; it records an unfilled evidence slot governed here by `CLOSURE_L1` until a concrete receipt fills the field.

## Finite route

```text
SOURCE_OBSERVED
  -> FORMULA_BOUND
  -> IMPLEMENTED
  -> BUILD_PROVEN
  -> RUNTIME_PROVEN
  -> DEVICE_PROVEN (when device-specific claim exists)
  -> REPRODUCED
  -> CLAIM_REVIEW
```

No state may be skipped by prose.

## Deterministic minimum cases

A future harness must include at least:

1. `r=5, d_perp=3` -> `q^2=16`, `Delta_B=64`, `SECANT`;
2. `r=5, d_perp=5` -> `q^2=0`, `Delta_B=0`, `TANGENT`;
3. `r=5, d_perp=6` -> `q^2=-11`, `Delta_B=-44`, `NO_REAL_INTERSECTION`.

These values are test vectors, not runtime receipts.

## Cross-repository route

```text
Matem-tica-#23 -> Mapa/PBIP-L1-FED-V1
ChipQuantum#68 -> implementation reference
Vectras-VM-Android -> executable consumer candidate
Rafaelia_Private -> low-level consumer candidate
RafPolimata -> evidence receipt route
papers#73 -> academic/claim ledger
RLL -> cross-domain consumer reference, no physical implication
```

## R3

- `F_ok`: receipt schema and deterministic test vectors are fixed.
- `F_gap`: no build/runtime/artifact hashes exist yet; gaps remain explicitly governed by `CLOSURE_L1`.
- `F_next`: a concrete consumer must emit a receipt using this schema.
