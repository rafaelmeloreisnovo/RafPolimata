# ZIPRAF Hardware Capability Fabric V1 — normative specification

## Scope

This module is a three-language control plane: C ABI, Rust `no_std`, and small architecture-specific assembly diagnostics. It selects an execution profile; it does not replace the canonical ZIPRAF codec and does not invent new cryptographic mathematics.

## Capability geometry

For a primitive with word width `w` and validated vector width `v`:

```text
lanes_raw = floor(v / w)
lanes = max(1, min(lanes_raw, max_parallel))
```

Parallelism is allowed only where independent work is valid. Register width alone is never a benchmark claim.

## Deterministic normalized cost

For input length `n`, block/chunk `b`, and lanes `L`:

```text
blocks = max(1, ceil(n / b))
C_q16 = ceil(blocks * 65536 / L)
```

V1 deliberately uses neutral coefficients so the score represents workload geometry only. Calibrated coefficients require physical measurement receipts tied to architecture, compiler and artifact.

## Promotion gate

A typed optimized backend may become `VALIDATED` only after standard/reference vectors, scalar-oracle equivalence, optimized bit-equivalence, architecture/compiler identity, explicit constant-time/side-channel scope where relevant, and a correctness receipt separate from a benchmark receipt.

## ASM boundary

The V1 assembly files are tiny integer diagnostics proving assembler/ABI routes. They are not cryptographic implementations.

## ZIPRAF boundary

Serialization, dictionary semantics, entropy limits, decompression and reconstruction remain governed by ZIPRAF_CORE / ZIPRAF_OMEGA_FULL. Any binary-format change belongs there and requires versioned compatibility tests.
