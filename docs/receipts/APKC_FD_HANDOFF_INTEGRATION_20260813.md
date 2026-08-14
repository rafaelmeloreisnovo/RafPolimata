# ApkC FD Handoff Integration Receipt — 2026-08-13

State: `VERIFIED_LIMITED / LOCAL_EQUIVALENT_PASS`

`claim_allowed=false`

## Provider source identity

- base `main`: `cd2511476f7119b47eea93fa6c228772122979e6`
- branch: `hardening/apkc-fd-handoff-integrated-20260813`
- integration-test head before this receipt: `a0ecdfd07e7cac1a7c5a4f8d7a2d742e782ea542`
- helper blob: `18fe4cf8ba4d47a84ad597dc1073bae5e18f29d0`
- wrapper blob: `2e2833793dc43fad29ddc2ecba48ceeb970899e0`
- test blob: `cc3e011b2b3435fa7872e4234f670cf761a88dd0`

## Contract

Existing hardened chain remains intact. The new wrapper interposes only `adb install` / `pm install` and delegates the APK pathname to `apkc_install_fd_handoff.sh`. The helper opens the APK, hashes `/proc/<pid>/fd/9`, requires equality with the already-frozen SHA-256, and keeps the descriptor open across the real installer child.

Targeted property:

`pathname replacement after FD open != change of installer-consumed inode`

Explicit limitation:

`same_inode_inplace_protection=TOKEN_VAZIO`

Real Android/ADB/PM acceptance of `/proc/<pid>/fd/N` is also `TOKEN_VAZIO` until physical execution.

## Controlled local behavioral probe

A functionally equivalent local harness was executed on Linux with a fake installer. Results:

```text
PASS rename_swap_consumes_fd_bound_original
PASS pre_fd_mutation_blocks_installer
PASS same_inode_limitation_preserved
RESULT pass=3 fail=0 claim_allowed=false
exit_code=0
```

Classification is deliberately `LOCAL_EQUIVALENT_PASS`, **not** `HEAD_EXACT_PASS`: the probe reproduced the landed logic but did not execute provider-fetched byte-identical blobs.

## F_ok

- FD handoff helper landed;
- compatibility wrapper landed without modifying the canonical hardened bridge;
- adversarial integrated test source landed;
- branch was `ahead_by=3 / behind_by=0` before this receipt;
- local equivalent 3/3 behavioral probe PASS.

## F_gap

- `TOKEN_VAZIO_HEAD_EXACT_FD_HANDOFF_TEST`;
- `TOKEN_VAZIO_ADB_FD_PATH_COMPATIBILITY_PHYSICAL`;
- `TOKEN_VAZIO_PM_FD_PATH_COMPATIBILITY_PHYSICAL`;
- `TOKEN_VAZIO_SAME_INODE_INPLACE_PROTECTION`;
- `TOKEN_VAZIO_PHYSICAL_INSTALL`;
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`;
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`;
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`.

## F_next

1. Execute the provider-fetched exact branch test and preserve transcript + blob/SHA-256 identities.
2. On Termux/Android, test `adb install` and `pm install` separately with the FD handoff.
3. Preserve negative evidence if either installer rejects `/proc/.../fd/N`.
4. Only if compatible, integrate this wrapper into the canonical invocation route; do not remove the existing path/digest/signature/final-receipt gates.
