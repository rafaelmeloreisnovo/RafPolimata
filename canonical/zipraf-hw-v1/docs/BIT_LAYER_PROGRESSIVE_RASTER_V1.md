# ZIPRAF Bit Layer Progressive Raster V1

Status: SPEC_CANONICAL / CLAIM_GATED  
claim_allowed=false

## Scope

This document formalizes the Bit Layer + progressive raster contract. It does not redefine ZIPRAF binary ABI, ZIPRAF_OMEGA_FULL container authority, or physical-performance claims.

SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM.

## Core equations

1. Bit-plane decomposition

```text
C_xy = sum_{k=0..7} 2^k b_xyk
```

2. Block/layer/channel composition

```text
I = direct-sum_{i,j,k,c} B_{i,j,k,c}
```

3. Masked reconstruction

```text
I_M = R(B, M, G(M))
```

4. Geometry state

```text
G_t = G(M_t)
```

5. Conditional structural transition

```text
Delta M => Delta G
```

This implication is normative only for blocks marked structural by the decoder contract.

6. Progressive MSB reconstruction

```text
I^(q) = sum_{k=8-q..7} 2^k L_k
```

7. Width refinement

```text
G_30 -> G_60 -> G_120
```

8. Multiresolution reduction

```text
D_s(I)(u,v) = sum_{m,n} K_s(m,n) I(su+m,sv+n)
```

9. Reconstruction error

```text
E_s = ||I - U_s(D_s(I))||_2
```

10. Unified V1 state

```text
I^(t) = R(sum_k 2^k [M_t dot L_k], G(M_t), W_t)
```

## Determinism contract

For identical versioned decoder, bit-layer set, width and block mask:

```text
(W,q,M,version)_a = (W,q,M,version)_b
=> I_a = I_b and G_a = G_b
```

Ambiguous geometry is FAIL, not a fallback.

## Repository authority

- RafPolimata: math/spec/ABI orchestration.
- Vectras-VM-Android: LayersBit + framebuffer/raster adapter.
- termux-app-rafacodephi: bit-matrix + block/decoder adapter.
- RafGitTools: inspection/conversion/debug bridge.
- RLL: representation experiment only; never codec authority.
- GAIA / Rafaelia_Private: TOKEN_VAZIO until a concrete data-flow and authority boundary are proven.

## Minimum tests

- T-BL-001 byte 0..255 exact bit-plane reconstruction.
- T-BL-002 repeated M is deterministic.
- T-BL-003 non-structural block loss leaves G unchanged.
- T-BL-004 structural block loss changes G deterministically.
- T-BL-005 W 30->60->120 preserves declared coarse invariants.
- T-BL-006 q 1->8 monotonic information under preregistered metric.
- T-BL-007 out-of-order arrival converges to same complete state.
- T-BL-008 interrupted download remains self-identifying when contract permits.
- T-BL-009 header/block mismatch fails closed.
- T-BL-010 Vectra/Termux/reference vectors agree.

## R3

F_ok: formal contract + authority split.
F_gap: exact historical G(M) implementation and cross-runtime vectors.
F_next: implement deterministic vectors for W={30,60}, q={1,2,4,8}, block-loss/order masks.
