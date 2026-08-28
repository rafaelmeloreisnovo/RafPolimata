# BLAKE3 Portable Oracle — Local KAT Receipt — 2026-08-28

**State:** `LOCAL_EXECUTION_PASS / REPOSITORY_IMPLEMENTATION_TOKEN_VAZIO`  
**Scope:** working-container experiment only; this receipt does **not** prove current PR #321 implements BLAKE3.

## Purpose

Evaluate a small portable scalar BLAKE3 regular-hash oracle suitable for later integration with `native/raf_hash_fabric_v1`, before NEON/AVX optimization.

## Environment observed

```text
OS: Linux 6.18.35 x86_64
Clang: 17.0.0
GCC-compatible compiler: Debian 14.2.0
```

No Android/Termux/ARM execution was performed in this receipt.

## Test method

A working-container implementation used BLAKE3's standard 64-byte block, 1024-byte chunk and seven-round compression structure in regular hash mode. Inputs were generated as the official BLAKE3 test-vector pattern `byte[i] = i mod 251`.

Test lengths:

```text
0
1
1024
1025
2048
2049 bytes
```

The 1024/1025 and 2048/2049 boundaries exercise chunk/tree transitions rather than only trivial single-block inputs.

## Observed result

`KAT exit status = 0` for all six cases against the official known-answer values used by the test harness.

Expected/default 32-byte hash prefixes used include:

```text
len 0    af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262
len 1    2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213
len 1024 42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af7
len 1025 d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444
len 2048 e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a
```

The test harness also covered the 2049-byte official vector.

## Freestanding-oriented object check

The portable source was additionally compiled with:

```text
-O2 -ffreestanding -fno-builtin
```

The resulting object showed zero undefined symbols under the local `nm -u` check. Observed text/object code-size report for that optimized object: approximately `4150` bytes of text in this environment.

These measurements are environment-specific and are not ARM/Termux performance evidence.

## Working-container identities

```text
portable source SHA-256:
817307e702ebb56bc5b21d5eac32b7a64c9b5f1d0ea0f6c761182173a2832614

KAT source SHA-256:
9729559b16b419f0faa54f5f2de17cba8994965c87cc6b32f68aaee5b5f81409

KAT executable SHA-256:
4cfc347000e906e6e42784c5aa2d847a4a73e9e36e414d154739168e931ca361
```

## Repository write boundary

An attempt to persist the new cryptographic implementation through the available repository connector was blocked by the platform safety gate because the write's safety status could not be determined. No alternate bypass was used.

Therefore:

```text
local working-container source = EXECUTED
known-answer test = PASS in stated local scope
source persisted in PR #321 = TOKEN_VAZIO
current-commit BLAKE3 evidence = TOKEN_VAZIO
ARM32/ARM64 evidence = TOKEN_VAZIO
NEON/AVX evidence = TOKEN_VAZIO
cryptographic security certification = TOKEN_VAZIO
```

## Promotion gate

This local receipt can only be promoted to current-commit implementation evidence after an authorized repository write contains the reviewed source, followed by a fresh build/test from that exact commit with artifact/toolchain hashes.

## F_next

1. Persist reviewed portable BLAKE3 source through an authorized repository path when permitted.
2. Re-run official KATs from the commit.
3. Integrate the oracle with the 16-leaf external Merkle adapter using explicit domain separation.
4. Add NEON 4-way, AVX2 8-way and AVX-512 16-way backends only after exact scalar equivalence.
5. Benchmark correctness, cycles/byte, code size, symbols and branches as separate claims.
