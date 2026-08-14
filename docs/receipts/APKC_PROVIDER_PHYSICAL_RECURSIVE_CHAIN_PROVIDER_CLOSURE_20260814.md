# ApkC Provider → Physical Recursive Custody Chain — Provider Closure — 2026-08-14

State: `PROVIDER_VERIFIED / PHYSICAL_TOKEN_VAZIO`

Claim gate: `claim_allowed=false`

## Corrected provider execution

Workflow: `ApkC Provider Physical Recursive Chain`

Run: `31840747698`

Job: `94896850591`

Provider runner: Ubuntu 24.04, real executed steps.

Observed step results:

- checkout: PASS
- syntax: PASS
- provider-to-physical custody falsifiers: PASS
- evidence upload: PASS

Observed exact summary:

`RESULT pass=7 fail=0 claim_allowed=false`

Passing controls:

1. positive recursive parent/child binding;
2. nested physical-tree substitution detected;
3. negative evidence sealed and original non-zero exit preserved;
4. dirty output directory rejected;
5. missing provider transfer gate rejected fail-closed;
6. unknown mode rejected;
7. claim gate preserved (`claim_allowed=true` absent).

## Evidence artifact

- artifact id: `9234132801`
- bytes: `361`
- provider-reported SHA-256: `10f8998aadc13b0878779d8c96f4395b1f38e4ade5eb0e0722f6b194310a43fc`

The earlier negative run `31840645187` and artifact `9234098108` remain preserved in the separate errata receipt; this closure does not erase the failed observation.

## TOKEN_VAZIO delta

`TOKEN_VAZIO_PROVIDER_PHYSICAL_RECURSIVE_CHAIN_EXECUTION`

→ `RESOLVED_PROVIDER_7_OF_7_PASS`

Still open:

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## Evidence boundary

The provider result verifies shell composition, recursive custody, falsifiers, and fail-closed behavior in the hosted Linux environment. It does not prove any Android/Termux physical behavior.

The next physical action remains a byte-exact C04 `probe-only` invocation. No install promotion is authorized before that physical probe succeeds.

## F_ok

Parent provider-transfer evidence now cryptographically binds nested physical evidence; negative observations remain sealable; provider execution passed all seven controls with real steps.

## F_gap

Physical device capability, Package Manager stdin acceptance, install, launch, runtime semantics, and cross-device determinism remain unproved.

## F_next

Use the merged/current C04 artifact and exact SHA on the user-controlled Termux device in `probe-only`, preserving the entire generated evidence tree. Only after a physical PASS should `adb-shell-pm` be attempted.
