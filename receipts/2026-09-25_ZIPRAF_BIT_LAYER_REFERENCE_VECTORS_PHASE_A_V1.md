# Receipt — ZIPRAF Bit Layer Reference Vectors Phase A V1

Date: 2026-09-25
Kind: `CANONICAL_REFERENCE_VECTOR_PHASE_A / APPEND_ONLY`
Parent: `RafPolimata#350`
claim_allowed=false

## Delta

Materialized a bounded freestanding reference for byte bit-plane decomposition, progressive q-MSB reconstruction and deterministic layer arrival, plus machine-readable vectors and a stdlib-only vector validator.

The implementation intentionally does not define canonical block mask `M` or historical geometry `G(M)`.

## State at write

```text
SOURCE = IMPLEMENTED
C_HOSTED_TEST = NOT_RUN
C_FREESTANDING_OBJECT = NOT_RUN
UNDEFINED_SYMBOL_GATE = NOT_RUN
JSON_VECTOR_VALIDATOR = NOT_RUN
T-BL-010_CROSS_RUNTIME = TOKEN_VAZIO
G(M) = TOKEN_VAZIO
PHYSICAL_RUNTIME = TOKEN_VAZIO
```

R3=<F_ok: Phase-A source/vectors materialized without inventing geometry; F_gap: exact-head gates + M/G semantics + Vectra/Termux agreement; F_next: run canonical CI, then expose read-only consumer adapters over these vectors while preserving geometry TOKEN_VAZIO>.
