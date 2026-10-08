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
- IBM z/s390x z13 vector facility — 128-bit one-block copy/select/zero using only `VR16..VR19`, outside the `FPR0..15` overlap region.
- POWER64 VSX/VMX — 128-bit one-block copy/select/zero pinned to `VR0..VR3` = `VSR32..VSR35`, outside the FPR-overlap view; MMA remains a separate open gap.

`ARM64` and `AArch64` name the same 64-bit Arm execution state here.

## Additional register topology — metadata build-proven

- POWER64: GPR/FP/VSX/MMA topology;
- LoongArch64: GPR/FP/LSX/LASX/CSR topology;
- IBM z/s390x: GPR/access/control/FP/vector/PSW topology; executor support is separately build/codegen-gated.

LoongArch64 remains metadata-only today. POWER64 has a bounded VSX/VMX fixed-vector slice and s390x has a bounded z13 vector executor; physical runtime remains `TOKEN_VAZIO (CLOSURE_L12)`.

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
9. `AUTHORIAL_ZERO_DEPENDENCY_V1.md`;
10. `GAPS.md` / `gaps.v1.json`;
11. `VALIDATION.md` / `CODEGEN_RECEIPT.md`.
12. `../docs/experiments/Z0_CONTEXTLESS_TOKENIZATION_V1.md` for the zero-context byte-observation experiment.

## Evidence gate / typed missingness (authorial, no runtime dependencies)

CLOSURE_L15 binds unresolved source/rights/runtime evidence in this exact scope; it does not promote any outcome.

- `include/raf_fs_evidence_gate.h`: deterministic, caller-owned gate classifier. Required source, rights, authority, target, evidence rule, execution observation, outcome, artifact identity, exact HEAD and receipt each remain independently typed.
- `tests/raf_fs_evidence_gate_test.c`: 21 positive and negative assertions, including `TOKEN_VAZIO != 0`, `NOT_RUN != FAIL` and invalid input rejection.
- `tests/verify_evidence_gate.sh`: hosted semantic harness plus ARMv7/AArch64 freestanding object compile and unresolved-symbol inspection. Requires available host compiler, Clang and nm as **build tools only**.
- `RAF_FS_GATE_PASS_SCOPED` is a policy outcome for provided marks, not a verification of their authenticity or a runtime/provider/scientific claim.
- P0: no root redistribution license was observed during this audit; new implementation does not grant reuse rights to other repositories. Resolve license/provenance per source before copying.

## Gates

```text
verify_evidence_gate.sh     -> typed evidence gate + ARMv7/AArch64 no-undefined object
verify_contract.sh          -> zero-runtime/source/comment contract
verify_authorial.sh         -> authorial item descriptor; external deps=0; six OS-neutral objects
verify_z0_token.sh          -> Z0 ABSENT/EMPTY/NUL/SPACE falsifier + six-ISA zero-helper compile
verify_matrix.sh            -> six primary scalar OS-neutral objects
verify_profiles.sh          -> SSE2/AVX2/AVX-512/NEON/Advanced-SIMD codegen
verify_s390x_vector.sh      -> z13 VR16..VR19-only fixed-vector codegen; rejects FPR/low-VR overlap
verify_power_vsx.sh         -> POWER64 VR0..VR3/VSR32..35 fixed-vector codegen; FPR-overlap view excluded
verify_scalable.sh          -> SVE/RVV predicate/VL codegen
verify_matrix_accel.sh      -> AMX TMM / SME ZA direct register codegen
verify_register_metadata.sh -> POWER/LoongArch/s390x topology objects
syscall/tests/verify_matrix.sh -> separate Linux syscall ABI matrix
```

Codegen gates reject unexpected helpers/calls/stack traffic in the relevant probes. Physical execution is a separate `CLOSURE_L12` evidence stage.

Canonical implementation entrypoints:

- `include/raf_fs_core.h`;
- `include/raf_fs_authorial.h`;
- `include/raf_fs_abi.h`;
- `include/raf_fs_registers.h`;
- `include/raf_fs_z0_token.h` — zero-context byte observer/tokenizer; no vocabulary/attention/weights/history;
- `arch/raf_fs_vector.h`;
- `arch/raf_fs_scalable.h`;
- `arch/raf_fs_matrix.h`.
