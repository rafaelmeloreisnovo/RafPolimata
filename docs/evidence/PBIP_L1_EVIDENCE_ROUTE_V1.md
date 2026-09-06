# PBIP-L1 — Evidence route

- `FEDERATION_ID`: `PBIP-L1-FED-V1`
- `FORMULA_ID`: `PBIP-L1`
- `role`: `EVIDENCE_ORCHESTRATION`
- `state`: `CROSS_IMPLEMENTATION_REPRODUCTION_PROVEN / PROVIDER_INDEPENDENCE_PENDING / ANDROID_RUNTIME_PENDING / DEVICE_PENDING`
- `claim_allowed`: `false`
- `date`: `2026-09-06`
- `governance_closure`: `CLOSURE_L1`

## Formal identity under test

```math
q^2=r^2-d_\perp^2,
\qquad
\Delta_B=4(r^2-d_\perp^2)=4q^2
```

## Evidence ladder

```text
formal relation bound                  = true
Vectras Java implementation observed  = true
Vectras unit build/test receipt        = true
second implementation observed        = true
cross-implementation vectors equal    = true
language independence bounded         = true
toolchain-family independence bounded = true
CI provider independence               = false
Android runtime                        = false
physical device                        = false
claim_allowed                          = false
```

`TOKEN_VAZIO` is not zero, failure or permission to infer a result.

## Producer A — Vectras Java

Receipt:

`evidence/pbip/PBIP_L1_CI_RECEIPT_20260906_RUN34025698586.v1.json`

Observed binding:

```yaml
repository: rafaelmeloreisnovo/Vectras-VM-Android
implementation_commit: 836569a69c69b9b5d194108d0079a4a593b9f5d4
implementation_path: app/src/main/java/com/vectras/vm/core/PbipL1.java
implementation_sha256: 1119f44f78f6fe3e2abc3540414522106a92b3570b55cd20808ba147385055a9
language: Java
toolchain_family: Gradle/JDK21
workflow_run_id: 34025698586
workflow_job_id: 101466152645
receipt_sha256: 2077b4674e263338da12d8d78a312744e01d2f3ba44561cfb61cb848aa551c6b
artifact_id: 9987007149
artifact_zip_sha256: 7c569d05c58bbbfd240d1263b7c40546a57ed58367d5facbeb7409469ef5130b
```

The first dedicated attempt failed before PBIP tests because CMake 3.22.1 was unavailable. The successful later run provisioned the required component and emitted the receipt only after test/build success. The failed attempt remains custody evidence rather than being rewritten.

## Producer B — Rafaelia_Private C11 freestanding

Cross-implementation record:

`evidence/pbip/PBIP_L1_CROSS_IMPLEMENTATION_REPRODUCTION_20260906.v1.json`

Observed binding:

```yaml
repository: rafaelmeloreisnovo/Rafaelia_Private
implementation_commit: d9bada8895f03805f3187e48cdadb3d435d5f5da
implementation_path: src/pbip/pbip_l1_freestanding.c
implementation_sha256: 41f39d3f30b90237ae6f250976a3b5240aa822ba3f4a72052b4d31814ece791d
language: C11
toolchain: Ubuntu clang 18.1.3
freestanding: true
undefined_external_symbols: 0
object_sha256: 14b8b5e8580dce137af3af4674c62f0d21a93652752af5455d2204950cd02597
stdout_sha256: 804cc3e8b4b84dde2547eef530fe0bd7d672ce50e7e63bcffccb6aa86d94824f
workflow_run_id: 34026460668
workflow_job_id: 101468184710
receipt_sha256: e1b562b2ee719b93e2f9d5326c06943a5a052b873a259ce0b685a3fe18a00c2b
artifact_id: 9987206552
artifact_zip_sha256: 0240db4d06925daaee604811d24c87a80ec3dad62f5b70c30cb6f32e09c84e89
```

## Canonical vectors compared

1. `r=5, d_perp=3` → `q²=16`, `Delta_B=64`, `SECANT`;
2. `r=5, d_perp=5` → `q²=0`, `Delta_B=0`, `TANGENT`;
3. `r=5, d_perp=6` → `q²=-11`, `Delta_B=-44`, `NO_REAL_INTERSECTION`.

The committed comparison record states:

```text
canonical_vectors_equal                = true
classification_equal                   = true
repository_independent                 = true
implementation_independent             = true
language_independent                   = true
toolchain_independent                  = true
cross_implementation_reproduction      = true
ci_provider_independent                = false
device_independent                     = false
provider_independent_reproduction      = false
```

## What this closes

The evidence supports a bounded statement:

> The three canonical PBIP-L1 vectors/classifications were reproduced by two repository-separated implementations using different implementation languages and toolchain families, with hashed receipts/artifacts recorded in the evidence chain.

It does **not** by itself establish:

- independent CI provider reproduction;
- Android runtime;
- physical-device execution;
- universal mathematical proof beyond the tested formal identity/vectors;
- physical-vortex interpretation;
- broader POLY3/cubic claims;
- scientific novelty or peer review.

## Remaining gates

```text
TOKEN_VAZIO_PBIP_PROVIDER_INDEPENDENT_REPRODUCTION
TOKEN_VAZIO_PBIP_DEVICE_PROOF
TOKEN_VAZIO_PBIP_ANDROID_RUNTIME
claim_allowed=false
```

## Cross-repository route

```text
Matem-tica-#23         -> formal authority
ChipQuantum#68         -> geometry reference
Vectras-VM-Android#1123 -> executed Java consumer / provider receipt
Rafaelia_Private       -> second C11 freestanding implementation receipt
Mapa#529               -> federated pins/state
RafPolimata#329/#330   -> evidence custody + cross-implementation comparison
papers#73              -> academic/claim ledger
RLL#832                -> typed cross-domain reference only
```

## R3

- **F_ok:** first provider execution plus second C11 freestanding implementation now support the bounded cross-implementation reproduction state.
- **F_gap:** provider-independent reproduction, Android runtime and physical device remain explicitly open.
- **F_next:** reproduce the same canonical vectors through an independent provider/environment, then close Android/device only if those dimensions are claimed.