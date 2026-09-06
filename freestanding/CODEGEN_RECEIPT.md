# RAFAELIA L0 — codegen/build receipt

Evidence type: historical CI execution bound to an exact commit/run; **not** physical-runtime evidence and not a self-referential current-head PASS.

Reference execution:

```text
commit      = d63b01694ee30ad7fc83416013844b9b778b4078
workflow    = Freestanding L0
run_id      = 34017063317
job         = six-isa-zero-runtime
compiler    = Ubuntu clang 18.1.3
result      = PASS
runtime     = TOKEN_VAZIO (CLOSURE_L12)
```

## Executed gates at that commit

| family | profiles | observed gate result |
|---|---|---|
| scalar L0 | x86_64, i686, ARMv7-A, AArch64, RV32, RV64 | 6/6 OS-neutral object compile; unresolved helpers = 0 |
| fixed vector | x86_64 SSE2, i686 SSE2, AVX2, AVX-512F/K, ARMv7 NEON, AArch64 Advanced SIMD | 6/6; helper/call/stack rejection + expected register-class markers |
| scalable vector | AArch64 SVE, RV32 V, RV64 V | 3/3; predicate/VL markers + helper/call/stack rejection |
| matrix register | x86 AMX-TILE, AArch64 SME | 2/2; direct TMM0/ZA register instruction markers + helper/call/stack rejection |
| extra topology | POWER64, LoongArch64, s390x | 3/3 metadata-only OS-neutral objects; no executor claim |
| syscall adapter | Linux x86_64, i686, ARMv7, AArch64, RV32, RV64 | 6/6 compile, outside freestanding L0 |

## OS-neutral proof boundary

Freestanding targets in the current gates use:

```text
x86_64-unknown-none
i686-unknown-none
armv7a-none-eabi
aarch64-none-elf
riscv32-unknown-elf
riscv64-unknown-elf
powerpc64le-unknown-none
loongarch64-unknown-none
s390x-unknown-none
```

Linux target/ABI assumptions are intentionally restricted to `syscall/` tests.

## Fixed-vector codegen invariants

The fixed-vector probe rejects any observed:

```text
undefined external helper
call / callq / bl / blx
RSP/RBP/ESP/EBP or push/pop stack traffic
missing required architectural register class
```

Observed required classes include `XMM0`, `YMM0`, `ZMM0`, ARM NEON/Advanced-SIMD registers and AVX-512 `K1`. The AVX-512 profile additionally requires masked `vmovdqu32` codegen using `K1`, so a partial residual need not fall into a scalar cleanup loop.

### i686 clarification

An older exported `cdecl` scalar inspection probe showed callee-save stack traffic. The explicit i686 SSE2 profile now uses `-mregparm=3` **only for the codegen probe** to isolate the inlined L0 from an external cdecl argument-passing convention. The production hot primitive remains forced inline; this does not declare `regparm(3)` as a universal external ABI.

## Scalable-vector invariant

SVE and RVV execute exactly one hardware-sized stage per call:

```text
SVE -> predicate P0 + Z0 -> consumed_lanes
RVV -> vsetvli + v0      -> consumed_lanes
```

Continuation belongs to the next pipeline stage/caller; no internal scalar tail loop is generated.

## Matrix-register boundary

The AMX/SME probe establishes direct architectural register ownership only:

```text
AMX -> tilezero TMM0
SME -> zero {ZA}
```

AMX tile configuration/XSTATE and SME state enablement are environment/caller preconditions. This receipt does not claim a complete AMX/SME data pipeline or device execution.

## Claim boundary

```text
SOURCE_IMPLEMENTED != BUILD_PROVEN != CODEGEN_PROVEN != RUNTIME_PROVEN != DEVICE_PROVEN
```

Only the gates listed above are promoted. Physical execution remains `TOKEN_VAZIO (CLOSURE_L12)` until same-artifact runtime receipts exist.
