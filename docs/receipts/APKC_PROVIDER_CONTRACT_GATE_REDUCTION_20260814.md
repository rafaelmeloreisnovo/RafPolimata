# ApkC — Provider Contract Gate Reduction — 2026-08-14

State: `VERIFIED_LIMITED_PROVIDER`

Claim gate: `claim_allowed=false`

Branch: `hardening/apkc-provider-drift-fix-20260814`

Head after workflow correction: `2e2f0a99d3ce4474ebb213bddb7a373e81928825`

## Purpose

Reduce two provider-contract ambiguities without weakening source/runtime falsifiers:

1. `ld.lld` absence was previously masked by a pipeline without `pipefail`;
2. Bash-only proof/hardening scripts were incorrectly syntax-checked with `sh -n`.

## Change

`.github/workflows/apkc-first-part.yml` now:

- runs tooling with `set -euo pipefail`;
- installs `qemu-user lld` when either required runtime tool is unavailable;
- requires `command -v qemu-aarch64`, `clang`, and `ld.lld`;
- keeps `sh -n` for POSIX-shell helpers;
- uses `bash -n` for Bash tests/proof helpers.

No source/runtime proof gate was removed or converted to warning.

## Provider execution

Workflow: `ApkC First Part Closure`

Run: `31790754368`

Job: `94736883153`

Observed provider evidence:

- hosted runner executed real steps;
- Ubuntu 24.04.4 / runner image 20260810.271.1;
- package install included `lld`, `qemu-user`, `qemu-user-binfmt`;
- `Ubuntu LLD 18.1.3 (compatible with GNU linkers)` observed;
- tooling step: `success`;
- syntax/contracts step: `success`;
- custody hardening step: `success`;
- contracts audit step: `success`.

Artifact:

- artifact ID: `9215356135`
- bytes: `124531`
- SHA-256: `1abe5528dffddd252e80913327d53553455cdb520609db51f1c462262a848a82`

## Remaining fail-closed roots

The final enforcement remains FAIL and this receipt does not promote it.

Provider-observed roots:

- `PROOF-FAIL-CLOSED` still checks the historical `source-to-binary-proof.v2` marker while the current proof family is v3;
- source-to-binary proof stops at `source-cap hardening failed`;
- runtime hardening stops at `APKC-RH-007: expected 1 occurrence(s), found 0`;
- browser/TLS unit test reports `https_transport_adapter_static_evidence=false`.

Downstream failures (`GENERATE_APK`, `FIRST_PART`, `STRUCTURAL_RECEIPT`) are preserved as consequences rather than relabeled as independent root causes.

## TOKEN_VAZIO delta

Resolved with provider evidence:

- `TOKEN_VAZIO_BASH_VS_SH_SYNTAX_GATE` -> `RESOLVED_PROVIDER_SYNTAX_SUCCESS`
- `TOKEN_VAZIO_TOOLING_LLD_FALSE_GREEN` -> `RESOLVED_PROVIDER_LLD_REQUIRED_AND_OBSERVED`

Still open:

- `TOKEN_VAZIO_PROOF_SCHEMA_V3_GATE_RECONCILIATION`
- `TOKEN_VAZIO_SOURCE_CAP_CURRENT_SOURCE_RECONCILIATION`
- `TOKEN_VAZIO_RUNTIME_PATCH_CURRENT_SOURCE_RECONCILIATION`
- `TOKEN_VAZIO_BROWSER_TLS_STATIC_EVIDENCE_RECONCILIATION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_ok

Provider tooling is now fail-closed and observable; shell syntax validation uses the interpreter actually required by each script; custody remains PASS; evidence artifact is retained; no Android or global closure claim was promoted.

## F_gap

Source-cap transform, proof-schema gate, RH-007 transform and HTTPS static evidence still fail. Physical Android/Termux execution remains absent.

## F_next

Prioritize current-source transform reconciliation, starting with a byte-level diagnosis of why earlier transforms make the RH-007 anchor disappear. Preserve exact-one anchor semantics; do not relax `replace_once`. In parallel, reconcile the v3 proof marker only after confirming the canonical proof schema string and fail-closed semantics.
