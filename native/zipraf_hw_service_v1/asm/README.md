# ASM Backends — Measured-Only Policy

No assembly primitive is introduced here merely to increase the amount of ASM in the project.

Promotion sequence:

```text
portable C/Rust oracle
-> official vectors
-> candidate ASM
-> bit-exact differential test
-> target capability receipt
-> benchmark on exact artifact
-> code-size/stack/security review
-> CANDIDATE or VALIDATED
```

Current module ASM implementations: `TOKEN_VAZIO`.

Existing assembly kernels in `native/raf_hash_fabric_v1/asm/` remain separate and keep their own scope/evidence. The canonical integration layer may dispatch to them only when their exact primitive, architecture and validation state are compatible with the request.
