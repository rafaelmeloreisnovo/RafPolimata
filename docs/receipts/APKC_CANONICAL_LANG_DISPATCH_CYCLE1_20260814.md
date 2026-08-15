# ApkC — Canonical Language Dispatch — Cycle 1 — 2026-08-14

Status: `IMPLEMENTED_CANDIDATE / claim_allowed=false`

## Origin / predecessor

- Repository: `rafaelmeloreisnovo/RafPolimata`
- Base `main`: `c6658c2486bcff60213d12e19ce9d5a5e4a81e08`
- Predecessor audit: `docs/receipts/APKC_CAPABILITY_ACTION_PROOF_AUDIT_20260814.md`
- Branch: `fix/apkc-canonical-lang-dispatch-20260814`
- Method: fix earliest falsified invariant first; append evidence; do not promote later gates by implication.

## Fato observed at predecessor

The predecessor audit recorded:

- `G-S1`: language identity mismatch between `apkc_branchless_handler.h` documentation/private ordering and canonical `LP_*` values from `lang_profile.h`;
- `G-S2`: the active branchless handler called `compile_universal()`, which ignored language identity and used permissive pattern fallback;
- `G-S3/G-S4`: current statement/expression frontends still contain structural-only and placeholder semantics;
- `G-S5`: VM semantic execution is not yet active.

## Delta implemented

### 1. One canonical language identity

`apkc_branchless_compile()` now accepts only canonical `LP_*` values from `lang_profile.h`.

There is no private ordering such as `0=Python, 1=Go, ...` at this boundary.

Unsupported or non-branchless profiles are rejected fail-closed with:

```text
APKC_BRANCHLESS_UNSUPPORTED_LANG
```

The handler persists the exact selected identity in:

```text
lang_profile_id
```

### 2. Active path enters language-specific frontends

A new dispatcher was added:

```text
Apkc/apkc_language_dispatch.h
```

It maps the currently enabled branchless profiles:

```text
LP_C
LP_RS
LP_JAVA
LP_PY
LP_JS
LP_GO
LP_SWIFT
```

to their existing language-specific scanner/statement frontend structures.

The active `apkc_branchless_compile()` path no longer calls the legacy pattern-only `compile_universal()` dispatcher.

### 3. Semantic proof is explicit and not promoted

The handler now records:

```text
frontend_kind = APKC_FRONTEND_LANGUAGE_SPECIFIC
semantic_proof = APKC_SEMANTIC_UNPROVEN
```

This is deliberate.  Routing to a real frontend does not close `G-S3`, `G-S4` or `G-S5`.

### 4. Exact instruction stream prepared for the future execution oracle

The generated master instruction stream is copied byte-for-byte by instruction into `ExecutionContext.code` before ARM64 encoding.

Execution is still disabled because current `RET`/control-flow and placeholder semantics would make an action claim premature.

## Test gate

Added:

```text
tests/test_apkc_canonical_lang_dispatch.c
.github/workflows/apkc-canonical-lang-dispatch.yml
```

The gate checks:

1. `LP_PY != 0`, preventing resurrection of the old private numbering;
2. exact `LP_PY` identity survives the handler;
3. all seven enabled branchless profiles route through the canonical dispatcher;
4. `LP_ASM`/legacy id zero is rejected rather than reinterpreted as Python;
5. `LP_CPP` is rejected because its profile does not enable branchless;
6. out-of-range ids fail closed;
7. empty input fails closed;
8. generated machine code is copied into the execution-oracle buffer;
9. `semantic_proof` remains unpromoted and `steps_executed==0`.

## Invariants preserved

```text
IDEA != IMPLEMENTATION != EXECUTION != EVIDENCE != CLAIM
COMPILE_PASS != SEMANTIC_PASS
FRONTEND_SELECTED != SEMANTIC_EQUIVALENCE
VM_BUFFER_READY != VM_EXECUTED
TOKEN_VAZIO != PASS
claim_allowed=false
```

## Closure state

```text
G-S1 = IMPLEMENTED_CANDIDATE_PENDING_CI
G-S2(active dispatcher) = ROUTED_TO_LANGUAGE_SPECIFIC_FRONTEND
G-S3 = OPEN
G-S4 = OPEN
G-S5 = OPEN
G-A1 = OPEN
G-A2 = OPEN
G-A3 = OPEN
```

`G-S2` is not marked fully closed because the selected frontends still contain structural-only semantics.  The important delta is that the active path no longer bypasses language identity through the universal pattern fallback.

## F_ok

- canonical LP identity enforced;
- unsupported language ids fail closed;
- active branchless path uses language-specific frontend routing;
- frontend/semantic proof states separated;
- exact VM instruction stream copied into the future oracle buffer;
- focused CI gate created.

## F_gap

- declarations/assignments still consume expressions without equivalent codegen;
- function bodies are partly structural-only;
- comparisons/equality/function calls still contain placeholders or simplifications;
- VM action oracle remains disabled;
- native layout/relocation and CALL/RET remain later independent gates;
- physical Android evidence remains `TOKEN_VAZIO`.

## F_next

Run the focused gate.  If green, promote only `G-S1` to code+CI evidence and start Cycle 2 at the earliest semantic falsifier: literal `return 42`, followed by precedence `2 + 3 * 4`.  The next cycle must compare executed VM result to an expected value instead of checking only instruction presence.
