# ApkC Physical Measurement Environment Provenance — 2026-08-14

State: `MATERIALIZED_PENDING_PROVIDER_EXECUTION / claim_allowed=false`

## Invariant

Comparable physical evidence requires more than the same APK and measurement-script bytes. The environment/toolchain used to perform the measurement must be observable and sealed without inventing unavailable values or blessing arbitrary versions.

`Artifact + GateCode + MeasurementEnvironment + EvidenceTree -> comparable provenance candidate`

This does **not** imply runtime equivalence or Android proof.

## Materialized delta

- `scripts/apkc_provider_to_physical_envbound.sh`
- `tests/test_apkc_physical_env_provenance.sh`
- `.github/workflows/apkc-physical-env-provenance.yml`

The wrapper records machine/uname, resolved tool paths, SHA-256 for required measurement binaries, first-line version observations, repository commit/dirty state, and `TOKEN_VAZIO` for adb when adb is not required in probe-only mode. Required compiler and required adb (for adb-shell-pm) fail closed.

Eight falsifiers are wired: positive provenance binding; tool-manifest tamper; nested-evidence tamper; missing compiler; adb optional in probe-only; adb required in adb-shell-pm; negative child exit/evidence preservation; claim gate.

## F_ok

PR #251 code-bound custody is landed in main and provider-verified. The environment-provenance layer is now materialized additively and does not modify lower-level physical gates.

## F_gap

- `TOKEN_VAZIO_PR253_PROVIDER_EXECUTION`
- `TOKEN_VAZIO_PHYSICAL_ENV_PROVENANCE_CURRENT_DEVICE`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_next

Observe the exact PR head on a real provider runner. Promote only the environment-provenance implementation if syntax + 8/8 falsifiers pass. Physical gaps remain TOKEN_VAZIO until the same wrapper is executed on-device.

`claim_allowed=false`
