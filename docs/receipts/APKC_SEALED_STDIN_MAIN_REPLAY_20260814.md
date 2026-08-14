# ApkC — Sealed stdin main replay receipt — 2026-08-14

State: `PROVIDER_MATERIALIZED / TOPOLOGY_REPAIRED / claim_allowed=false`

Mode: `APPEND_ONLY / BYTE_PRESERVING_REPLAY / NO_RETROACTIVE_REWRITE`

## Trigger

PR #233 (`hardening(apkc): stream sealed memfd to Package Manager stdin`) was merged into its stacked base branch `hardening/apkc-combined-install-receipt-20260814` after PR #232 had already merged that base into `main`.

Observed current `main` before repair:

- commit: `39f35ed4fae95bafe3b7991aab2d97711bd2b536`
- tree: `030f3c310cac3ec0afc3fcbaaed54aed4ee19bfe`
- message: merge PR #232

Therefore the PR #233 state `merged=true` did not imply that its seven-file sealed-stdin delta was present in current `main`.

## Byte-preserving replay

Source head: `83db582eb09f16618b793cacfe225b9672dfb8dd` (PR #233).

Fresh branch: `hardening/apkc-sealed-stdin-main-replay-20260814`, created from current main `39f35ed4fae95bafe3b7991aab2d97711bd2b536`.

Replay commit: `feef0dc03cca96b6c79336d8746dd71982fbc4a1`.

The replay used the exact provider Git blob identities from PR #233 rather than regenerating equivalent text:

- `docs/receipts/APKC_SEALED_STDIN_TRANSPORT_20260814.md` -> blob `35ad9d0e194c57732e0a4ae7e53223a51039ec45`
- `scripts/apkc_finalize_stdin_install_receipt.sh` -> blob `101fa76d1dc1228db68defe2784afa79931a9ab8`
- `scripts/apkc_install_sealed_stdin.c` -> blob `944f039e188992987e6a14f9cf97dbf777cb2384`
- `scripts/apkc_install_sealed_stdin.sh` -> blob `cac0dfa72ccdee6644c3790791ed37a17c89ee8a`
- `tests/test_apkc_finalize_stdin_install_receipt.sh` -> blob `2599129c9a36ff75ced720ff651dc24eb74c1264`
- `tests/test_apkc_install_sealed_stdin.sh` -> blob `e16d91deeb9f0deae1c42af1895d77a0794dd5e3`
- `tests/test_apkc_install_sealed_stdin_launcher.sh` -> blob `fa35dcd00bc873b7c521da834462fbc260661188`

Provider comparison immediately after replay:

- status: `ahead`
- ahead_by: `1`
- behind_by: `0`
- changed files: `7`
- additions/deletions: `+450/-0`

## Evidence boundary

This receipt proves provider topology and exact Git-object replay. It does not re-run or promote the previous local tests, and it does not prove Android/Termux physical behavior.

At the first workflow query for replay head `feef0dc03cca96b6c79336d8746dd71982fbc4a1`, no workflow runs were returned. Therefore:

- `provider_ci_execution=TOKEN_VAZIO_NO_RUN_OBSERVED`
- not PASS
- not CODE_FAIL

Still open:

- `TOKEN_VAZIO_HEAD_EXACT_REPLAY_TEST_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PM_LOCAL_STDIN_PERMISSION_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

`claim_allowed=false`.

## F_ok

The orphaned stacked delta is now represented on a branch based directly on current main, using the exact seven source blobs. PR #234 records the repair without rewriting PR #233 history.

## F_gap

The replay head has not yet executed its test suite on an observable provider runner or on the physical Android/Termux target. Physical stdin compatibility remains unobserved.

## F_next

1. execute the three landed replay tests on the exact replay head when a runner actually starts;
2. if exact-head tests pass, land the topology repair into main without modifying the replayed blobs;
3. on physical Termux/Android, run the existing memfd capability probe, then the sealed-stdin launcher in `adb-shell-pm` mode;
4. preserve stdout/stderr, all status files, APK SHA-256, device/build identity and exit code;
5. any physical rejection remains evidence and must not trigger pathname fallback.
