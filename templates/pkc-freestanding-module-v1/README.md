# Copy template — PKC freestanding module V1

Copy this directory when creating a new PKC-compatible module.

Governance: `CLOSURE_L2` (target runtime) + `CLOSURE_L12` (license).

## Structural rule

Human vocabulary is metadata/frontend. Machine semantics are canonical:

```text
many source words
      ↓
one operation id
      ↓
one implementation
      ↓
one emitted instruction sequence
```

Do not create one implementation per language synonym. This makes symbol and
instruction reduction structural instead of depending on linker-specific
identical-code folding.

## Required boundaries

- no headers from libc/libm;
- no heap;
- no syscall;
- no external runtime;
- caller-owned buffers;
- no undefined symbols in the freestanding object;
- strict builds treat shadowing as an error;
- strict builds disable sibling/tail-call optimization;
- no duplicated kernel for PT/EN aliases;
- warnings are bitfields/state, never terminal I/O.

## Files

- `module.h` — C freestanding template;
- `module.rs` — Rust `no_std` contract template; final-runtime parity must be proven per target;
- `module.pt.pkc` — word-assembly example;
- `manifest.template.json` — provenance/version/license/contract metadata.

## License header

The header records copyright/provenance and the current policy reference.
It does not call the Berne Convention a software license.

Current status:

```text
LICENSE_STATUS=TOKEN_VAZIO_FINAL_LEGAL_TEXT
claim_allowed=false
```
