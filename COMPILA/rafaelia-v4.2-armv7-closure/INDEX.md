# RAFAELIA V4.2 ARMv7 Closure — Index

**Parent:** `../rafaelia-v4.1-hardened/`  
**Source anchor:** `RAFAELIA_COMPLETE_v4.zip`  
**Source SHA-256:** `d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454`  
**V4.1→V4.2 patch SHA-256:** `50aa730140f0577f2672c205f3294ce6acf02b07302cec57c8dc847e071a2a56`

## Purpose

This append-only layer closes the architecture-independent ARMv7 assembler/linker gap
without rewriting V4.1. It then moves the frontier to the physical Android/Termux gate.

## Contents

- `v4.1-to-v4.2.patch.gz.b64` — deterministic gzip of the V4.1→V4.2 delta, base64-wrapped for Git transport; decompressed SHA is gated.
- `.bootstrap` — append-only staging marker from the remote write sequence.
- `MATERIALIZE_AND_VERIFY.sh` — source→V4.1→V4.2 reconstruction + verification.
- `RUN_OMEGA_ON_DEVICE.sh` — one-command last-mile physical gate.
- `EVIDENCE_MANIFEST.json` — machine-readable claim boundary.
- `TEST_RECEIPT.md` — reference-environment evidence.
- `OMEGA_LADDER.md` — exact remaining transitions.
- `SHA256SUMS` — content hashes.

`CLAIM_ALLOWED=false` until the physical gates are evidenced.
