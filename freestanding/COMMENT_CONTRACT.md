# RAFAELIA L0 — file/comment contract

Comments in L0 are part of the engineering contract. They are not decorative summaries and must change with semantics.

## Required file marker

Every canonical implementation header under `freestanding/include/` and `freestanding/arch/` carries:

```text
RAFAELIA-L0-FILE-CONTRACT
```

and documents these fields:

```text
PURPOSE            what this file is responsible for
SCOPE              freestanding boundary and exclusions
PRECONDITIONS      compile-time/runtime assumptions
REGISTER_OWNERSHIP registers or caller-owned state touched/owned
CLOBBERS           architectural/compiler clobbers or NONE
MEMORY_ORDER       ordering semantics or NONE
TAIL_SHADOW        residual/tail and shadow-state rule
EVIDENCE           implementation/build/runtime boundary
```

## Function/primitive comments

A primitive that uses inline assembly, raw encoding, non-obvious register binding, barrier, aliasing or architecture-specific behavior documents directly beside the primitive:

- ISA/profile floor;
- inputs/outputs;
- bound registers when applicable;
- clobbers;
- ordering/side effects;
- whether it may create stack traffic at an external ABI boundary;
- equivalence/fallback rule.

## Raw hexadecimal encodings

Raw `.byte`, `.word`, `.long` or equivalent encoding is allowed only when the selected assembler cannot express a required instruction. The comment must include:

```text
ISA/profile
mnemonic equivalent
encoding value and endianness
operand mapping
clobbers/side effects
toolchain reason
```

A readable mnemonic is preferred whenever the assembler supports it.

## Evidence text

Use `IMPLEMENTED` only for source existence. Use `PASS` only with an executed named gate. Runtime/device absence remains `TOKEN_VAZIO (CLOSURE_L12)` rather than being inferred from build success.
