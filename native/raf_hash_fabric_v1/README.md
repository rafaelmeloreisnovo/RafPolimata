# RAF Hash Fabric V1

**Status:** `IMPLEMENTED` (source only; runtime evidence is `TOKEN_VAZIO`)  
**Owner:** `low-level-methods-maintainer`  
**Parent:** `README.md -> docs/INDEX.md -> this directory`  
**Base contract:** `Benchmark/raf_runtime_router.h` and `Benchmark/raf_types.h`

A seven-language, freestanding-oriented execution fabric for deterministic hash/Merkle workloads. It does **not** redefine MD5, SHA-2, SHA-3, BLAKE3, AES, Ed25519, or any other cryptographic primitive. Algorithm block sizes, rounds, constants and security boundaries remain those of their standards/reference implementations.

## Seven implementation languages

1. C11 freestanding (`c/`)
2. C++20 freestanding subset (`cpp/`)
3. Rust `#![no_std]` (`rust/`)
4. Zig freestanding (`zig/`)
5. D `-betterC` (`d/`)
6. Forth core (`forth/`)
7. Assembly (`asm/`) with ARMv7/AArch64/x86-64 kernels

The implementations share one small control contract: capability profile, fixed-point control values, table-driven state machine, watchdog, fail-safe/failover/rollback/failback and a 16-leaf Merkle scheduling primitive.

## Critical invariants

- `Q32.32` / `Q16.16` are **control-plane fixed-point formats**, not replacements for cryptographic word sizes.
- SIMD increases parallel independent work; it never shrinks a standard algorithm block. For 32-bit words: NEON-128 = 4 lanes, AVX2-256 = 8 lanes, AVX-512 = 16 lanes.
- Merkle reduction is binary: a 16-leaf batch reduces `16 -> 8 -> 4 -> 2 -> 1`. Hashing of each leaf/node must use the selected validated primitive.
- `TOKEN_VAZIO` means evidence/implementation is missing for a requested backend; it is not a cryptographic value.
- Reference scalar behavior is the oracle. Optimized backends must match it bit-for-bit before promotion to `VALIDATED`.
- No heap, malloc, GC or hidden runtime is permitted in the hot path of freestanding targets.

## State machine

`VOID -> PROBE -> BASELINE -> CANDIDATE -> VALIDATED`

Fault events can transition to `DEGRADED -> FAILOVER -> ROLLBACK -> FAILBACK -> VALIDATED`, or to `SAFE` if recovery is not proven. The transition table is explicit and bounded.

## SIMD/Merkle scheduling

For a 32-bit cryptographic word:

```text
scalar       1 lane
NEON 128     4 lanes
AVX2 256     8 lanes
AVX-512     16 lanes
```

This is scheduling parallelism, not a change from (for example) a 512-bit message block to 96 bits. BLAKE3, SHA-256 and other algorithms keep their specified block/chunk/state dimensions.

## Licensing and commercial boundary

Original files enumerated in `LICENSE_SCOPE_V1.json` are offered under **PolyForm Noncommercial License 1.0.0** through `LICENSE.md`. The scope is module-specific; it does not relicense unrelated RafPolimata material or any third-party content.

Research/noncommercial use follows the standardized license. Commercial/production rights outside that grant require a separate written commercial agreement. Third-party components retain their own licenses and must be recorded before distribution.

Contract/privacy/security templates are routed from `../../docs/legal/README.md`.

## Evidence boundary

Source presence = `IMPLEMENTED` only. The original change that introduced this directory did not execute compiler, ISA, device, benchmark or cryptographic vectors.

A later working-container experiment produced a **local** BLAKE3 known-answer-test PASS, documented at `evidence/BLAKE3_PORTABLE_LOCAL_KAT_2026-08-28.md`. The BLAKE3 implementation itself was not persisted to the repository because the available repository write path was blocked by a safety gate. Therefore current-commit BLAKE3 implementation/runtime evidence remains `TOKEN_VAZIO`.

## F_next

1. Compile/rerun the existing C/C++/ISA gates from the exact PR commit in an available build environment.
2. Persist the reviewed portable BLAKE3 oracle through an authorized repository path when permitted.
3. Re-run official known-answer vectors from that exact commit.
4. Add NEON 4-way, AVX2 8-way and AVX-512 16-way backends only after scalar equivalence.
5. Record cycles/byte, code size, symbol count, branches and rollback behavior separately.
