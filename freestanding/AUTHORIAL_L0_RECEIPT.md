# RAFAELIA Freestanding Authorial L0 Receipt V1

Scope: `freestanding/` on branch `codex/freestanding-authorial-zero-deps-v1-20261007`.
Base: `main@607272c8d69836678a5a7cf69d17d40e2418827b`.

## Delta

- Added `freestanding/include/raf_fs_authorial.h` as a caller-owned L0 item descriptor.
- Added `freestanding/tests/authorial_probe.c` to make the descriptor compile-inspectable.
- Added `freestanding/tests/verify_authorial.sh` to compile the probe across the six OS-neutral target triples already used by L0 gates.
- Routed the gate through `make execution-boundary-audit` and `make authorial-freestanding`.

## Boundary

- `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`.
- `external_dep_count` is explicitly set to `0` for the authorial item descriptor.
- `TOKEN_VAZIO` is represented by `RAF_FS_AUTHORIAL_TOKEN_VAZIO`, a nonzero sentinel; governance `CLOSURE_L12`.
- No hosted header, libc call, heap allocation, syscall instruction, inline assembly or external helper is introduced by the authorial primitive.
- The fold is a deterministic local identity fold, not a cryptographic or scientific validation claim.

## Evidence

Local scratch evidence from this change:

```text
sh -n freestanding/tests/verify_authorial.sh                     PASS
grep forbidden tokens in raf_fs_authorial.h + authorial_probe.c   PASS (no matches)
cc -std=c99 -ffreestanding -fno-builtin -nostdinc ... -c          PASS
nm -u /tmp/rafaelia-authorial-native.o                            PASS (empty)
nm -g --defined-only /tmp/rafaelia-authorial-native.o             PASS (raf_fs_authorial_probe)
```

Cross-target `verify_authorial.sh` is prepared for `clang -target` execution. Physical runtime/device evidence remains `TOKEN_VAZIO (CLOSURE_L12)` until a same-scope execution receipt exists.
