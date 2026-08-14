# ApkC — CI Custody Shell Suite Wiring — 2026-08-14

State: `PROVIDER_MATERIALIZED / EXECUTION_TOKEN_VAZIO`

Claim gate: `claim_allowed=false`

## Purpose

Close a CI coverage gap without promoting physical Android evidence. The landed ApkC sealed-memfd/stdin/preflight hardening had adversarial shell tests in the repository, but `.github/workflows/apkc-first-part.yml` did not execute those newer shell batteries in its enforced result.

Base main observed before this delta:

`128caf280549eedc3ff7a4e60ea9f624a29cd41a`

This base is the merge of PR #235 and already contains the Termux/Android sealed-stdin preflight gate.

## Delta

The existing `ApkC First Part Closure` workflow is extended additively.

A new step `custody_hardening` runs five existing batteries:

1. `tests/test_apkc_probe_memfd_runtime.sh` — expected contract summary `8/8`;
2. `tests/test_apkc_install_sealed_stdin.sh` — expected contract summary `6/6`;
3. `tests/test_apkc_finalize_stdin_install_receipt.sh` — expected contract summary `7/7`;
4. `tests/test_apkc_install_sealed_stdin_launcher.sh` — expected contract summary `4/4`;
5. `tests/test_apkc_termux_sealed_stdin_gate.sh` — expected contract summary `9/9`.

Total contract checks represented by those existing batteries: `34`.

The step:

- compiles `scripts/apkc_install_sealed_stdin.c` with `clang -std=c11 -O2 -Wall -Wextra -Werror` before its binary-level test;
- uses `set -euo pipefail`;
- requires all five exact `RESULT pass=... fail=0 claim_allowed=false` summaries;
- requires exactly five result summaries;
- rejects an observed `claim_allowed=true` line;
- preserves the combined transcript at `results/apkc-custody-hardening-shell-tests.log` as a workflow artifact.

The final workflow enforcement now requires:

`CUSTODY_HARDENING_OUTCOME == success`

alongside the pre-existing tooling, syntax, Python tests, source proof, APK generation, hardening, first-part, contracts and structural receipt outcomes.

## Compatibility / anti-regression

No ApkC installer, preflight, finalizer, C helper, test semantics, Android package-manager command or structural receipt behavior is modified by this delta. Only CI coverage/enforcement and evidence preservation are changed.

The existing structural boundary remains mandatory:

- `claim_allowed=false`;
- `runtime_boundary.apk_installed=TOKEN_VAZIO`;
- `runtime_boundary.package_launched=TOKEN_VAZIO`.

## Evidence boundary

Provider readback proves the updated workflow content is materialized on branch `hardening/apkc-ci-custody-shell-suite-20260814`.

This receipt does **not** claim that GitHub Actions executed the new step. Until a workflow job with non-empty executed steps is observed for the exact head, provider execution remains:

`TOKEN_VAZIO_PROVIDER_RUNNER_EXECUTION`.

It also does not claim Termux ARM32, Android memfd/seals, adb-shell-pm stdin compatibility, physical install, launch, runtime semantic behavior or cross-device determinism.

## TOKEN_VAZIO delta

`TOKEN_VAZIO_HEAD_EXACT_HARDENING_TEST_WIRING`
→ `MATERIALIZED_IN_CI_CONTRACT`

Still open:

- `TOKEN_VAZIO_PROVIDER_RUNNER_EXECUTION`
- `TOKEN_VAZIO_HEAD_EXACT_CUSTODY_SHELL_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## R3

`F_ok = CI wiring is provider-materialized, fail-closed in final enforcement, and preserves a combined transcript artifact.`

`F_gap = exact provider execution and all physical Android evidence remain unobserved.`

`F_next = open a draft PR, observe the exact-head workflow; if runner executes, inspect the custody_hardening step and artifact before any promotion. If runner again has steps=[]/runner_id=0, classify startup failure, not code failure.`
