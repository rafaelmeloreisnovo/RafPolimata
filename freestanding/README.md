# RAFAELIA Freestanding L0

Status: initial implementation contract.
Governance evidence closure: `CLOSURE_L12`.

This directory is the OS-agnostic execution layer. It must remain buildable without libc, malloc, heap, garbage collection, hosted runtime, system calls, or OS headers.

## Invariants

1. `freestanding/` never performs a syscall.
2. No dynamic allocation.
3. No GC or hidden runtime ownership.
4. No tail-handler fallback that silently drops to a scalar compatibility path; residual lanes are represented explicitly by masks/counts.
5. No shadow state that duplicates canonical state without a declared reason.
6. Hot-path primitives are header-only, macro-based, or `static inline`/`always_inline` so the linker does not need external helper symbols.
7. Architecture-specific instructions are isolated under `arch/`.
8. Comments define preconditions, clobbers, register ownership, ordering and evidence boundaries; comments are part of the code contract.
9. Branchless form is preferred where it reduces measured cost, but semantic correctness is never sacrificed for branch removal.
10. Missing architecture/runtime evidence is `TOKEN_VAZIO`, never a promoted claim.

## Separation

```text
freestanding/   -> pure CPU/register/memory primitives; OS independent
syscall/        -> optional OS ABI bindings generated from the same architecture contract
```

The freestanding layer may be compiled into kernels, firmware, boot stages, VM engines, Android native components or userspace objects without changing its semantic core. The caller owns entry/exit, memory map, stack policy and external I/O.

## Initial architecture set

- x86_64
- i686 / IA-32
- ARMv7-A / AArch32
- AArch64
- RISC-V RV32
- RISC-V RV64

See `ARCH_CONTRACT.md` and `include/raf_fs_core.h`.
