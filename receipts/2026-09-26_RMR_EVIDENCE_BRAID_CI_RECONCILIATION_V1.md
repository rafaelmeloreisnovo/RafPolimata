# Receipt — RMR Evidence Braid V1 CI Reconciliation — 2026-09-26

**Kind:** `STATE_SUCCESSOR / FAIL_CLOSED_REPAIR / APPEND_ONLY`  
**Governance:** `CLOSURE_L1`  
**claim_allowed:** `false`

## Parent state

- merged PR: `RafPolimata#354`
- merge commit: `fa37660ec937f72da9cb91e6dd8ce373e06bf4e5`
- original Evidence Braid commit: `93d6259c5c14c2f06affdca6c045184a7c8812ad`
- exact-head CI run observed on original commit: `36274611608`

Observed:
- Formal Science Orchestrator: PASS
- Internal Custody Ledger: PASS
- CI: FAIL
- failing step: `Validate TOKEN_VAZIO gates (Hotfix H1)`

## Root cause

The Evidence Braid source intentionally used `TOKEN_VAZIO` semantics for an unobserved evidence channel, but the newly added file did not bind that explicit unknown state to a repository closure record.

The repository validator therefore failed closed. This is governance behavior, not a cryptographic/KAT failure.

`TOKEN_VAZIO != PASS`; it remains governed by `CLOSURE_L1`.

## Reconciliation delta

1. Add file-level `CLOSURE_L1` provenance/reproducibility governance anchor to:
   - `canonical/rmr-evidence-braid-v1/rmr_evidence_braid_v1.c`
2. Add canonical CI execution gate:
   - hosted selftest with SHA-256/CRC32C KATs;
   - channel separation;
   - predecessor binding;
   - challenge binding;
   - unset-channel semantics;
   - freestanding x86-64 object;
   - freestanding ARMv7 cross-object;
   - freestanding AArch64 cross-object;
   - zero undefined symbols required for all three production objects.

## Boundaries

`SOURCE_PRESENT != CI_PASS != DEVICE_PASS != LEGAL_QUALIFICATION`

The initial merge remains historical and addressable. This successor does not rewrite it.

## Current gate

- successor exact-head CI: `TOKEN_VAZIO` pending provider observation; governed by `CLOSURE_L1`
- physical ARM32/ARM64 execution: `TOKEN_VAZIO`; governed by `CLOSURE_L1`
- BLAKE3 producer adapter: `TOKEN_VAZIO`; governed by `CLOSURE_L1`
- ApkC/ZIPRAF receipt adapter: `TOKEN_VAZIO`; governed by `CLOSURE_L1`

## F_next

Require canonical exact-head CI PASS on the successor. Only then create the ApkC/ZIPRAF receipt adapter that consumes the Evidence Braid root without replacing independent external verification.

claim_allowed=false
