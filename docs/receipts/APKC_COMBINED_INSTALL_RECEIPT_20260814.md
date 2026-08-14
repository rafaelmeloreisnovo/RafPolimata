# ApkC / RAFAELIA — Combined install receipt hardening — 2026-08-14

State: `LOCAL_CONTENT_PASS / PROVIDER_MATERIALIZED`

Claim gate: `claim_allowed=false`

Mode: `APPEND_ONLY / FAIL_CLOSED / CUSTODY_PRESERVING / STATUS_SEMANTICS_HARDENED`

## Purpose

Prevent a valid sealed snapshot from being mistaken for a successful APK installation.

Before this delta, `sealed-memfd.status` could remain `sealed_snapshot_status=READY` even when the installer returned non-zero; installer failure was recorded separately in `installer-exit.status`. The cryptographic handoff remained protected, but a consumer reading only the snapshot status could over-promote the operational result.

This delta does not alter sealed-memfd bytes, snapshot construction, digest comparison, FD transport, the existing hardened capture path, or Android package-manager behavior.

## Materialized artifacts

Base main: `32d3ea132134e94b87827007d3cf2a9da289a15d`.

Branch: `hardening/apkc-combined-install-receipt-20260814`.

- `scripts/apkc_finalize_install_receipt.sh`
- `scripts/capture_android_proof_chain_sealed_receipted.sh`
- `tests/test_apkc_finalize_install_receipt.sh`
- `tests/test_apkc_receipted_bridge.sh`

The new recommended entrypoint is additive. The pre-existing sealed hardened entrypoint remains unchanged for compatibility.

## Combined evidence contract

`apkc-install-chain.status` is authoritative only for the conjunction:

`sealed snapshot READY` AND `expected_sha256 == sealed_snapshot_sha256` AND `required seals include 0xf` AND `installer_exit == 0`.

Any missing, duplicate, malformed, contradictory, or non-zero state fails closed and preserves `claim_allowed=false`.

The finalizer writes atomically through a temporary file followed by rename.

## Local exact-content behavioral evidence

Finalizer adversarial battery:

```text
PASS positive rc=0
PASS installer_nonzero rc=23
PASS missing_installer_status rc=153
PASS duplicate_installer_exit rc=155
PASS digest_mismatch rc=156
PASS snapshot_not_ready rc=157
PASS incomplete_seals rc=157
PASS malformed_installer_exit rc=155
RESULT pass=8 fail=0 claim_allowed=false
```

SHA-256:

- finalizer: `a6063657f090db193b0326e2da6ce429d4104a6f4a666716c9b1e890c37ddeca`
- finalizer test: `9d80163f49387000681c0f096f6c24b649bdca325adf8fb0f381739c6226aa1f`
- finalizer transcript: `59c658c3bc63c37e60ac56ef1f77d152e76fa2b0e61527a7ffa1d3f844e43ad8`

Receipted bridge battery:

```text
PASS positive rc=0
PASS installer_failure rc=23
PASS missing_evidence rc=153
RESULT pass=3 fail=0 claim_allowed=false
```

SHA-256:

- receipted bridge: `78a3a980b77308c197af3bcfa297713a742f025cd69c0843261929ee37d2ccd4`
- bridge test: `7779b6cf673a35f434eedc4370f46d1c817a8065cab67e9b82e2cf051df96fcb`
- bridge transcript: `6dd8074ecbfe13007825464656170793303621bea953a70aa36aac187dff78f3`

Classification is limited to the authored content executed in a reference local environment. No Android physical execution is claimed.

## TOKEN_VAZIO delta

Resolved:

- `TOKEN_VAZIO_COMBINED_SNAPSHOT_INSTALL_RESULT_CONTRACT` -> `LOCAL_FAIL_CLOSED_CONTRACT_MATERIALIZED`
- `TOKEN_VAZIO_INSTALLER_NONZERO_STATUS_AMBIGUITY` -> `REJECTED_BY_COMBINED_RECEIPT`

Still open:

- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_PM_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_ADB_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`
- `TOKEN_VAZIO_PROVIDER_CI_EXECUTION`

## F_ok

- semantic ambiguity between snapshot readiness and installer success removed in the new path;
- finalizer rejects incomplete/contradictory evidence;
- installer non-zero propagates as non-zero;
- 8/8 finalizer adversarial cases PASS;
- 3/3 integrated bridge cases PASS;
- old hardened entrypoint preserved unchanged;
- `claim_allowed=false` invariant preserved.

## F_gap

The new combined receipt proves only local evidence semantics. It does not prove that Android can create/seal the memfd, that Termux ARM32 reproduces the probe, that `pm` or `adb` accept `/proc/self/fd/N`, or that a physical APK installs/launches correctly.

## F_next

1. execute the already-landed runtime capability probe byte-identically on Termux ARM32;
2. if and only if that passes, run the new receipted entrypoint with `pm install @APK_FD@` and preserve all status files plus exact stdout/stderr/exit;
3. test `adb install @APK_FD@` separately because its transport semantics are independent;
4. preserve negative outcomes without pathname fallback;
5. do not promote physical install, launch, semantic runtime or cross-device claims without corresponding receipts.
