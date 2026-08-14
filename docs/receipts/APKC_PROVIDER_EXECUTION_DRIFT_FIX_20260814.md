# ApkC — Provider Execution + Contract Drift Fix — 2026-08-14

State: `VERIFIED_LIMITED / claim_allowed=false`

## Provider execution evidence

A manual re-run of GitHub Actions run `31782502621` produced a real hosted runner execution for PR #236 head `420cd839760bad4069be46c83bdda8b00dc3826c`.

Observed job: `94723309452` (`apkc-first-part`).

This closes the earlier ambiguity where jobs ended with `steps=[]` and `runner_id=0`: on this attempt the runner executed actual steps and uploaded an evidence artifact.

Artifact:

- ID: `9213694813`
- name: `apkc-first-part-evidence-58cf0aabfa738b9398c1929286cfffaee98504bf`
- bytes: `124399`
- digest: `sha256:a57c335cfa03bf05349842b9d5fe8fdf45e50e0979645d2b19aa0067605705be`

## Evidence split

The provider run must not be summarized as a single generic `CI FAIL`. It exposed independent classes:

### Custody shell family

Four of five governed suites passed exactly in provider execution:

- memfd runtime: `8/8 PASS`
- sealed memfd -> stdin: `6/6 PASS`
- stdin receipt finalizer: `7/7 PASS`
- sealed stdin launcher: `4/4 PASS`

The Termux/Android preflight controlled fixture suite returned `6 PASS / 3 FAIL`; all direct gate invocations returned rc `126`. Root cause: the test executed the GitHub-materialized gate path directly although the contents API does not prove/preserve executable mode. The gate is a Bash script and its test now invokes it through `bash` explicitly.

### Syntax drift

`sh -n scripts/apkc_finalize_stdin_install_receipt.sh` failed on Bash-only `[[ ... ]]` constructs even though the file declares `#!/usr/bin/env bash`. The finalizer was rewritten to shell-portable `case`/`[` validation while preserving its fail-closed semantics and status contract.

### Manifest test drift

The canonical manifest is `1.6.0`, while the unit test still asserted `1.5.0`. The assertion now follows the versioned manifest without changing `claim_allowed=false` or the frozen-feature boundary.

### Still-real blockers — not relaxed

The same provider execution also exposed failures that are intentionally NOT hidden by this delta:

- `APKC-RH-007: expected 1 occurrence(s), found 0` in `patch_apkc_runtime_source.py`;
- source-cap transformer anchor drift (`source-cap hardening failed`);
- `PROOF-FAIL-CLOSED` marker drift because the proof is now schema v3 while the gate still searches the historical v2 marker;
- browser TLS static-evidence assertion false;
- source proof, generated APK, runtime hardening and structural closure consequently remain failed/blocked.

Those need separate semantic reconciliation against current source. No assert was weakened to create a false green.

## TOKEN_VAZIO delta

- `TOKEN_VAZIO_PROVIDER_RUNNER_EXECUTION` -> `RESOLVED_PROVIDER_STEPS_EXECUTED_RUN_31782502621_ATTEMPT_2`
- `TOKEN_VAZIO_PROVIDER_CUSTODY_SHELL_EXECUTION` -> `PARTIAL_PROVIDER_EXECUTION_4_OF_5_SUITES_PASS`
- `TOKEN_VAZIO_PREFLIGHT_FIXTURE_EXEC_PERMISSION_DRIFT` -> `CORRECTION_MATERIALIZED`
- `TOKEN_VAZIO_FINALIZER_SHELL_SYNTAX_DRIFT` -> `CORRECTION_MATERIALIZED`

Still open:

- `TOKEN_VAZIO_PROVIDER_CUSTODY_SHELL_5_OF_5_PASS`
- `TOKEN_VAZIO_RUNTIME_PATCH_CURRENT_SOURCE_RECONCILIATION`
- `TOKEN_VAZIO_SOURCE_CAP_CURRENT_SOURCE_RECONCILIATION`
- `TOKEN_VAZIO_PROOF_SCHEMA_V3_GATE_RECONCILIATION`
- `TOKEN_VAZIO_BROWSER_TLS_STATIC_EVIDENCE_RECONCILIATION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`

## R3

`F_ok`: provider runner actually executed; evidence artifact preserved with digest; 25 custody checks passed in provider; three objective contract drifts corrected without changing physical/runtime claims.

`F_gap`: current-source transformer anchors and browser/proof gate reconciliation remain real failures; Android physical evidence remains absent.

`F_next`: run this drift-fix branch in provider CI; require the custody family to become 5/5 while preserving the remaining source/runtime failures, then reconcile `APKC-RH-007` and source-cap anchors against the current `Apkc/apkc.c` structure rather than weakening falsifiers.
