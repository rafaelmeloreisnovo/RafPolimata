---
applyTo: "freestanding/**,syscall/**"
---

# Freestanding L0 / raw syscall — path-specific Copilot instructions

Read `AGENTS.md`, `docs/AGENTES.md`, and the nearest scoped `AGENTS.md` before editing.

## Freestanding boundary

For `freestanding/**` preserve:

```text
libc=0 heap=0 GC=0 hosted-runtime=0 syscall=0 hidden-tail=0 undeclared-shadow=0
```

Do not add hosted headers. Do not silently raise the ISA floor. Use explicit architecture profiles for NEON/SVE/SME, SSE/AVX/AVX-512/AMX and RVV.

A branch may hand ownership to the next pipeline stage. For in-stage value selection, prefer a branchless mask/select only when semantics and codegen remain correct.

## Syscall boundary

For `syscall/**`, raw OS ABI instructions are allowed only inside that subtree. Document trap instruction, register assignment, clobbers and return convention. No libc/errno/TLS/cancellation/allocator/hidden retry wrapper belongs in the raw layer.

## Stubs

A stub is structural only. It must fail closed or remain explicitly unavailable; it must never emit a fake successful result. Missing runtime/device evidence is `TOKEN_VAZIO (CLOSURE_L12)`.

## Comments are contract

Canonical L0 headers must preserve the `RAFAELIA-L0-FILE-CONTRACT` comment block and its fields defined in `freestanding/COMMENT_CONTRACT.md`. Update comments and tests in the same change as semantics.

## Minimum validation

```sh
sh freestanding/tests/verify_contract.sh
sh freestanding/tests/verify_matrix.sh
sh syscall/tests/verify_matrix.sh
```

Run only the gates applicable to the changed subtree. Report exactly what ran; do not infer runtime/device proof from compile success.
