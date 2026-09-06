# RAFAELIA Freestanding L0 — current gap map

Scope: PR/branch `rafaelia/freestanding-l0-v1`. This is a maintenance map, not a claim that every repository gap is listed here.

Governance:

- implementation/topology gaps -> `CLOSURE_L11`;
- runtime/device evidence gaps -> `CLOSURE_L12`.

## Implemented baseline

| item | state | evidence boundary |
|---|---|---|
| scalar/ordering primitives: x86_64, i686, ARMv7-A, AArch64, RV32, RV64 | IMPLEMENTED | source + local cross-object compile receipt |
| branchless fixed-width copy/select/residual core | IMPLEMENTED | source + local codegen inspection |
| raw Linux syscall adapters for six targets | IMPLEMENTED, separate from L0 | source + local cross-object compile receipt |
| source contract gate | IMPLEMENTED | executable shell gate |

`IMPLEMENTED` above does not mean physical runtime proven.

## Open implementation gaps — CLOSURE_L11

| id | gap | current state | promotion gate |
|---|---|---|---|
| L0-GAP-001 | x86 fixed-vector executor (SSE2/AVX2/AVX-512) over the common lane contract | `TOKEN_VAZIO` | equivalence tests + no hidden helper/tail codegen |
| L0-GAP-002 | ARMv7/AArch64 NEON executor | `TOKEN_VAZIO` | scalar equivalence + ARM32/ARM64 object/codegen gates |
| L0-GAP-003 | AArch64 SVE/SME scalable/matrix executor | `TOKEN_VAZIO` | predicate/VL semantics + target compiler gate |
| L0-GAP-004 | RV32/RV64 V executor | `TOKEN_VAZIO` | VL/mask semantics + RVV target compiler gate |
| L0-GAP-005 | x86 AMX/matrix executor | `TOKEN_VAZIO` | explicit opt-in ISA profile + state/tile contract tests |
| L0-GAP-006 | automated unresolved-symbol/codegen inspection in CI for all six scalar targets | `TOKEN_VAZIO` | CI job proves zero unexpected calls/helpers |

## Open execution/evidence gaps — CLOSURE_L12

| id | gap | current state | promotion gate |
|---|---|---|---|
| L0-GAP-101 | current-head GitHub CI receipt after the latest maintenance commits | `TOKEN_VAZIO` | workflow run bound to current head |
| L0-GAP-102 | physical x86_64 runtime receipt | `TOKEN_VAZIO` | target execution + artifact/commit identity |
| L0-GAP-103 | physical i686 runtime receipt | `TOKEN_VAZIO` | target execution + artifact/commit identity |
| L0-GAP-104 | physical ARMv7 runtime receipt | `TOKEN_VAZIO` | target execution + artifact/commit identity |
| L0-GAP-105 | physical AArch64 runtime receipt | `TOKEN_VAZIO` | target execution + artifact/commit identity |
| L0-GAP-106 | physical RV32 runtime receipt | `TOKEN_VAZIO` | target execution/emulation scope explicitly identified |
| L0-GAP-107 | physical RV64 runtime receipt | `TOKEN_VAZIO` | target execution/emulation scope explicitly identified |
| L0-GAP-108 | raw Linux syscall runtime receipts per architecture | `TOKEN_VAZIO` | OS/ABI-specific execution, separate from L0 |

## Deliberate non-gaps

These are excluded by architecture, not missing accidentally:

- system-call numbers and OS entry/exit policy inside `freestanding/`;
- loader/ELF entry symbol inside generic L0;
- linker script and memory map inside generic L0;
- GPU runtime/driver orchestration inside L0;
- automatic scalar tail fallback;
- hidden allocator/GC/runtime ownership.

Use `gaps.v1.json` for machine-readable status. Never close a row by documentation alone.
