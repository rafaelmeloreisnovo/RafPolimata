# RafBBS authorial zero-dependency receipt

Scope: the pure RafBBS V1 slice only. Hosted operators remain adapters and are
not included in this claim.

## Declared pure set

- rafbbs_types.h
- rafbbs_freestanding.h
- rafbbs_baremetal.h
- rafbbs_crc32_core.h
- rafbbs_sha256_core.h
- tests/rafbbs_authorial_probe.c

## Gate contract

freestanding/tests/verify_authorial_zero_dep.sh:

- rejects hosted/system includes, host/runtime calls, syscall instruction
  strings, inline assembly and external declarations in the declared set;
- compiles the caller-owned probe with -ffreestanding -nostdinc -fno-builtin;
- checks the six OS-neutral targets:
  x86_64, i686, armv7a, aarch64, riscv32, riscv64;
- rejects undefined object symbols and requires the exported probe symbol.

rafbbs_build.sh freestanding remains a local known-vector and object-level
falsifier. The compiler, shell and nm are evidence/factory tools, not runtime
dependencies of the pure objects.

## Boundary

This receipt establishes a source/build gate for the named RafBBS slice. It does
not establish hosted-adapter purity, physical bare-metal execution, device
coverage, semantic equivalence on every target, or claim promotion.

Unobserved runtime/device state remains TOKEN_VAZIO (CLOSURE_L12).
Repository-wide migration beyond this named slice remains TOKEN_VAZIO
(CLOSURE_L11).
