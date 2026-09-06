# RAFAELIA L0 — Architecture Contract

The architecture contract is a compile-time map, not an OS abstraction.

## Canonical fields

```text
ISA
WORD_BITS
PTR_BITS
ENDIAN
GPR_CLASS
SIMD_CLASS
VECTOR_CLASS
PREDICATE_CLASS
MATRIX_CLASS
MEMORY_BARRIER
RELAX_HINT
ZEROING_PRIMITIVE
REGISTER_CLOBBERS
```

No syscall number, file descriptor, errno model, loader contract or executable format belongs in this layer.

## Initial matrix

| target | word | pointer | endian | scalar register class | vector/SIMD path | barrier | relax |
|---|---:|---:|---|---|---|---|---|
| x86_64 | 64 | 64 | LE | RAX..R15 | SSE2 baseline; AVX/AVX2/AVX-512 optional | `mfence`/compiler memory | `pause` |
| i686 | 32 | 32 | LE | EAX..EDI | x87 baseline; MMX/SSE optional by target | `mfence` only when ISA permits; compiler barrier baseline | `pause` only when ISA permits |
| ARMv7-A | 32 | 32 | LE/BE profile-dependent | R0..R15 | NEON optional/profiled | `dmb ish` where available | `yield` |
| AArch64 | 64 | 64 | LE normally | X0..X30 | NEON baseline; SVE/SME optional | `dmb ish` | `yield` |
| RV32 | 32 | 32 | LE normally | x0..x31 or x0..x15 in E profile | V optional | `fence rw,rw` | implementation/profile hint |
| RV64 | 64 | 64 | LE normally | x0..x31 | V optional | `fence rw,rw` | implementation/profile hint |

## Pipeline law

A stage consumes explicitly owned state and either:

1. emits the next stage state;
2. emits an explicit residual mask/count;
3. emits a fault/status bit into caller-owned state.

There is no hidden tail loop and no shadow copy of canonical state.

```text
stage_n(state, lanes, residual)
    -> stage_n+1(state', lanes', residual')
```

A branch that only selects between values should be lowered to a mask/select primitive when profitable. A branch that changes pipeline ownership is allowed and must be documented as a stage transition.

## Symbol law

External helper symbols are forbidden inside the hot core. Prefer, in order:

1. compile-time constants/macros;
2. `static inline` with forced inline;
3. architecture inline assembly;
4. standalone assembly only when encoding/control cannot be expressed safely inline.

Raw hexadecimal instruction encodings are permitted only when the assembler cannot express a required instruction for the selected toolchain. Every raw encoding must document ISA, encoding, operands, clobbers, and a readable mnemonic equivalent.

## Evidence law

`SUPPORTED_BY_SOURCE` means the code path exists.
`BUILD_PROVEN` requires a target compiler receipt.
`RUNTIME_PROVEN` requires execution evidence.
`DEVICE_PROVEN` requires physical device evidence.

Until those exist, the later states remain `TOKEN_VAZIO`.
