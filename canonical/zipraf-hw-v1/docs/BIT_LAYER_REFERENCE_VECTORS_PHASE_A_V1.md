# ZIPRAF Bit Layer Reference Vectors — Phase A V1

State: `IMPLEMENTED_UNTESTED`  
Authority: `rafaelmeloreisnovo/RafPolimata`  
Parent: `ZIPRAF-BIT-LAYER-PROGRESSIVE-RASTER-V1`  
claim_allowed=false

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

- T-BL-001: **Phase-A covered** for canonical byte samples; exhaustive 0..255 remains the next strengthening step.
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
