# Receipt — ZIPRAF Bit Layer Phase A Exact-Head CI PASS V1

Date: 2026-09-25  
Parent: `receipts/2026-09-25_ZIPRAF_BIT_LAYER_REFERENCE_VECTORS_PHASE_A_V1.md`  
Source head: `9c2486553ecc4c1dec5acc85d03eaeea9aa8444e`  
Merge: `d52afbc38acf6d9580b32cbf9f7f259fa4afdf4b`  
Gap governance: `CLOSURE_L1`  
claim_allowed=false

## Exact-head evidence

- CI `36103211996`: SUCCESS.
  - TOKEN_VAZIO gate: PASS.
  - ZIPRAF Bit Layer reference vectors Phase A: PASS.
  - ARM64 encoder golden tests: PASS.
  - ARM32 encoder golden tests: PASS.
- ZIPRAF HW Canonical V1 `36103211994`: SUCCESS.
- Workflow Graph Audit `36103212055`: SUCCESS.
- Internal Custody Ledger `36103212028`: SUCCESS.
- Formal Science Orchestrator `36103212038`: SUCCESS.
- M063 language completion freestanding contract `36103212057`: SUCCESS.

## Promoted state

```text
PHASE_A_SOURCE = PASS_CI_BOUNDED
T-BL-001_BYTE_0_255 = PASS_CI_BOUNDED
Q_1_TO_8_RECONSTRUCTION = PASS_CI_BOUNDED
OUT_OF_ORDER_COMPLETE_LAYERS = PASS_CI_BOUNDED
FREESTANDING_OBJECT_UNDEFINED = 0
M = TOKEN_VAZIO_CLOSURE_L1
G(M) = TOKEN_VAZIO_CLOSURE_L1
T-BL-010 = TOKEN_VAZIO_CLOSURE_L1
PHYSICAL_RUNTIME = TOKEN_VAZIO_CLOSURE_L1
```

No physical or cross-runtime claim is promoted by this receipt.
