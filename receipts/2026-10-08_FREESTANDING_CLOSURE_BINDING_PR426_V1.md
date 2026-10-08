# PR #426 — exact-path closure binding hotfix

Date: 2026-10-08
Repository: `rafaelmeloreisnovo/RafPolimata`
Parent PR: #426
Parent commit: `ac3309771c6757ddd57c6ec3afaa889cd1682dcc`
Source falsifier: [run 37840640767](https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/37840640767), job 113528825719
Disposition: IMPLEMENTED_UNTESTED_CI / claim_allowed=false

## Evidence and cause

- The existing validator's changed-lines gate reported 41 errors on the eight added freestanding files.
- The changed files contain explicit `TOKEN_VAZIO` states (CLOSURE_L11 governs this absence; it is not a numeric zero).
- Source contract mismatches affecting root README, LICENSE and LICENSE_SCOPE were observed in the same job but these source files were not changed by this PR. Their provenance is a separate P0 investigation.

## Minimal correction

Add **only exact paths** for eight files to `TokenVazioValidator.STRUCTURED_CLOSURE_PATHS`.
Use `CLOSURE_L2` for physical device/runtime gaps and `CLOSURE_L11` for operational routing/contract unknowns.
Add a regression test for all eight bindings, plus a sibling path that must still fail.
Preserve all original data manifests, schemas, specs, historical receipts and copyrights byte-for-byte.

## Validation boundary

- Source defect and regression test: materialized in this delta.
- Previous 41 source findings: expected to become governed warnings after fresh CI executes.
- No new build/ELF/APK or physical device run performed.
- Do not weaken numerical, license, ownership or provider gates.
- If fresh CI still fails, retain FAIL and diagnose the exact logs.

## Rollback

Revert this commit; previous eight files are not modified.
No history rewrite or force-push.

## R3

F_ok: exact-path closure map and negative sibling test.
F_gap: CI readback at new HEAD, source-contract ownership mismatches and device evidence.
F_next: observe exact-HEAD CI and attach a successor receipt only for tested outcomes.
