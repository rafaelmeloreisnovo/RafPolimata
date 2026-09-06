# RAFAELIA syscall layer

`syscall/` is optional and is intentionally outside `freestanding/`.

The generation rule is:

```text
freestanding semantic stage
        + target ISA contract
        + OS syscall ABI contract
        = raw syscall binding
```

A syscall binding must never leak back into the L0 core.

## Initial Linux architecture bindings

- x86_64: `syscall`, number in RAX, args RDI/RSI/RDX/R10/R8/R9.
- i686: `int 0x80`, number in EAX, args EBX/ECX/EDX/ESI/EDI/EBP.
- ARMv7 EABI: `svc #0`, number in R7, args R0..R5.
- AArch64: `svc #0`, number in X8, args X0..X5.
- RV32/RV64: `ecall`, number in A7, args A0..A5.

The raw layer does not translate negative returns into `errno`, does not allocate, does not retry, and does not implement libc cancellation semantics. Syscall numbers remain caller/OS-owned rather than being copied into the architecture-neutral core.

Other OSes must live under their own directory because a syscall ABI is an OS+ISA contract, not an ISA property.
