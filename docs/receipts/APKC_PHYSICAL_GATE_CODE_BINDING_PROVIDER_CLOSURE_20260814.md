# ApkC Physical Gate Code Binding — Provider Closure — 2026-08-14

State: `PROVIDER_VERIFIED / PHYSICAL_TOKEN_VAZIO`

Claim gate: `claim_allowed=false`

## Exact provider execution

Pull request: `#251`

Tested branch head: `9f7a70a5c5cd4ebb74ab3dd93cf9c3e84c27cf48`

GitHub pull-request merge ref checked out by Actions: `18c96b764d6565d1a29537ff8af1da86b849de30`, which merged that exact head into base `7acf3857f6c4cc1d65d1b415e92fe215cc4d8bfe` for testing.

Workflow: `ApkC Physical Gate Code Binding`

Run: `31845289620`

Job: `94910351990`

Runner: GitHub-hosted Ubuntu 24.04.4, runner version `2.336.0`, real executed steps.

Observed steps:

- checkout: PASS;
- Bash syntax: PASS;
- gate-code custody falsifiers: PASS;
- evidence upload: PASS.

Observed exact summary:

`RESULT pass=7 fail=0 claim_allowed=false`

Passing controls:

1. positive code-bound chain;
2. gate-code tamper detected;
3. nested execution-evidence tamper detected;
4. missing dependency rejected fail-closed;
5. negative child exit and evidence preserved;
6. dirty output directory rejected;
7. claim gate preserved.

## Evidence artifact

- artifact name: `apkc-physical-gate-codebinding-evidence`;
- artifact id: `9235662287`;
- bytes: `322`;
- SHA-256 reported by upload action: `4ed006a9ce2b1e1d5e31131db12141cbb0934b1b7ae6d8cd357a111ac679207b`.

## What this proves

The provider execution proves that the additive code-bound wrapper can freeze SHA-256 identities of nine transitive ApkC measurement/gate files, detect code tamper, execute the existing child custody chain, preserve negative evidence and non-zero exits, and recursively bind the code manifest together with the execution evidence tree.

This closes only the provider/code-custody ambiguity.

It does **not** prove Termux ARM32 capability, Android `memfd_create`/seals, Package Manager stdin acceptance, physical installation, launch, runtime semantics, TLS runtime certification, or cross-device determinism.

## TOKEN_VAZIO delta

`TOKEN_VAZIO_PHYSICAL_GATE_CODEBINDING_PROVIDER_EXECUTION`

→ `RESOLVED_PROVIDER_7_OF_7_PASS`

The earlier broader gap:

`TOKEN_VAZIO_PHYSICAL_GATE_CODE_IDENTITY_BINDING`

can now be represented as `PROVIDER_VERIFIED_CODEBOUND_CUSTODY`, but remains `TOKEN_VAZIO_PHYSICAL_EXECUTION` on real Android until an actual device receipt contains the generated `gate-code.sha256` and `codebound-tree.sha256`.

Still open:

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_ok

Provider execution of the code-binding contract is real and reproducible on hosted Linux; 7/7 adversarial controls pass; artifact evidence is preserved; `claim_allowed=false` remains enforced.

## F_gap

The first physical invocation through the code-bound wrapper has not occurred. Therefore no device capability or runtime state is promoted.

## F_next

Run the byte-exact C04 artifact on the user-controlled Termux device in `probe-only` through `scripts/apkc_provider_to_physical_codebound.sh`. Preserve the complete code-bound root even if the physical child gate rejects. Attempt `adb-shell-pm` only after a physical `probe-only` PASS.
