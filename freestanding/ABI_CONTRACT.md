# RAFAELIA L0 — Freestanding Internal ABI Contract

This contract is deliberately **not** a host/OS ABI. AVX2, AVX-512, NEON, SVE/SME and RVV are ISA/register profiles; SysV, Windows x64, AAPCS, Linux EABI and syscall conventions remain outside the L0 hot core.

## Internal ABI law

```text
entry/OS ABI        = caller-owned
state ownership     = caller-owned `void *state`
heap / GC           = 0
libc / hosted CRT   = 0
TLS                 = 0
syscall             = 0
red-zone reliance   = 0
external hot helper = 0
hidden scalar tail  = 0
undeclared shadow   = 0
```

Production primitives are macros or forced `static inline`; therefore no L0 call boundary is required when the caller permits inlining. Exported symbols under `tests/` are probes only and obey the compiler target ABI solely so generated code can be inspected.

## Primary profiles

| profile | target | scalar | vector/predicate | L0 role |
|---|---|---|---|---|
| X64-SCALAR | x86_64 | RAX..R15 | SSE2 baseline | portable x86-64 floor |
| X64-AVX2 | x86_64 + AVX2 | RAX..R15 | YMM0..YMM15 | 256-bit fixed blocks |
| X64-AVX512 | x86_64 + AVX-512F | RAX..R15 | ZMM0..ZMM31 + K0..K7 | 512-bit fixed/predicated blocks |
| I686-SCALAR | i686 | EAX..EDI | x87; SSE only when explicitly enabled | 32-bit x86 floor |
| ARM32 | ARMv7-A | R0..R15 | NEON Q0..Q15 when profile enables it | 32-bit ARM floor/vector profile |
| ARM64 | AArch64 | X0..X30 | V0..V31 baseline | 64-bit Arm floor |
| ARM64-SVE | AArch64 + SVE | X0..X30 | Z0..Z31 + P0..P15 + FFR | scalable vector profile |
| ARM64-SME | AArch64 + SME | X0..X30 | Z/P + ZA (and ZT0 when SME2) | matrix/streaming profile |
| RV32 | RISC-V RV32 | x0..x31 | v0..v31 when V enabled | 32-bit RISC-V |
| RV64 | RISC-V RV64 | x0..x31 | v0..v31 when V enabled | 64-bit RISC-V |

## No-tail rule

A fixed-vector primitive operates on exactly one complete architectural block. A residual stage may use a predicate/mask when the ISA provides it. On fixed-vector ISAs without masked memory, residual processing requires the caller to provide a full accessible block and an explicit lane mask; L0 never invents a hidden scalar cleanup loop.

## Register preservation

L0 does not promise SysV/AAPCS/Windows callee-save semantics internally because the hot primitives are inlined into the caller. Inline assembly declares every clobber it owns. A standalone entry layer that chooses to expose a callable symbol must apply the selected external ABI separately.

## Stack policy

L0 primitives do not allocate dynamically and must not rely on a red zone. Profile tests compile x86-64 with `-mno-red-zone`. Avoiding every compiler spill is a code-generation property and is verified at the generated-object/assembly gate, not claimed from source text alone.

## Promotion states

```text
SOURCE_IMPLEMENTED -> BUILD_PROVEN -> CODEGEN_PROVEN -> RUNTIME_PROVEN -> DEVICE_PROVEN
```

Missing later stages stay `TOKEN_VAZIO (CLOSURE_L12)`; compile success is never promoted to physical execution.