# ApkC / RAFAELIA — memfd runtime capability probe — 2026-08-14

State: `HEAD_CONTENT_EXACT_LOCAL_PASS / PROVIDER_MATERIALIZED`

Claim gate: `claim_allowed=false`

Mode: `APPEND_ONLY / FAIL_CLOSED / CAPABILITY_BOUNDARY / CUSTODY_PRESERVING`

## Purpose

Reduce the deployment uncertainty between the already-landed sealed-memfd handoff and the still-unproved Android/Termux execution boundary.

This probe does not install an APK. It tests only the runtime capabilities that the sealed handoff requires before a physical installer probe is meaningful:

1. `memfd_create(..., MFD_ALLOW_SEALING)` succeeds;
2. the complete seal set `F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL` can be applied and read back as `0xf`;
3. a write after sealing is rejected with `EPERM`;
4. the descriptor survives `fork + exec` and a child process can consume the same object through `/proc/self/fd/N`;
5. Android compatibility remains explicitly `TOKEN_VAZIO`.

## Provenance

Base main: `0dbb14afd7511783030a9e8628eb1660feaf9826` (merge of sealed-memfd PR #230).

Branch: `hardening/apkc-memfd-runtime-probe-20260814`.

Materialized files:

- `scripts/apkc_probe_memfd_runtime.c`
- `tests/test_apkc_probe_memfd_runtime.sh`

The probe is additive and does not modify the landed sealed installer handoff.

## Exact local execution

The authored branch content was reproduced in repository layout and executed with `-std=c11 -O2 -Wall -Wextra -Werror`.

Observed transcript:

```text
memfd_create=PASS
readback_seals=PASS
seals=0xf
sealed_write_rejected=PASS
proc_self_fd_exec_inheritance=PASS
runtime_probe=PASS
android_compatibility=TOKEN_VAZIO
claim_allowed=false
RESULT pass=8 fail=0 claim_allowed=false
```

SHA-256:

- probe source: `93e2efbc6532e7180348deed12ebac0c7e831f4636d8cf0bac7c64696c105daa`
- test script: `4ad9b6bc0d59bcb5619469072dadf28a1f762c24ac6f36761621ab22ba957934`
- execution transcript: `823b9e69b35e7cff54f729e26f69e033d7d853b48e18699826e33c03080020cf`

Classification: `HEAD_CONTENT_EXACT_LOCAL_PASS` for this newly authored probe family. This is not a physical Android result and not a provider CI result.

## TOKEN_VAZIO delta

Resolved only for the reference runtime capability layer:

- `TOKEN_VAZIO_REFERENCE_RUNTIME_MEMFD_CREATE_AND_SEALS` -> `PASS`
- `TOKEN_VAZIO_REFERENCE_RUNTIME_PROC_SELF_FD_EXEC_INHERITANCE` -> `PASS`

Still open:

- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS`
- `TOKEN_VAZIO_ADB_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_PM_PROC_SELF_FD_COMPATIBILITY`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`
- `TOKEN_VAZIO_PROVIDER_CI_EXECUTION`

## F_ok

- sealed-memfd implementation already present in main;
- deployable runtime probe materialized without changing the installer path;
- exact local branch-content test: 8/8 PASS;
- full seal readback `0xf` observed;
- post-seal write rejection observed;
- `/proc/self/fd/N` fork+exec inheritance observed;
- `claim_allowed=false` preserved.

## F_gap

The reference Linux result cannot be promoted to Android. Termux/Android must execute these exact probe bytes. `adb install` and `pm install` must then be tested separately because a generic child process accepting `/proc/self/fd/N` does not imply Android package-manager compatibility.

## F_next

1. execute `tests/test_apkc_probe_memfd_runtime.sh` byte-identically in Termux ARM32 and preserve transcript, compiler version, kernel version and SHA-256;
2. only if that passes, run the sealed handoff with `pm install @APK_FD@` and preserve its exact exit/status output;
3. test `adb install @APK_FD@` separately from a context where the inherited FD semantics are actually available;
4. preserve every negative result; do not fall back silently to pathname transport;
5. physical install/launch/runtime gates remain closed until receipts exist.
