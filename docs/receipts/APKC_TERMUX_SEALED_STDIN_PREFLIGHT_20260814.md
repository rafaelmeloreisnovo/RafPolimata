# ApkC/RAFAELIA — Termux/Android Sealed-stdin Preflight Receipt — 2026-08-14

State: `VERIFIED_LIMITED_LOCAL_CONTROLLED / PROVIDER_MATERIALIZED`

Claim gate: `claim_allowed=false`

## Purpose

Bridge the already-landed structural Termux ARM32 validator and the sealed-memfd→stdin installer path without modifying either historical gate. The new gate is compositional and fail-closed: exact APK digest and memfd runtime capability must pass before any installer attempt is allowed.

## Base authority

- repository: `rafaelmeloreisnovo/RafPolimata`
- base main: `c31168b0f37d93c26c7ca1fd8bc45340d755ad40`
- base state: PR #234 merged; sealed-stdin replay is on main
- existing ARM32 structural validator remains unchanged: `scripts/apkc_validate_termux_arm32.sh`
- existing sealed-stdin launcher remains unchanged: `scripts/apkc_install_sealed_stdin.sh`
- existing memfd capability probe remains unchanged: `scripts/apkc_probe_memfd_runtime.c`

## New provider-materialized artifacts

- `scripts/apkc_termux_sealed_stdin_gate.sh`
  - Git blob: `d8596fdb6d750d8ba003fbe51c74d9902ca6e2e5`
  - local SHA-256: `891d5a263a139aab428373b0ffc6eb39ba0b4772be0e9894d94e2d3ee14e09ed`
- `tests/test_apkc_termux_sealed_stdin_gate.sh`
  - Git blob: `7e1f4b40d1443dcd5dd1dfd15ac45435a05645d0`
  - local SHA-256: `6702ec40bef5eb7e75f8078f5506b431627aaddb2d50111db4bdbbcc377e1274`

## Gate sequence

`APK -> SHA256 exact match -> compile/run memfd capability probe -> require runtime_probe=PASS + claim_allowed=false -> mode-specific transport preflight -> sealed-stdin launcher -> deterministic receipt verification`

Modes:

- `probe-only`: never attempts installation; successful capability probe leaves installation explicitly `TOKEN_VAZIO_NOT_ATTEMPTED`.
- `adb-shell-pm`: requires `adb get-state == device`, records Android SDK/ABI/build fingerprint, stores only SHA-256 of device serial, then calls the already-landed sealed-stdin launcher.
- `pm-local`: calls the existing sealed-stdin launcher locally; physical Android authority remains dependent on measured environment evidence.

There is no fallback to pathname installation.

## Local controlled adversarial execution

Transcript:

```text
PASS probe_only rc=0
PASS digest_mismatch rc=215
PASS probe_claim_missing rc=221
PASS probe_runtime_fail rc=9
PASS unknown_mode rc=223
PASS adb_not_ready rc=227
PASS adb_install_mock_pass rc=0
PASS install_mock_fail rc=23
PASS receipt_and_serial_pseudonymization
RESULT pass=9 fail=0 claim_allowed=false
```

Transcript SHA-256: `a9165185b2277c9e856fb44f2f535f3c7e140071dde5e2ebd4b1f60a727f21c3`.

The adb and installer paths in this local battery are mocks. Therefore this execution proves control flow, fail-closed semantics, receipt verification and serial pseudonymization, not Android installation compatibility.

## Privacy/provenance boundary

Raw adb serial is never written to evidence files by the gate; only `SHA256(serial)` is recorded. Android build fingerprint is a build identity, not a user/device serial. Every status file carries or inherits `claim_allowed=false` semantics.

## TOKEN_VAZIO delta

Reduced:

- `TOKEN_VAZIO_PHYSICAL_INSTALL_PREFLIGHT_CHAIN` -> `MATERIALIZED_FAIL_CLOSED_PREFLIGHT_GATE`
- `TOKEN_VAZIO_DEVICE_IDENTITY_CUSTODY_FOR_INSTALL_ATTEMPT` -> `PSEUDONYMIZED_ADB_IDENTITY_CAPTURE_MATERIALIZED`

Still open:

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PM_LOCAL_STDIN_PERMISSION_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`
- `TOKEN_VAZIO_PROVIDER_CI_EXECUTION_THIS_HEAD`

## F_ok

A bounded post-build preflight gate now joins digest identity, memfd capability, transport readiness, pseudonymized device identity and receipt verification without altering the prior structural validator or sealed-stdin launcher. Local controlled battery: `9/9 PASS`.

## F_gap

No current Termux ARM32/Android physical execution is claimed in this receipt. Mock adb/installer success is not physical evidence. Installation, launch, runtime semantics, cross-device determinism and provider CI remain unproven.

## F_next

On the physical target, first run this gate in `probe-only` against the exact APK+SHA. Only if it passes, rerun the same APK+SHA in `adb-shell-pm`; preserve all output/status/receipt files regardless of outcome. A negative transport/install result is evidence and must not trigger pathname fallback.
