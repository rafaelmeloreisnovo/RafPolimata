# ApkC / RAFAELIA — Document Governance Observability Receipt — 2026-08-14

State: `MATERIALIZED_PENDING_PROVIDER_EXECUTION`

Claim gate: `claim_allowed=false`

## Provenance

- repository: `rafaelmeloreisnovo/RafPolimata`
- base main: `a77d6c2e36ddfa80312aa1413f66b750d1c2be95`
- prior PR: `#246`, merged into main
- prior Document Governance run: `31817563194`
- prior job: `94822796753`
- hardening branch: `hardening/apkc-doc-governance-observability-20260814`
- first hardening commit: `d8765375b87645fe185c5b473e82bbca74846a63`
- draft PR: `#247`

## Observed failure boundary

On the exact PR #246 provider run:

- governance JSON syntax validation: PASS;
- governance unit tests: 18/18 PASS;
- `validate_root_file_decisions.py --write results/root-file-decisions-validation.json`: exit 1;
- downstream ZIP audit/catalog/determinism/evidence steps: skipped because the workflow stopped at the validator;
- the generated validation JSON was not printed before exit, so the precise set of unmapped/stale/critical root decisions remained observationally hidden in the job log.

The validator itself remains fail-closed: it returns exit 1 for `state=FAIL` and computes `claim_allowed=true` only for `state=PASS`.

## Hardening delta

The workflow now:

1. runs the existing root-file validator unchanged;
2. captures its exit status;
3. prints `results/root-file-decisions-validation.json` when materialized;
4. preserves the original validator failure as `steps.root_decisions.outcome=failure` via `continue-on-error`;
5. allows later governance evidence generation to proceed;
6. uploads all available evidence under `if: always()`;
7. executes a final `Enforce root-file decision gate` that fails unless the validator outcome was success.

No route, risk, hash, criticality, root policy, or claim rule is relaxed.

## F_ok

- PR #246 is landed on main.
- CI, ApkC First Part Closure, and Formal Science Orchestrator succeeded on the exact #246 head.
- Document Governance failure was isolated to root-file decision validation after 18/18 governance tests passed.
- PR #247 materializes deterministic diagnostic preservation without masking the exit code.

## F_gap

- `TOKEN_VAZIO_PR247_PROVIDER_EXECUTION`
- `TOKEN_VAZIO_ROOT_DECISION_EXACT_FAILURE_SET`
- physical ApkC install/launch/runtime evidence remains absent and is not affected by this CI-only delta.

## F_next

Execute PR #247 on a hosted runner. Read the emitted root-decision JSON and classify each exact failure as `UNMAPPED`, `STALE_HASH`, or `CRITICAL/QUARANTINE`. Update only the decisions justified by current Git blob identity/content; do not mass-refresh hashes or suppress critical states. Re-run until the governance state is evidence-backed.
