# AGENTS.md — RAFAELIA syscall adapters

Scope: everything under `syscall/`.

This subtree is intentionally outside `freestanding/`. It may encode an OS ABI, but it must never leak that ABI back into L0.

## Read order

Read repository `AGENTS.md`, `docs/AGENTES.md`, `freestanding/AGENTS.md`, and the OS/architecture ABI document relevant to the edited adapter.

## Boundary

```text
freestanding/ = ISA/register/memory primitives, OS agnostic
syscall/      = optional OS ABI bindings
```

- Syscall numbers are caller/OS-ABI owned unless a versioned ABI table is explicitly introduced.
- No libc wrapper, errno translation, TLS, cancellation point, allocator, GC or hidden retry in raw bindings.
- Register assignments, clobbers, return-value convention and trap instruction must be documented per ISA.
- Do not use a raw syscall adapter as evidence that L0 itself depends on syscalls.
- Unsupported OS/ISA combinations fail closed; never return a fake success value.

A stub is allowed only as a structural/evidence placeholder and must be explicitly unavailable. Missing runtime/device evidence remains `TOKEN_VAZIO (CLOSURE_L12)`.

## Validation

Compile the syscall probe separately from the L0 probe. A passing syscall adapter gate does not promote the freestanding gate, and vice versa.

Keep source, ABI, runtime and device evidence separated in the handoff.
