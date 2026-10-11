# MU Freestanding Gap Falsifier Gate Receipt - 2026-10-11

mu_id: `MU-FREESTANDING-GAP-FALSIFIER-GATE-V1`  
timestamp_local: `2026-10-11T17:20:00-03:00`  
repo: `rafaelmeloreisnovo/RafPolimata`  
pr: `#429`  
branch: `codex/freestanding-contract-v1-20261011-1540`  
status: `SOURCE_DELTA / FAIL_CLOSED / CLAIM_ALLOWED_FALSE`  
Closure: `CLOSURE_L13`  
claim_allowed: `false`

## Delta

Added the freestanding gap falsifier layer:

1. `tools/validate_freestanding_gap_falsifiers.py`
2. `.github/workflows/freestanding-gap-falsifiers.yml`
3. `docs/evidence/FREESTANDING_GAP_FALSIFIERS_V1.md`

## Evidence boundary

This receipt records source materialization only. Provider execution remains `TOKEN_VAZIO_PROVIDER` until the workflow reaches terminal status at an exact commit.

## Non-regression boundary

No freestanding runtime, Android, Termux, Frida, Gradle, JNI, NDK, R8, Java/Kotlin/Python/JS/Lua implementation path was modified by this delta.

## Rollback

Rollback is branch-level and append-only: revert the commits adding the script, workflow and evidence document. Existing receipts are not deleted.

## R3

F_ok:
- a new fail-closed source-side falsifier attacks untyped gaps and over-promotion.

F_gap:
- provider run, report artifact, physical runtime, independent reproduction and full license map remain open until measured.

F_next:
- observe terminal GitHub Actions checks and bind the run id, job id and conclusion in the next receipt.
