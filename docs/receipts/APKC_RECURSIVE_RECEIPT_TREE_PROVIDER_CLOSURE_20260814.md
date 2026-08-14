# ApkC — Recursive Receipt Tree — Provider Closure — 2026-08-14

State: `VERIFIED_LIMITED_PROVIDER`

Claim gate: `claim_allowed=false`

## Tested head

- repository: `rafaelmeloreisnovo/RafPolimata`
- branch: `hardening/apkc-recursive-receipt-tree-20260814`
- exact tested head: `33afadb9cd572abc2a0ce847f2687dc4cc820e2f`
- workflow: `ApkC Recursive Receipt Tree`
- run: `31836121778`
- job: `recursive-receipt-custody`
- job conclusion: `success`

Executed provider steps include checkout, Bash syntax validation, receipt-tree falsifiers, receipted-gate falsifiers and artifact preservation. This was a real executed job, not an empty runner/startup observation.

## Provider evidence

Artifact:

- id: `9232533262`
- name: `apkc-recursive-receipt-tree-evidence`
- bytes: `556`
- digest: `sha256:d8cf9454feafe58295648bca6f315eb7e3fedee72d4345aaaad15e7f70bedf00`
- artifact head: `33afadb9cd572abc2a0ce847f2687dc4cc820e2f`

The dedicated workflow requires these exact terminal lines:

- `RESULT pass=5 fail=0 claim_allowed=false` for recursive receipt-tree falsifiers;
- `RESULT pass=5 fail=0 claim_allowed=false` for the compositional receipted physical-gate falsifiers.

Therefore the provider contract covers 10 adversarial checks in total.

## Closed gap

`TOKEN_VAZIO_RECURSIVE_RECEIPT_TREE_PROVIDER_EXECUTION -> RESOLVED_PROVIDER_10_OF_10_PASS`

The result proves recursive custody logic and fail-closed composition on the hosted provider. It does not prove physical Android behavior.

## Still open

- `TOKEN_VAZIO_RECURSIVE_RECEIPT_TREE_PHYSICAL_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_ok

Recursive nested evidence binding, post-seal nested tamper detection, symlink rejection, deterministic manifest, empty-tree rejection, physical-gate failure propagation and claim gate were provider-executed successfully. Existing canonical physical gate was not rewritten.

## F_gap

No device-side execution of the receipted wrapper has been observed. Provider success cannot promote physical compatibility or runtime.

## F_next

Use this wrapper first in physical `probe-only` mode on the exact C04 APK/SHA pair. Preserve the resulting full recursive manifest. Only after that passes should an install-mode attempt be made. Any device rejection remains evidence and must not silently fall back to pathname transport.
