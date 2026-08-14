# ApkC / RAFAELIA — Sealed memfd installer handoff — 2026-08-13

State: `VERIFIED_LIMITED_LOCAL_EQUIVALENT / PROVIDER_MATERIALIZED`

Claim gate: `claim_allowed=false`

Mode: `APPEND_ONLY / FAIL_CLOSED / ADDITIVE_COMPATIBILITY / CUSTODY_PRESERVING`

## Purpose

Close the logical same-inode mutation gap left by the pathname/FD handoff without changing the existing FD wrapper in place.

The new path creates a private `memfd`, copies the APK bytes into it, applies Linux seals `F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL`, hashes the sealed snapshot, compares that digest with the already-frozen expected SHA-256, and only then invokes the installer with a single `/proc/self/fd/N` placeholder.

This changes the handoff object from the mutable source inode to an immutable sealed snapshot. A later in-place write to the original APK therefore cannot alter the bytes consumed through the sealed descriptor.

## Provenance

Base `main`: `7a6d1cd2b9c488ed7d3706aef0eb6716841be31c` (merge of PR #229).

Branch: `hardening/apkc-sealed-memfd-handoff-20260813`.

Provider-materialized files:

- `scripts/apkc_install_sealed_memfd.c`
  - Git blob: `d7d708d17d70f1a6679312ec4a9c6f350332753c`
- `scripts/apkc_install_sealed_memfd_gate.sh`
  - Git blob: `eeedac4977f38f229b82cd483d406a685ea19197`
- `scripts/capture_android_proof_chain_sealed_hardened.sh`
  - Git blob: `277ab1de8fb3d47c8481b7fd1df3c590e4fc3492`
- `tests/test_apkc_sealed_memfd_handoff.sh`
  - Git blob: `dac32e43180e6de2f11e74984a5faf2874268e07`

The delta is additive: the prior `scripts/apkc_install_fd_handoff.sh` and `scripts/capture_android_proof_chain_fd_hardened.sh` are not replaced. The sealed chain explicitly selects the stronger gate through `APKC_FD_HANDOFF_GATE`.

## Fail-closed invariants

1. expected SHA-256 must be lowercase 64-hex;
2. source must open as a non-empty regular file;
3. `memfd_create` must exist;
4. complete copy into the memfd must succeed;
5. all four required seals must be applied and read back;
6. SHA-256 of the sealed snapshot must equal the frozen expected digest;
7. exactly one `@APK_FD@` placeholder is required;
8. installer exit code is propagated;
9. every status keeps `claim_allowed=false`.

No fallback from the sealed gate to the weaker unsealed FD gate is performed inside the sealed path. A platform without `memfd_create`, seals, compiler, `/proc`, or `sha256sum` fails closed.

## Local executable evidence

A locally reconstructed equivalent of the landed source/launcher behavior was compiled with `-std=c11 -O2 -Wall -Wextra -Werror` and exercised against an adversarial mock installer.

Observed result:

```text
PASS same_inode_source_mutation_isolated
PASS sealed_memfd_rejects_write
PASS pre_snapshot_mutation_blocks_installer
PASS duplicate_placeholder_rejected
RESULT pass=4 fail=0 claim_allowed=false
```

Exit code: `0`.

Transcript SHA-256: `51e29e88f528c4ff0819d5173d97d5d00fd168fd0c807e5f8c00b31715dd26a5`.

Observed write attempt against the sealed `/proc/self/fd/N` failed with `Operation not permitted` while the installer still consumed bytes whose SHA-256 equaled the frozen original digest after the original source inode had been rewritten in place.

Classification is deliberately `LOCAL_EQUIVALENT_PASS`, not `HEAD_EXACT_PASS`, because the test was not executed by fetching and running the provider branch bytes directly.

## Threat boundary

This mitigation covers the specific source-inode mutation class by changing the installer handoff object to a sealed snapshot. It does not establish that Android `adb install` or `pm install` accept `/proc/self/fd/N` backed by memfd on the target device.

It also does not establish physical launch, runtime semantics, signature identity, or cross-device determinism.

## Provider execution

At the first inspection of branch head `5b4a2b9fef7051ff813a124afe4eade2dd4dc952`, no pull-request workflow run was yet observable. Therefore provider CI execution remains `TOKEN_VAZIO`, not PASS or FAIL.

## TOKEN_VAZIO delta

- `TOKEN_VAZIO_SAME_INODE_INPLACE_PROTECTION` -> `MITIGATION_MATERIALIZED_LOCAL_EQUIVALENT_PASS`

Still open:

- `TOKEN_VAZIO_HEAD_EXACT_SEALED_MEMFD_TEST`
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

- sealed immutable snapshot mechanism materialized;
- additive integration path materialized;
- source-inode post-snapshot mutation isolated in executable local probe;
- write-through sealed FD rejected in executable local probe;
- pre-snapshot digest mismatch blocks installer;
- duplicate placeholder fails closed;
- branch topology initially `ahead=4 / behind=0` before this receipt.

## F_gap

The Android kernel/userspace compatibility boundary is still unproved. In particular, `memfd_create`/seals and `adb`/`pm` consumption of `/proc/self/fd/N` require execution on the physical Termux/Android environment. Provider CI also has not executed this head at the time of the receipt.

## F_next

1. execute `tests/test_apkc_sealed_memfd_handoff.sh` from an exact checkout of this branch and bind commit/blob/transcript hashes;
2. run `capture_android_proof_chain_sealed_hardened.sh` in Termux and preserve compiler version, helper binary SHA-256 and status files;
3. probe `pm install` and `adb install` separately with the sealed memfd path;
4. if either transport rejects the path, preserve that negative result and design a different content-bound transport rather than silently falling back;
5. only after physical install/launch evidence consider promoting the corresponding TOKEN_VAZIO states.
