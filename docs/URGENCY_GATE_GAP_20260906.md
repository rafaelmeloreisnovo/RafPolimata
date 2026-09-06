# RafPolimata — Urgency / Gate / Gap Matrix — 2026-09-06

Status: AUDIT / APPEND-ONLY SNAPSHOT
Source revision: `820e7ea29bebbf0154ea245d401f691715fea5d8`
Claim policy: `claim_allowed=false` unless the specific gate below is closed by revision-bound evidence.

## Invariants

`SOURCE_OBSERVED != IMPLEMENTED != BUILD_PROVEN != RUNTIME_PROVEN != DEVICE_PROVEN != REPRODUCED`

`TOKEN_VAZIO != FAIL != PASS`

A gap can only leave `TOKEN_VAZIO` when an identified gate is executed against an exact revision/artifact and emits a traceable receipt.

## Priority scale

- `U0`: blocks trustworthy promotion, release, provider safety or principal runtime claim.
- `U1`: high-value implementation/evidence debt that materially limits the architecture.
- `U2`: maturity, reproducibility, scientific or governance debt.
- `U3`: optimization/expansion after higher gates are closed.

## Matrix

| ID | Urgency | Surface | Current state | Gap | Gate required | Closure evidence | F_next |
|---|---|---|---|---|---|---|---|
| RP-U0-01 | U0 | GitHub governance | `UNPROTECTED_OBSERVED` | `main` has no active branch protection in the observed provider response | provider readback must show intended protection/ruleset and required checks/review policy | provider JSON + revision + ruleset/protection receipt | define desired policy, dry-run, apply only with rollback/readback |
| RP-U0-02 | U0 | physical execution | `TOKEN_VAZIO` | current freestanding/ApkC/runtime artifacts lack same-artifact physical execution proof | execute exact artifact on declared target and capture stdout/stderr/device/ABI/hash | device receipt bound to commit + artifact SHA-256 | close one reference device first, then expand matrix |
| RP-U0-03 | U0 | PBIP-L1 | `IMPLEMENTATION_REPRODUCED` | independent CI provider and Android/device proof remain open | reproduce canonical vectors using independent provider/runtime and preserve receipts | independent provider receipt + Android/device receipt | keep `claim_allowed=false` until remaining dimensions close |
| RP-U0-04 | U0 | ApkC provenance | `TOKEN_VAZIO` for current-artifact chain | current APK is not yet proven to originate from the same exact commit/toolchain with complete DEX/ELF/signing evidence | source→binary→APK provenance gate on one exact revision | APK SHA-256, DEX SHA-1/Adler-32, ELF ABI inventory, signing and build receipt | produce a single canonical current-head artifact chain |
| RP-U1-01 | U1 | freestanding L0/L11 | `PARTIAL_ADVANCED` | AMX full tile path, SME/SME2 operational path/ZT0 and several ISA executors remain incomplete | compile/codegen + zero-helper + behavioral harness per target | object/codegen receipt and deterministic test vectors | implement one architecture family per isolated PR |
| RP-U1-02 | U1 | architecture coverage | `METADATA_ONLY/PENDING` | POWER VSX/MMA, LoongArch LSX/LASX, s390x vector; then Hexagon/HVX, MVE/Helium, APX | executor implementation followed by target-appropriate build/runtime gate | per-ISA receipt, never infer runtime from metadata compile | preserve OS-neutral L0 / syscall separation |
| RP-U1-03 | U1 | Conversation Indexer | `IMPLEMENTED_PARTIAL` | streaming extractor, atomic writer, checkpoint/resume and BLAKE3 remain pending | deterministic corpus round-trip and corruption/recovery tests | corpus fixture hashes + test receipt | close writer/recovery before performance expansion |
| RP-U1-04 | U1 | scientific engine | `REFERENCE/IMPLEMENTED_COMPONENTS` | external scientific reproduction/novelty is not established | exact-input reproduction + citations + independent review where claims require it | dataset/input hashes, scripts, outputs, bibliographic and review receipt | separate engineering validity from scientific novelty |
| RP-U2-01 | U2 | licensing | `TOKEN_VAZIO_REPO_LICENSE_METADATA` | repository metadata exposes no recognized license | authorial/license decision + compatibility audit for imported/reference material | LICENSE + provenance/attribution inventory | do not infer reuse rights from public visibility |
| RP-U2-02 | U2 | documentation governance | `ACTIVE` | loose/historical documents may still diverge from source truth | document-governance scan + duplicate/temporal/relation review queue | catalog/relations/review-queue hashes | reconcile without deleting historical evidence |
| RP-U2-03 | U2 | CI final-head truth | `DYNAMIC` | predecessor success must not be projected onto a moved head | run required workflows on exact PR/final head | workflow run/job IDs + artifact digests | bind every promotion to final-head CI |
| RP-U3-01 | U3 | performance | `TOKEN_VAZIO_BENCHMARK_MATRIX` | cross-ISA performance claims lack a controlled reproducible denominator | pinned compiler/flags/corpus/hardware benchmark protocol | raw measurements + environment + confidence/statistics | benchmark only after correctness gates |

## Closure order

`RP-U0-01 → RP-U0-04 → RP-U0-02 → RP-U0-03 → RP-U1-* → RP-U2-* → RP-U3-*`

This ordering is risk-weighted, not a statement that lower-priority items are unimportant.

## R3

- `F_ok`: evidence discipline, PBIP cross-implementation reproduction, freestanding source/codegen coverage, ApkC and governance tooling exist.
- `F_gap`: provider protection, exact-artifact physical runtime, complete ApkC provenance, independent provider/device PBIP, remaining ISA executors and independent scientific validation.
- `F_next`: close revision-bound U0 gates without replacing missing evidence with inference.
