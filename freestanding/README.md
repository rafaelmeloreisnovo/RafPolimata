# RAFAELIA Freestanding L0

Status: active implementation contract.
Governance closures: implementation/topology `CLOSURE_L11`; runtime/device evidence `CLOSURE_L12`.

This directory is the OS-agnostic execution layer. It must remain buildable without libc, malloc, heap, garbage collection, hosted runtime, system calls, OS headers, hidden scalar tails or undeclared shadow state.

## Invariants

1. `freestanding/` never performs a syscall.
2. No dynamic allocation, heap or GC.
3. No hosted CRT/libc/TLS ownership.
4. Residual lanes stay explicit through mask/predicate/VL; no scalar compatibility tail.
5. No undeclared shadow state or hidden retry.
6. Hot-path primitives are macros or forced `static inline`/`always_inline`; gates reject unresolved helpers and calls.
7. Architecture instructions are isolated under `arch/`.
8. Comments define preconditions, clobbers, register ownership, ordering and evidence boundaries.
9. `void *state` is caller-owned; `void` avoids unnecessary return-object traffic but is not treated as magic optimization.
10. Missing implementation/runtime evidence remains `TOKEN_VAZIO (CLOSURE_L11/CLOSURE_L12)`.

## Separation

```text
freestanding/ -> OS-neutral CPU/register/memory primitives
syscall/      -> optional Linux ABI bindings, deliberately separate
```

Freestanding gates compile with `unknown-none`, `none-eabi` or `unknown-elf` target triples. Linux target/ABI assumptions are confined to `syscall/`.

## Build/codegen-gated primary profiles

- x86_64 SSE2 — XMM fixed block;
- i686 SSE2 — XMM fixed block with test-only register argument probe;
- x86_64 AVX2 — YMM 256-bit block;
- x86_64 AVX-512F — ZMM 512-bit block plus native `K1` masked residual memory;
- ARMv7-A/AArch32 NEON — D/Q 128-bit block;
- AArch64/ARM64 Advanced SIMD — Q/V 128-bit block;
- AArch64 SVE — P0/Z0 one-stage predicated block with explicit `consumed_lanes`;
- RV32/RV64 V — v0 one-stage `VL`-bounded block with explicit `consumed_lanes`;
- x86 AMX-TILE — direct `TMM0` register primitive with external state/config precondition;
- AArch64 SME — direct `ZA` register primitive with caller/environment state precondition.

`ARM64` and `AArch64` name the same 64-bit Arm execution state here.

## Additional register topology — metadata build-proven

- POWER64: GPR/FP/VSX/MMA topology;
- LoongArch64: GPR/FP/LSX/LASX/CSR topology;
- IBM z/s390x: GPR/access/control/FP/vector/PSW topology.

These three are metadata-only today; no executor claim is promoted by file presence or compilation.

## Maintenance/navigation

Read in this order:

1. `AGENTS.md`;
2. `ARCH_CONTRACT.md`;
3. `ABI_CONTRACT.md`;
4. `REGISTER_TOPOLOGY.md`;
5. `FLAGS.md`;
6. `NO_SHADOW_NO_TAIL.md`;
7. `COMMENT_CONTRACT.md`;
8. `STUB_POLICY.md`;
9. `GAPS.md` / `gaps.v1.json`;
10. `VALIDATION.md` / `CODEGEN_RECEIPT.md`.

## Gates

```text
verify_contract.sh          -> zero-runtime/source/comment contract
verify_matrix.sh            -> six primary scalar OS-neutral objects
verify_profiles.sh          -> SSE2/AVX2/AVX-512/NEON/Advanced-SIMD codegen
verify_scalable.sh          -> SVE/RVV predicate/VL codegen
verify_matrix_accel.sh      -> AMX TMM / SME ZA direct register codegen
verify_register_metadata.sh -> POWER/LoongArch/s390x topology objects
syscall/tests/verify_matrix.sh -> separate Linux syscall ABI matrix
```

Codegen gates reject unexpected helpers/calls/stack traffic in the relevant probes. Physical execution is a separate `CLOSURE_L12` evidence stage.

Canonical implementation entrypoints:

- `include/raf_fs_core.h`;
- `include/raf_fs_abi.h`;
- `include/raf_fs_registers.h`;
- `arch/raf_fs_vector.h`;
- `arch/raf_fs_scalable.h`;
- `arch/raf_fs_matrix.h`.
