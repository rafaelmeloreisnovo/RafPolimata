# RAFAELIA CI V4.4 — Falsificação, evidência, incertezas (HOLD)

State: **LOCAL_PASS_SCOPED · REMOTE_SOURCE_TOKEN_VAZIO_NOT_PUBLISHED · PROVIDER_NOT_RUN · PROMOTION_HOLD**. This is a receipt of local experiments, **not** the V4.4 source.

## Baseline and byte identity
- V4.3 known HOLD baseline HEAD: `6d37dce4317545fc85ea55c3250b17650a0d05b4`. Existing main remains untouched; no workflow YAML changed.
- V4.4 **full new source and tests** are in the reviewed archive in official Drive: [RAFAELIA V4.4](https://drive.google.com/file/d/1VFZ-oprWihas9u6D57TTkYXEfDWurXU1/view).
- Archive SHA-256: `9aecd149a9e1564137c15cc4ee8b519039f8f9b3ab39f61c6401655dff7d428a`; 97,763 bytes, 79 files, ZIP CRC PASS, manifest SHA-256 PASS.
- New authorial source `raf_ci_gate_v44.h` SHA-256 `2b88c58a8c8a229c190537ecb5ccc838828f3e6feee19a34ce3b80ace7c5879a`; this source is **not committed** to GitHub yet. Do not confuse this audit document with source publication.

## Adversarial falsifiers (re-executed after final corrections)
1. Earlier V4.3 false REVIEW: empty `run: |`/`run: >`, duplicate `run`, unknown/empty event, non-string `run`, malformed quote and YAML alias: fail-closed HOLD in local V4.4.
2. Oracle phase 1: 2,000 mutations (seed 0x4400aa11), 1,784 independent YAML syntax errors, final **0** false REVIEW in that corpus; initial V4.4 prototype had **917**, which were fixed before this receipt.
3. Oracle phase 2: 12,000 different mutations (seed 0x857c4a8d), 10,581 independent YAML syntax errors, final **0** false REVIEW in that corpus; discovered unsafe `runs-on` values and unexpected nested keys.
4. Directed mutations: 30,000 (seed 0x8f99d4231ab3), **0** failed assertions, 24,376 former V4.3 REVIEW -> V4.4 HOLD. This count is specific to the selected corpus, **not** an industry false-positive rate.
5. Random hostile bytes with ASan+UBSan: 30,000 deterministic cases (seed 0xc0a17b2d), **0** monotonicity violations in tested cases.
6. GCC C11, G++ C++17 -Werror local tests PASS. Six Clang cross-ISA *object builds* (x86_64/i686/armv7/aarch64/riscv32/riscv64) have zero undefined symbols by `nm -u`.
7. Producer V4.4 -> RafGitTools independent 32-byte RCG4 consumer bridge PASS: it returns SOURCE_UNVERIFIED (never provider PASS); bit-corrupted receipt rejected.

## What did not close
- **P0 TOKEN_VAZIO_NOT_RUN** GitHub Actions provider execution at exact V4.4 SHA: no remote source commit or provider job executed, no claim of green CI.
- **P1 TOKEN_VAZIO_UNVERIFIED** complete YAML parsing/expression semantics, GitHub security controls, cryptographic attestation, chain-of-trust, independent safety review and commercial rights.
- **P1 TOKEN_VAZIO_COMPATIBILITY** RafGitTools HOLD integration branch diverged from main (last observed ahead 5 / behind 29); never force-merge or automatically rebase.
- **P1 TOKEN_VAZIO_NOT_RUN** physical ARM/Android/Termux and ART/DEX execution.
- **P2 TOKEN_VAZIO_UNMEASURED** latency, energy, real branchless disassembly and business value.
- `∅` != `NULL` != `NUL` != `0` != `""` != `TOKEN_VAZIO`. No translation/origin inference from Ω00..Ω15.
- PyYAML is a **host-only optional syntax falsifier**. L0 has no Python/libc/malloc/syscall/GC dependency. A parser's syntax success is not GitHub Actions approval.

## Closure and rollback
- V4.4 is **optional additive** over V4.3; existing 40/24/32 byte receipts and 24-byte G4 structure unchanged.
- Rollback: discard new V4.4 overlay / source bundle; never rewrite main or merge without separate permission and independent execution receipts.
- Next verifiable steps: exact-source Git commit and SHA readback; official GitHub run on exact commit; device-specific ARM receipts; official Actions semantic differential and supply-chain/legal review.
- claim_allowed=false; PROMOTION=HOLD; versioning follows only evidence-backed deltas.
