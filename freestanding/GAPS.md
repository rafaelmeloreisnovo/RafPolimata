# RAFAELIA Freestanding L0 — current gap map

Scope: PR/branch `rafaelia/freestanding-abi-vector-v2`. This is a maintenance map, not a claim that every repository gap is listed here.

Governance:

- implementation/topology gaps -> `CLOSURE_L11`;
- runtime/device evidence gaps -> `CLOSURE_L12`.

## Implemented / build-codegen proven baseline

| item | state | evidence boundary |
|---|---|---|
| scalar/ordering primitives: x86_64, i686, ARMv7-A, AArch64, RV32, RV64 | IMPLEMENTED + BUILD_PROVEN | OS-neutral cross-object compile gate |
| branchless fixed-width copy/select/residual core | IMPLEMENTED + CODEGEN_PROBED | source + codegen-oriented probe |
| x86_64 SSE2 and i686 SSE2 one-block executors | IMPLEMENTED + BUILD/CODEGEN_PROVEN | XMM path; no unresolved helper/call/stack in probe |
| x86_64 AVX2 one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | YMM path; no unresolved helper/call/stack in probe |
| x86_64 AVX-512F one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | ZMM path; no unresolved helper/call/stack in probe |
| AVX-512 native K-mask residual copy | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `K1` + masked `vmovdqu32`; bounded inactive memory lanes; no scalar tail |
| ARMv7-A NEON one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | NEON D/Q path; no unresolved helper/call/stack in probe |
| AArch64 Advanced SIMD one-block executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | Q/V path; no unresolved helper/call/stack in probe |
| AArch64 SVE one-stage predicated executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `whilelo`/P0/Z0 path; explicit consumed lanes |
| RV32/RV64 V one-stage VL executor | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `vsetvli`/v0 path; explicit consumed lanes |
| x86 AMX direct TMM register primitive | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `tilezero TMM0`; external tile-state/configuration precondition |
| AArch64 SME direct ZA register primitive | IMPLEMENTED + BUILD/CODEGEN_PROVEN | `zero {za}`; caller/environment owns SME/ZA enablement |
| POWER64 / LoongArch64 / s390x register topology metadata | IMPLEMENTED + BUILD_PROVEN | three OS-neutral metadata-only objects; no executor claim |
| ABI/register geometry metadata | IMPLEMENTED | `raf_fs_abi.h` + `raf_fs_registers.h`; no privileged access |
| raw Linux syscall adapters for six targets | IMPLEMENTED, separate from L0 | separate Linux cross-object compile gate |

`BUILD/CODEGEN_PROVEN` above does **not** mean semantic equivalence on physical silicon or device/runtime proof.

## Open implementation gaps — CLOSURE_L11

| id | gap | current state | promotion gate |
|---|---|---|---|
| L0-GAP-003 | SME/SME2 operational matrix data path beyond direct ZA zero; includes ZA load/compute/store strategy and SME2 `ZT0` where supported | `TOKEN_VAZIO (CLOSURE_L11)` | explicit state/precondition contract + target codegen/equivalence gate |
| L0-GAP-005 | AMX operational tile data path beyond direct TMM0 zero (`tileloadd`/compute/`tilestored`) | `TOKEN_VAZIO (CLOSURE_L11)` | caller-owned tile config + bounded tile memory + codegen/equivalence gate |
| L0-GAP-007 | POWER64 VSX/MMA executor profile | `TOKEN_VAZIO (CLOSURE_L11)` | VSX/MMA instruction path + no-helper/no-tail codegen gate |
| L0-GAP-008 | LoongArch64 LSX/LASX executor profile | `TOKEN_VAZIO (CLOSURE_L11)` | LSX/LASX instruction path + alias-safe codegen gate |
| L0-GAP-009 | IBM z/s390x vector executor profile | `TOKEN_VAZIO (CLOSURE_L11)` | vector-facility profile + FPR/VR overlap-safe codegen gate |
| L0-GAP-010 | Qualcomm Hexagon/HVX register topology and executor | `TOKEN_VAZIO (CLOSURE_L11)` | available target toolchain + topology then executor gate |
| L0-GAP-011 | ARM M-profile MVE/Helium register topology and executor | `TOKEN_VAZIO (CLOSURE_L11)` | explicit M-profile target + MVE instruction/codegen gate |
| L0-GAP-012 | x86 APX `R16..R31` explicit profile | `TOKEN_VAZIO (CLOSURE_L11)` | assembler/compiler APX profile + register-allocation/codegen gate |

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
| L0-GAP-109 | physical x86_64 AVX2/SSE2 profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | CPUID/profile identity + current object execution |
| L0-GAP-110 | physical x86_64 AVX-512F/K profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | CPUID/XSTATE identity + current object execution |
| L0-GAP-111 | physical ARMv7 NEON profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | device feature identity + current object execution |
| L0-GAP-112 | physical AArch64 SVE/SME profile receipt | `TOKEN_VAZIO (CLOSURE_L12)` | SVE VL/SME state identity + current object execution |
| L0-GAP-113 | RV32/RV64 V runtime receipt | `TOKEN_VAZIO (CLOSURE_L12)` | VLEN/profile identity + current object execution |
| L0-GAP-114 | physical x86 AMX register-state receipt | `TOKEN_VAZIO (CLOSURE_L12)` | AMX/XSTATE/config identity + current object execution |

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
