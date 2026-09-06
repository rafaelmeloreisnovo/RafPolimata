# PBIP-L1 — Evidence route

- `FEDERATION_ID`: `PBIP-L1-FED-V1`
- `FORMULA_ID`: `PBIP-L1`
- `role`: `EVIDENCE_ORCHESTRATION`
- `state`: `FIRST_EXECUTED_RECEIPT_OBSERVED / ANDROID_RUNTIME_PENDING / REPRODUCTION_PENDING`
- `claim_allowed`: `false`
- `date`: `2026-09-06`
- `governance_closure`: `CLOSURE_L1`

Governance anchor: `CLOSURE_L1`. Remaining missing runtime/device/reproduction/stdout-hash evidence stays explicit and cannot be promoted by prose.

## Formal identity under test

```math
q^2=r^2-d_\perp^2,
\qquad
\Delta_B=4(r^2-d_\perp^2)=4q^2
```

## First executed provider receipt

Receipt path:

`evidence/pbip/PBIP_L1_CI_RECEIPT_20260906_RUN34025698586.v1.json`

Observed provider binding:

```yaml
federation_id: PBIP-L1-FED-V1
formula_id: PBIP-L1
implementation_repo: rafaelmeloreisnovo/Vectras-VM-Android
implementation_commit: 836569a69c69b9b5d194108d0079a4a593b9f5d4
implementation_path: app/src/main/java/com/vectras/vm/core/PbipL1.java
implementation_sha256: 1119f44f78f6fe3e2abc3540414522106a92b3570b55cd20808ba147385055a9
test_path: app/src/test/java/com/vectras/vm/core/PbipL1Test.java
test_sha256: 2eb4f2d2ceeb587bcdfc19aec45ddfb220397bfdad5d5bb60864415bf2c8a6af
provider: github-actions
workflow: PBIP-L1 Evidence
workflow_run_id: 34025698586
workflow_job_id: 101466152645
workflow_conclusion: success
numeric_tolerance: 1.0e-12
receipt_sha256: 2077b4674e263338da12d8d78a312744e01d2f3ba44561cfb61cb848aa551c6b
artifact_id: 9987007149
artifact_name: pbip-l1-ci-receipt
artifact_zip_sha256: 7c569d05c58bbbfd240d1263b7c40546a57ed58367d5facbeb7409469ef5130b
stdout_sha256: TOKEN_VAZIO
source_observed: true
implemented_pbip_consumer: true
pbip_unit_build_proven: true
unit_test_execution_proven: true
android_runtime_proven: false
device_proven: false
reproduced_independently: false
claim_allowed: false
```

`TOKEN_VAZIO` above is governed by `CLOSURE_L1`; it is not zero, false, or failure.

## Canonical cases executed by the dedicated gate

1. `r=5, d_perp=3` -> `q^2=16`, `Delta_B=64`, `SECANT`;
2. `r=5, d_perp=5` -> `q^2=0`, `Delta_B=0`, `TANGENT`;
3. `r=5, d_perp=6` -> `q^2=-11`, `Delta_B=-44`, `NO_REAL_INTERSECTION`.

The dedicated workflow runs `:app:testDebugUnitTest --tests com.vectras.vm.core.PbipL1Test`. The successful provider log reported `BUILD SUCCESSFUL`, and only after that did the workflow emit and upload the receipt artifact.

## Failed-attempt custody

The first dedicated attempt failed before PBIP test execution because the runner lacked Android CMake `3.22.1`. Its receipt and artifact steps were skipped. The workflow was then corrected to provision the canonical CMake component, after which run `34025698586` passed. The failed attempt is preserved as evidence of fail-closed behavior, not rewritten as success.

## Finite route

```text
SOURCE_OBSERVED          = true
FORMULA_BOUND            = true
IMPLEMENTED              = true
PBIP_UNIT_BUILD_PROVEN   = true
UNIT_TEST_EXECUTION      = true
ANDROID_RUNTIME_PROVEN   = false
DEVICE_PROVEN            = false
REPRODUCED_INDEPENDENTLY = false
CLAIM_ALLOWED            = false
```

`PBIP_UNIT_BUILD_PROVEN` is narrower than an Android runtime/device proof. No state after unit execution is inferred automatically.

## Cross-repository route

```text
Matem-tica-#23 -> formal authority
ChipQuantum#68 -> geometry reference
Vectras-VM-Android#1123 -> executed PBIP consumer + provider receipt
Mapa#529 -> federated pins/state
Rafaelia_Private#211 -> low-level consumer reference, implementation still pending
RafPolimata#329 -> custody/evidence route
papers#73 -> academic/claim ledger
RLL#832 -> typed cross-domain reference only; no physical implication
```

## R3

- `F_ok`: first executed PBIP-L1 provider receipt is observed, hashed, artifact-backed and custody-bound.
- `F_gap`: Android runtime, physical device, independent reproduction and `stdout_sha256` remain open under `CLOSURE_L1`; physical-vortex and cubic/POLY3 claims are also not closed by this receipt.
- `F_next`: reproduce the same vectors through an independent implementation/provider before any broader claim promotion, then separately bind Android/device runtime if such a claim is required.
