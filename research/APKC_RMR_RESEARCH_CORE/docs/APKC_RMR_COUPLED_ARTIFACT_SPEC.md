# Coupled Artifact Specification

**Coupling ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`  
**Schema:** `raf.apkc-rmr-coupled-artifact.v1`

## Invariant

The unit is not “code plus optional prose.” It is a coupled research object:

\[
U = C \oplus M \oplus D \oplus L \oplus P \oplus H
\]

where:

- `C` = source code;
- `M` = source comments and mandatory notices;
- `D` = technical specification;
- `L` = license references;
- `P` = commercial/high-risk policies;
- `H` = cryptographic coupling lock.

A bit change in any sealed member changes its digest and invalidates the unit:

\[
\Delta bit_i \Rightarrow SHA256_i' \ne SHA256_i \Rightarrow state=FAIL
\]

This is an integrity claim, not a claim that every textual sentence is executable code.

## Required states

```yaml
source_status: SEALED_BY_SHA256
documentation_status: SEALED_BY_SHA256
license_status: SEALED_BY_SHA256
commercial_authorization: TOKEN_VAZIO_UNLESS_SEPARATE_AGREEMENT
high_risk_authorization: FORBIDDEN_BY_RESEARCH_GRANT
claim_allowed: false
```

## Change protocol

1. open an isolated branch;
2. modify all semantically affected artifacts;
3. record the reason and falsifier;
4. run tests;
5. regenerate the coupling lock;
6. review third-party boundaries;
7. review licensing impact;
8. publish a new coupling generation;
9. never rewrite the prior receipt.

## ApkC relationship

The core consumes only evidence pointers from ApkC:

```text
source commit
source-to-binary receipt
APK structural receipt
ZIP/DEX/ELF/ABI status
runtime boundary
```

It must not convert `PASS_STRUCTURAL` into signing, installation, launch or production authorization.

## RMR/BLAKE3 relationship

- RMR owns operational snapshots, evidence capsules and custody metadata.
- BLAKE3 may seal final artifacts as the public cryptographic integrity primitive.
- SHA-256 is mandatory in this nucleus for broad tool availability.
- BLAKE3 is a second seal and remains `TOKEN_VAZIO` when no verified implementation is available.
- A hash proves byte identity, not scientific truth, legal compliance or safety.

## Falsifiers

- missing file;
- changed SHA-256;
- absent or divergent coupling ID;
- removed required notice;
- commercial permission inferred from mere communication;
- third-party code presented as relicensed;
- research receipt promoted to production approval;
- missing runtime boundary.
