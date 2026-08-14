# ApkC — Sealed stdin transport receipt — 2026-08-14

State: `VERIFIED_LIMITED_LOCAL / PROVIDER_MATERIALIZED`

Claim gate: `claim_allowed=false`

Mode: `APPEND_ONLY / SEALED_MEMFD / STDIN_TRANSPORT / FAIL_CLOSED / NO_PATHNAME_FALLBACK`

## Purpose

Reduce dependence on unverified Package Manager access to `/proc/<pid>/fd/N` while preserving the already-materialized sealed memfd authority over APK bytes.

AOSP `PackageManagerShellCommand` documents stdin installation explicitly: `install ... [PATH|-]`, with `-S BYTES` required when package bytes are read from stdin. `install-write` likewise accepts `PATH|-` and requires `-S` for stdin.

Primary provenance:

- AOSP main: `platform/frameworks/base/services/core/java/com/android/server/pm/PackageManagerShellCommand.java`
- AOSP Android 13 reference: same PackageManagerShellCommand contract.

This source-level contract establishes that Package Manager shell supports stdin as an input mode. It does **not** prove that the user's current Android/Termux/adb environment accepts the full end-to-end transport.

## Materialized delta

Stack base: `hardening/apkc-combined-install-receipt-20260814` at `abdadb41b5d69f597449a6b0c1c7ec07eb6d2a0a`.

Branch: `hardening/apkc-sealed-stdin-transport-20260814`.

Added without replacing the existing FD path:

- `scripts/apkc_install_sealed_stdin.c`
  - opens source once;
  - copies to `memfd_create(MFD_ALLOW_SEALING)`;
  - requires `F_SEAL_WRITE|F_SEAL_GROW|F_SEAL_SHRINK|F_SEAL_SEAL`;
  - hashes the sealed snapshot and requires equality to the externally supplied frozen SHA-256;
  - rewinds the sealed memfd;
  - duplicates it onto stdin of the installer child;
  - replaces exactly one `@APK_SIZE@` and one `@APK_STDIN@` placeholder;
  - writes `sealed-stdin.status` and `installer-exit.status`;
  - never sets `claim_allowed=true`.

- `scripts/apkc_finalize_stdin_install_receipt.sh`
  - requires transport `SEALED_MEMFD_STDIN`;
  - requires READY snapshot, exact digest equality, complete seals, positive `apk_bytes`, and installer exit 0;
  - missing, malformed, contradictory, or non-zero evidence rejects closed.

- `scripts/apkc_install_sealed_stdin.sh`
  - default explicit transport: `adb-shell-pm` -> `adb shell pm install -S @APK_SIZE@ @APK_STDIN@`;
  - `pm-local` exists only as explicit opt-in;
  - unknown mode rejects;
  - there is no silent pathname fallback and no automatic fallback to `/proc/.../fd`.

## Local execution evidence

### Sealed stdin helper

Controlled equivalent execution:

- positive exact stdin bytes/size: PASS
- same-inode source mutation after READY isolated from installer input: PASS
- digest mismatch blocks before installer: PASS
- missing stdin placeholder rejected: PASS
- installer exit 23 propagated and receipted: PASS
- claim gate preserved: PASS

Result: `6/6 PASS`, exit 0.

Hashes:

- helper source SHA-256: `0d3645766a1138840240d1e5a3d704242fe2861275d31912f820caedbee2f046`
- helper test SHA-256: `0513cc1f990b982685c0e3e6847e9a6895ed2fc844fe0332cc78c9eab9ebb88d`
- helper transcript SHA-256: `59894d3eaf07066418ebe6f301d600c26147969bcc2333dd95e042fcbf8e9531`

### Stdin finalizer

Adversarial execution:

- positive: PASS
- wrong transport: rejected
- zero size: rejected
- installer nonzero: rejected/propagated
- incomplete seals: rejected
- missing installer status: rejected
- claim gate preserved

Result: `7/7 PASS`, exit 0.

Hashes:

- finalizer SHA-256: `82dc9b9fc36e7dba937ec46e97b83cf0521335ae18e4022f17b28e121310f2a1`
- finalizer test SHA-256: `603057252e71365b2eab08577fad01ea6e2b552f8a616a34081f3bb2e9447864`
- transcript SHA-256: `e318ffd387eff08780ae8f31a6df077168970d093eaca7d229baf3e5b7eb6797`

### Explicit adb-shell-pm launcher

Mocked transport execution preserved the exact APK bytes and Package Manager stdin argument shape. Additional controls:

- `adb shell pm install -S N -` argument/byte path: PASS
- missing adb: fail-closed
- pathname fallback mode: rejected
- claim gate preserved

Result: `4/4 PASS`, exit 0.

Hashes:

- launcher SHA-256: `3ed166dc332a62a7111cc14a0536681f72fa56aa0ae10096159f2cbc262eb105`
- launcher test SHA-256: `b20979cbdd60f3d17129ff055bfc4c0f17e73172f6efb9ecef1016b3a1593ecc`
- transcript SHA-256: `44ad108d1a282678d8e80bed76901634716ce36f65079ec84645d225104fb264`

## Evidence boundary

The local tests prove the content logic in the available runtime and the provider materializes the exact source family. They do not prove current-device Android installation.

Not promoted:

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PM_LOCAL_STDIN_PERMISSION_COMPATIBILITY`
- `TOKEN_VAZIO_ADB_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_PM_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

The older `/proc/.../fd` compatibility gaps remain historically valid; stdin is an additive route around them, not evidence that those paths work.

## F_ok

AOSP-supported stdin input semantics were bound to the sealed memfd snapshot design; helper, explicit launcher, fail-closed finalizer, and three local batteries are materialized; no pathname fallback was introduced.

## F_gap

Current Android/Termux/adb physical compatibility remains unobserved. Provider CI execution is also not promoted without executed workflow steps.

## F_next

On the physical Termux/Android runtime, execute the exact branch bytes in this order:

1. run the landed memfd capability probe;
2. connect/verify adb transport explicitly;
3. invoke `scripts/apkc_install_sealed_stdin.sh APK EXPECTED_SHA OUT adb-shell-pm`;
4. preserve `sealed-stdin.status`, `installer-exit.status`, `apkc-stdin-install-chain.status`, stdout/stderr, APK digest, device/build identifiers, and exit code;
5. if the Package Manager rejects stdin or permissions, preserve the negative result and do not fall back silently;
6. only after an actual install PASS proceed to launch/runtime semantic gates.
