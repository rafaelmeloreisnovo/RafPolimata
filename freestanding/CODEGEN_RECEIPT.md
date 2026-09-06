# RAFAELIA L0 — codegen receipt

Scope: compile-only probe, Clang 17, `-Os`, freestanding flags. This is source/codegen evidence, not physical-runtime evidence.

## Observed lowering

| target | value-selection lowering | hot-path branch | helper call |
|---|---|---:|---:|
| x86_64 | `test` + `cmovne` + `or` | 0 | 0 |
| i686 | `test` + `cmovne` + `or` | 0 | 0 |
| ARMv7-A | `cmp` + conditional `orreq` | 0 | 0 |
| AArch64 | `cmp` + `csel` + `orr` | 0 | 0 |
| RV32 | `snez` + `addi` + `andi` + `or` | 0 | 0 |
| RV64 | `snez` + `addi` + `andi` + `or` | 0 | 0 |

All six paths end in the architecture fence selected by `raf_fs_arch.h`; no loop or external helper call was emitted for the selection stage.

## i686 ABI note

The exported inspection probe generated `pushl %esi` / `popl %esi` because the probe itself is an externally visible SysV-style function and the compiler preserves a callee-saved register. This does **not** establish that an inlined L0 stage requires stack traffic. Production L0 primitives are `static inline`/macro-based and are absorbed into the caller; final register allocation belongs to the caller's ABI/entry contract.

Therefore the evidence states:

```text
L0 external helper symbol      = none required
selection branch               = none observed in six probes
selection call                 = none observed in six probes
exported-probe ABI stack use   = observed on i686
physical runtime               = evidence pending under CLOSURE_L12
```
