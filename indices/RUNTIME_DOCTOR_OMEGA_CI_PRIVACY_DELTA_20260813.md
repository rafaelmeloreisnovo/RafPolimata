# Runtime Doctor Ω — CI + Privacy Delta — 2026-08-13

State: `VERIFIED_LIMITED_PROVIDER_CLASSIFICATION`
Claim allowed: `false`
Parent index: `indices/RUNTIME_DOCTOR_OMEGA_ROUTE_INDEX_V1.md`
PR: `#227`
Head: `0c0fc66115e06c54d6781528ecf23a40e4668d4d`

## Privacy default

The readiness receipt minimizes device-identifying data by default:

- raw device identifiers are not stored;
- raw device names are not stored;
- raw device-enumeration stdout is not stored;
- an absolute optional artifact path is not stored;
- bounded hashes/pseudonyms are correlation/provenance aids, not identity proof.

Implementation commit: `eb6697f090bdd11bc9c91f64910408f3d9eec507`
Test-extension commit: `0c0fc66115e06c54d6781528ecf23a40e4668d4d`

## Provider CI classification

For head `0c0fc66115e06c54d6781528ecf23a40e4668d4d`:

- CI run `31757117402`: conclusion `failure`, job `94635220864`, `runner_id=0`, `steps=[]`;
- Formal Science Orchestrator run `31757117361`: conclusion `failure`, job `94635220951`, `runner_id=0`, `steps=[]`.

Classification: `INFRASTRUCTURE_RUNNER_NOT_STARTED_NOT_CODE_REGRESSION`.

A provider workflow failure with no assigned runner and no executed steps is not evidence that repository tests failed. Repository tests therefore remain `TOKEN_VAZIO_NOT_EXECUTED_BY_PROVIDER`.

## Reconstruction route

`Master Navigation Registry -> Drive Context Index K_RUNTIME_DOCTOR_FRIDA_20260813_CI_ERRATA -> PR #227 -> this delta -> provider job records -> independent runtime receipt`

## F_ok

Provider failure cause is separated from code semantics; privacy minimization is materialized on the branch.

## F_gap

No independent executed-test receipt or physical-device runtime receipt is present in this delta.

## F_next

Execute branch tests in an available independent runtime, then execute the read-only readiness probe on the physical Termux environment and preserve both receipts before promotion.
