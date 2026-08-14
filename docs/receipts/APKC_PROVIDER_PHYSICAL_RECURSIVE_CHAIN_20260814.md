# ApkC Provider → Physical Recursive Custody Chain — 2026-08-14

State: `MATERIALIZED / PROVIDER_EXECUTION_PENDING`

Claim gate: `claim_allowed=false`

## Purpose

Close the custody edge between the byte-exact provider C04 transfer evidence and nested physical/preflight evidence without changing the already-proven transfer, physical, or recursive-sealer implementations.

Before this delta, `scripts/apkc_verify_provider_c04_transfer.sh` created a top-level `receipt.sha256` using `find ... -maxdepth 1`, while physical evidence is emitted under `physical/`. PR #249 landed a recursive sealer and a receipted physical gate, but the parent provider-to-device layer did not itself recursively bind the child tree.

## New composition

`provider artifact -> byte-exact C04 transfer -> receipted physical gate -> whole-root recursive receipt tree`

New entrypoint:

- `scripts/apkc_provider_to_physical_receipted.sh`

It:

1. requires a clean evidence output directory;
2. preserves the existing C04 provider artifact transfer gate;
3. for physical modes, injects the existing receipted physical gate rather than bypassing it;
4. writes final PASS/FAIL status before sealing;
5. recursively seals the entire evidence root into `provider-physical-tree.sha256`;
6. independently verifies that manifest;
7. preserves the physical/transfer exit code when the underlying gate rejects;
8. keeps `claim_allowed=false`.

This makes replacement of the whole nested `physical/` directory detectable by the parent custody manifest.

## Adversarial contract

`tests/test_apkc_provider_to_physical_receipted.sh` requires:

- positive recursive binding;
- nested physical tamper/substitution detection;
- negative evidence sealed while original non-zero exit is preserved;
- dirty output directory rejection;
- missing transfer gate fail-closed;
- unknown mode rejection;
- no `claim_allowed=true` evidence.

The dedicated provider workflow requires the exact summary:

`RESULT pass=6 fail=0 claim_allowed=false`

## Evidence boundary

At materialization time, no GitHub-hosted execution of this exact branch is claimed. Physical Android/Termux execution is also not claimed.

`provider execution = TOKEN_VAZIO_PROVIDER_EXECUTION_PENDING`

`physical execution = TOKEN_VAZIO_PHYSICAL_EXECUTION`

## F_ok

- PR #249 recursive receipt baseline is already merged into main.
- Parent/child custody gap identified from current main content.
- Composition is additive; proven lower-level gates are not rewritten.
- Negative evidence remains preservable and fail-closed.

## F_gap

- `TOKEN_VAZIO_PROVIDER_PHYSICAL_RECURSIVE_CHAIN_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_next

Execute the exact head in the dedicated GitHub workflow. If the six falsifiers pass with real steps, reduce only the provider custody gap. The next physical action remains `probe-only` on the user-controlled Termux device with the byte-exact C04 artifact; install is permitted only after that gate passes.
