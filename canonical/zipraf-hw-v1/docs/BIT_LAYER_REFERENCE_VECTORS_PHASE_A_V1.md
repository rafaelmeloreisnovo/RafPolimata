# ZIPRAF Bit Layer Reference Vectors — Phase A V1

State: `PASS_CI_BOUNDED / PHYSICAL_TOKEN_VAZIO`  
Authority: `rafaelmeloreisnovo/RafPolimata`  
Parent: `ZIPRAF-BIT-LAYER-PROGRESSIVE-RASTER-V1`  
claim_allowed=false  
Gap governance: `CLOSURE_L1`

## Why Phase A exists

The canonical raster contract defines bit-plane reconstruction and progressive MSB reconstruction, but the historical block-mask semantics `M` and exact geometry function `G(M)` are not yet proven by an executable authority.

Therefore Phase A deliberately implements only the part that is already reconstructible without inventing geometry.

## Implemented

```text
byte C
→ L_0..L_7
→ exact full reconstruction
→ q-MSB reconstruction
→ deterministic layer accumulation
→ order-independent complete state
→ duplicate-conflict fail-closed
```

Widths `30` and `60` are carried in the canonical vector set as contract metadata. The reference header also accepts `120` because the parent specification explicitly names the refinement chain `30→60→120`. Width does not alter a byte result in Phase A.

The C reference is header-only, caller-buffer based, and uses no includes, libc, libm, heap, syscall or external runtime.

## Test mapping

- T-BL-001: **Phase-A covered exhaustively** for byte values 0..255 in the C gate; the JSON keeps a compact canonical witness set.
- T-BL-002: **Phase-A covered** for repeated bit-layer state.
- T-BL-006: **Phase-A covered** for q={1,2,4,8} under exact byte information ordering.
- T-BL-007: **Phase-A covered** for complete out-of-order layer arrival.
- T-BL-009: **partial only** — reference version/width/q header failure is covered; canonical block/header mismatch remains geometry-dependent.
- T-BL-003/004/005: **TOKEN_VAZIO_GEOMETRY**.
- T-BL-008: **TOKEN_VAZIO_INTERRUPTED_DOWNLOAD_CONTRACT**.
- T-BL-010: **TOKEN_VAZIO_CROSS_RUNTIME**.

## Boundary

```text
LAYER_MASK != BLOCK_MASK_M
BITPLANE_RECONSTRUCTION != GEOMETRY_G(M)
PHASE_A_PASS != T-BL-010_PASS
HOSTED_PASS != PHYSICAL_RUNTIME_PASS
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
```

No geometry state is synthesized from visual similarity, BitStack layout, BITRAF-1008 state, or repository naming.


## Exact-head evidence successor — 2026-09-25

Source head `9c2486553ecc4c1dec5acc85d03eaeea9aa8444e` completed the exact-head gates before merge.

Observed SUCCESS:
- CI `36103211996`, including `ZIPRAF Bit Layer reference vectors Phase A`;
- ZIPRAF HW Canonical V1 `36103211994`;
- Workflow Graph Audit `36103212055`;
- Internal Custody Ledger `36103212028`;
- Formal Science Orchestrator `36103212038`;
- M063 language completion freestanding contract `36103212057`.

Merged to `main` as `d52afbc38acf6d9580b32cbf9f7f259fa4afdf4b`.

This promotes only the Phase-A source and hosted/freestanding CI evidence. It does not promote `M`, `G(M)`, T-BL-010, Vectra/Termux runtime parity or physical execution.
