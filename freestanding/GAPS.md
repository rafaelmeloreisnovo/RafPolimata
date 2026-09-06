# RAFAELIA Freestanding L0 — current gap map

Scope: PR/branch `rafaelia/freestanding-abi-vector-v2`. This is a maintenance map, not a claim that every repository gap is listed here.

Governance:

- implementation/topology gaps -> `CLOSURE_L11`;
- runtime/device evidence gaps -> `CLOSURE_L12`.

## Implemented baseline

| item | state | evidence boundary |
|---|---|---|
| scalar/ordering primitives: x86_64, i686, ARMv7-A, AArch64, RV32, RV64 | IMPLEMENTED + BUILD_PROVEN | source + cross-object compile gate |
| branchless fixed-width copy/select/residual core | IMPLEMENTED + CODEGEN_PROBED | source + codegen-oriented probe |
| x86_64 AVX2 one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_profiles.sh`; YMM path, no unresolved helper/call/stack in probe |
| x86_64 AVX-512F one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_profiles.sh`; ZMM path, no unresolved helper/call/stack in probe |
| ARMv7-A NEON one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_profiles.sh`; NEON D/Q path, no unresolved helper/call/stack in probe |
| AArch64 Advanced SIMD one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_profiles.sh`; Q/V path, no unresolved helper/call/stack in probe |
| AArch64 SVE one-stage predicated executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_scalable.sh`; `whilelo`/predicate path, explicit consumed lanes |
| RV32/RV64 V one-stage VL executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `verify_scalable.sh`; `vsetvli` path, explicit consumed lanes |
| ABI/register geometry metadata | IMPLEMENTED | `raf_fs_abi.h` + `raf_fs_registers.h`; no privileged access |
| raw Linux syscall adapters for six targets | IMPLEMENTED, separate from L0 | source + separate cross-object compile gate |
| source contract gate | IMPLEMENTED | executable shell gate |
| six-ISA unresolved-helper rejection | IMPLEMENTED | `verify_matrix.sh` rejects undefined helper/runtime symbols per object |

`BUILD/CODEGEN_PROVEN` above does **not** mean semantic equivalence on physical silicon or device/runtime proof.

## Open implementation gaps — CLOSURE_L11

| id | gap | current state | promotion gate |
|---|---|---|---|
| L0-GAP-001 | explicit SSE2 fixed-vector executor/profile (x86_64 baseline and optional i686 SSE2) | `TOKEN_VAZIO (CLOSURE_L11)` | one-block executor + no-helper/no-tail codegen gate |
| L0-GAP-003 | AArch64 SME/SME2 matrix/streaming executor (`ZA`/`ZT0`) | `TOKEN_VAZIO (CLOSURE_L11)` | streaming/matrix state contract + target compiler/codegen gate |
| L0-GAP-005 | x86 AMX matrix/tile executor (`TMM0..TMM7`) | `TOKEN_VAZIO (CLOSURE_L11)` | explicit opt-in ISA/state profile + tile-state codegen tests |
| L0-GAP-006 | AVX-512 native `K`-mask residual load/store path (current fixed executor uses explicit full-vector lane mask) | `TOKEN_VAZIO (CLOSURE_L11)` | K-register codegen + bounded masked-memory contract |

## Open execution/evidence gaps — CLOSURE_L12

| id | gap | current state | promotion gate |
|---|---|---|---|
| L0-GAP-102 | physical x86_64 scalar runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution + artifact/commit identity |
| L0-GAP-103 | physical i686 runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution + artifact/commit identity |
| L0-GAP-104 | physical ARMv7 runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution + artifact/commit identity |
| L0-GAP-105 | physical AArch64 runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution + artifact/commit identity |
| L0-GAP-106 | RV32 runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution/emulation scope explicitly identified |
| L0-GAP-107 | RV64 runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | target execution/emulation scope explicitly identified |
| L0-GAP-108 | raw Linux syscall runtime receipts per architecture | `TOKEN_VAZIO (CLOSURE_L12)` | OS/ABI-specific execution, separate from L0 |
| L0-GAP-109 | physical x86_64 AVX2 profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | CPUID/profile identity + current object execution |
| L0-GAP-110 | physical x86_64 AVX-512F profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | CPUID/XSTATE profile identity + current object execution |
| L0-GAP-111 | physical ARMv7 NEON profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | current object + device feature identity + execution |
| L0-GAP-112 | physical AArch64 SVE profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | SVE VL/feature identity + current object execution |
| L0-GAP-113 | RV32/RV64 V runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | VLEN/profile identity + current object execution |

## Dynamic evidence boundary

Current-head CI is deliberately **not** a static gap row. A file commit changes the head, so storing “current-head CI PASS” in the same commit would be self-invalidating. Current-head workflow state must be read from GitHub Actions and reported with its exact head/run identity.

## Deliberate non-gaps

These are excluded by architecture, not missing accidentally:

- system-call numbers and OS entry/exit policy inside `freestanding/`;
- loader/ELF entry symbol inside generic L0;
- linker script and memory map inside generic L0;
- GPU runtime/driver orchestration inside L0;
- automatic scalar tail fallback;
- hidden allocator/GC/runtime ownership.

Use `gaps.v1.json` for machine-readable status. Never close a row by documentation alone.
