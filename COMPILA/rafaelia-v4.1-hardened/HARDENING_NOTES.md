# RAFAELIA V4.1 Hardening Notes — RafPolimata

Source: `RAFAELIA_COMPLETE_v4.zip`  
SHA-256: `d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454`  
State: `VERIFIED_LIMITED`  
`CLAIM_ALLOWED=false`

## Purpose

Preserve the V4 source lineage while closing host-verifiable P0 failures. This directory is append-only evidence under RafPolimata's `COMPILA` convention. It does not claim ARM32, Android-device, or GPU-kernel completion.

## P0 fixes applied

1. **Commit gate** — `rafaelia_core.c` now checks LOAD|PROCESS|VERIFY before setting COMMIT; COMMIT is no longer required before it can be set.
2. **CPU topology parser** — Linux CPU-list forms such as `0-7` and comma-separated ranges are counted instead of being parsed as a single leading integer.
3. **GPU fallback reporting** — fallback identifies CPU scalar/NEON without promoting an unavailable GPU path.
4. **Baremetal include/build** — corrected `baremetal.h` -> `baremetal_nomalloc.h`, repaired AVX2 preprocessor syntax, made `O_CLOEXEC` conditional, and removed warning-causing dead state.
5. **Java native-library initialization** — `RafaeliaCore.java` now assigns the blank `static final` exactly once after try/catch.
6. **Orchestrator OOB** — hexadecimal buffer enlarged to include its terminator.
7. **Memory CRC initialization** — zeroed memory buffers receive a matching initial CRC, removing false CRC errors before first write.
8. **Scheduler frequency normalization** — detected cluster frequencies are normalized into Q16 instead of overflowing the semantic scale.
9. **Executable memory tier** — the lowest execution band maps to RAM rather than non-executable storage.
10. **Fail-closed build paths** — `build_all.sh`, `rafaelia_master.sh`, and `termux_arm32_build.sh` no longer convert known compile/run failures into PASS/log-and-continue paths. Non-target ARM execution is reported as `TOKEN_VAZIO` rather than success.
11. **Android baseline** — new `Android.mk` builds only C/JNI sources; ARM32 `.S` is excluded until architecture-specific closure rather than being incorrectly reused for arm64.
12. **Reproducible host verifier** — `verify_hardened.sh` executes the local evidence gates and emits explicit `TOKEN_VAZIO` states.

## Verified after patch

- shell syntax: PASS
- `rafaelia_core.c`: Clang ASan+UBSan build/run PASS; sanitizer stderr empty
- core observed receipt: `COMMITS=42`, `ROLLBACKS=0`, `TOTAL_PTS=1008`, `GPU=CPU-SCALAR`
- `rafaelia_orchestrator.c`: Clang ASan+UBSan build/run PASS; `OK=42`, `CRC_ERR=0`, GPU path reports CPU fallback; sanitizer stderr empty
- `baremetal_nomalloc.c`: `-Wall -Wextra -Werror` compile PASS
- `RafaeliaCore.java`: `javac -Xlint:all` PASS

## Open evidence gates

- ARMv7 assembly cross-check: **0/9 assembled** with the current Clang integrated assembler target. This is not promoted to device failure or device success.
- Physical ARM32 Termux execution: `TOKEN_VAZIO`.
- Android NDK ABI matrix + physical device run: `TOKEN_VAZIO`.
- Real OpenCL/Vulkan compute execution: `TOKEN_VAZIO`; current path is CPU fallback.

The next release should not be called fully complete until those gates have independent receipts.

## Repository reconstruction form

RafPolimata stores this delta as five ordered text fragments. The materializer concatenates them and requires SHA-256 `739d0123f953c3bde2546095788dc2fc62143fcf0e016997b2ce2615378c4c68` before patch application. Fragmentation is storage-only; it does not alter the logical patch.
