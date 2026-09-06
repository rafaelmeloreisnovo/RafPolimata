# GPT.md — GPT/ChatGPT adapter for RafPolimata

This is an adapter, not a second source of truth.

Required authority order:

1. `AGENTS.md`;
2. `docs/AGENTES.md`;
3. nearest scoped `AGENTS.md`;
4. subsystem source/tests/receipts;
5. this adapter.

For `freestanding/**`, read `freestanding/AGENTS.md` before editing. For `syscall/**`, read `syscall/AGENTS.md` as well.

## GPT execution discipline

- Work source-first and preserve current negative/contradictory evidence.
- Never convert implementation into execution evidence.
- Never convert object compilation into physical runtime proof.
- Preserve the L0 zero-runtime boundary: no libc, heap, GC, hosted runtime, syscalls, hidden scalar tail or undeclared shadow state.
- Use stubs only as explicit structural placeholders; never report unavailable behavior as success.
- Preserve file-level comment contracts and update comments together with code semantics.
- Keep raw syscall bindings outside `freestanding/`.
- Raw hexadecimal instruction encodings require a documented assembler/toolchain reason and a readable mnemonic equivalent.
- Unavailable runtime/device evidence remains `TOKEN_VAZIO (CLOSURE_L12)`.

For handoff, report only observed `F_ok`, open `F_gap`, and the smallest reproducible `F_next`.
