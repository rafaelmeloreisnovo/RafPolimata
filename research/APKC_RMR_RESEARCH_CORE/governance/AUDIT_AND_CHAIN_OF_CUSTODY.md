# Audit and Chain-of-Custody Contract

**Coupling ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`  
**Schema:** `raf.apkc-rmr-custody.v1`

## Authority split

```text
ApkC      → source-to-binary and APK/DEX/ELF/ABI structural evidence
RMR       → operational snapshots, capsules, state and custody metadata
BLAKE3    → optional public cryptographic artifact seal
SHA-256   → mandatory broadly available seal in this nucleus
Git       → authorship history, parentage and review path
License   → permitted field of use and obligations
```

No layer may silently promote another layer’s claim.

## Evidence record

Each release or commercial candidate must record:

- coupling ID;
- repository and immutable commit;
- parent commits and reviewed pull request;
- source/blob hashes;
- compiler, linker, SDK/NDK and flags;
- dependency and Action digests;
- operating system, ABI and device class;
- deterministic test vectors and outcomes;
- artifact hashes and signatures;
- runtime status separately from structural status;
- license and third-party inventory;
- reviewer identities/roles;
- timestamps and clock source;
- contradictions, blockers and `TOKEN_VAZIO`;
- incident and rollback receipts.

## State model

```yaml
PRESENT: bytes exist
SEALED: exact bytes match the lock
TESTED_STRUCTURAL: declared structural gates passed
TESTED_DEVICE: execution observed on identified device class
REPRODUCED: independent repeat matched
COMMERCIAL_AUTHORIZED: separate signed agreement exists
TOKEN_VAZIO: evidence absent or insufficient
FAIL: falsifier triggered
```

## Bit-coupling rule

The verifier must fail closed when any sealed member changes. Deliberate updates require a new lock and coupling generation; old locks remain immutable evidence.

## Audit rights model

Commercial audit rights may include:

- document and repository inspection;
- build reproduction;
- binary/hash comparison;
- deployment sampling;
- license-meter and usage review;
- subcontractor and supply-chain review;
- incident-triggered forensic preservation;
- corrective-action plan and re-audit.

Audits must protect legitimate confidential information and personal data and must not authorize access beyond the signed agreement or applicable law.
