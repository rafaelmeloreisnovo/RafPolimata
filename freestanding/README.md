# RAFAELIA Freestanding L0

Status: active implementation contract.
Governance closures: implementation/topology `CLOSURE_L11`; runtime/device evidence `CLOSURE_L12`.

This directory is the OS-agnostic execution layer. It must remain buildable without libc, malloc, heap, garbage collection, hosted runtime, system calls, or OS headers.

## Invariants

1. `freestanding/` never performs a syscall.
2. No dynamic allocation.
3. No GC or hidden runtime ownership.
4. No tail-handler fallback that silently drops to a scalar compatibility path; residual lanes are represented explicitly by masks/counts/predicates/VL.
5. No shadow state that duplicates canonical state without a declared reason.
6. Hot-path primitives are header-only, macro-based, or `static inline`/`always_inline` so the linker does not need external helper symbols.
7. Architecture-specific instructions are isolated under `arch/`.
8. Comments define preconditions, clobbers, register ownership, ordering and evidence boundaries; comments are part of the code contract.
9. Branchless form is preferred where it reduces measured cost, but semantic correctness is never sacrificed for branch removal.
10. Missing implementation/runtime evidence is `TOKEN_VAZIO (CLOSURE_L11/CLOSURE_L12)`, never a promoted claim.

## Separation

```text
freestanding/   -> pure CPU/register/memory primitives; OS independent
syscall/        -> optional OS ABI bindings generated from the same architecture contract
```

The freestanding layer may be compiled into kernels, firmware, boot stages, VM engines, Android native components or userspace objects without changing its semantic core. The caller owns entry/exit, memory map, stack policy and external I/O.

## Architecture/profile set

Scalar floor:

- x86_64;
- i686 / IA-32;
- ARMv7-A / AArch32;
- AArch64 / ARM64;
- RISC-V RV32;
- RISC-V RV64.

Build/codegen-gated vector profiles:

- x86_64 AVX2 — 256-bit fixed block;
- x86_64 AVX-512F — 512-bit fixed block;
- ARMv7-A NEON — 128-bit fixed block;
- AArch64 Advanced SIMD — 128-bit fixed block;
- AArch64 SVE — one predicated scalable block with explicit `consumed_lanes`;
- RV32/RV64 V — one `VL`-bounded scalable block with explicit `consumed_lanes`.

Physical runtime remains a separate evidence stage under `CLOSURE_L12`.

## Maintenance/navigation

Read in this order when changing L0:

1. `AGENTS.md` — scoped agent rules;
2. `ARCH_CONTRACT.md` — ISA/ownership boundary;
3. `ABI_CONTRACT.md` — OS-agnostic internal ABI/profile law;
4. `REGISTER_TOPOLOGY.md` — GPR/SIMD/vector/predicate/matrix/control geometry;
5. `FLAGS.md` — compiler/link profile;
6. `NO_SHADOW_NO_TAIL.md` — state/residual invariant;
7. `COMMENT_CONTRACT.md` — mandatory code-comment schema;
8. `STUB_POLICY.md` — fail-closed structural placeholder rule;
9. `GAPS.md` / `gaps.v1.json` — human + machine-readable open gaps;
10. `VALIDATION.md` / `CODEGEN_RECEIPT.md` — evidence and limitations.

The source gate `tests/verify_contract.sh` enforces zero-runtime rules and file-comment contracts. `verify_profiles.sh` verifies fixed-vector profiles; `verify_scalable.sh` verifies SVE/RVV one-stage profiles; both reject unresolved helpers/calls/stack traffic in their probes.

Canonical implementation entrypoints:

- `include/raf_fs_core.h` — scalar/fixed-width residual primitives;
- `include/raf_fs_abi.h` — ABI/profile geometry;
- `include/raf_fs_registers.h` — register-class topology metadata;
- `arch/raf_fs_vector.h` — AVX2/AVX-512/NEON/Advanced SIMD fixed blocks;
- `arch/raf_fs_scalable.h` — SVE/RVV scalable one-stage blocks.
