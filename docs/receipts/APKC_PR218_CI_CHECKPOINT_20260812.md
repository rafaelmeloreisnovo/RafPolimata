# ApkC PR #218 CI checkpoint — 2026-08-12

status: `VERIFIED_LIMITED`
claim_allowed: `false`
pr: `#218`
head_before_this_receipt: `0e20426dfaad3e0f323b495fb7bb031ea39ccc94`

## Workflow observations

GitHub Actions runs associated with head `0e20426d...`:

| workflow | run_id | run conclusion | job observation | classification |
|---|---:|---|---|---|
| M063 language completion freestanding contract | 31595800348 | failure | job `contract`, steps=null, logs_url=null | TOKEN_VAZIO_RUNNER_EXECUTION |
| CI | 31595800337 | failure | `Semantic coherence and falsifiability gates`, steps=null, logs_url=null | TOKEN_VAZIO_RUNNER_EXECUTION |
| ApkC First Part Closure | 31595800338 | failure | `apkc-first-part`, steps=null, logs_url=null | TOKEN_VAZIO_RUNNER_EXECUTION |
| Formal Science Orchestrator | 31595800442 | failure | `Units, evidence, statistics and time gates`, steps=null, logs_url=null | TOKEN_VAZIO_RUNNER_EXECUTION |
| Document Governance | 31595800400 | workflow completed/failure | job `document-governance` observed queued, conclusion=null, steps=null, logs_url=null | CONTRADICTION_RUNNER_STATE |

## Epistemic rule

A workflow-level `failure` without executed step evidence or logs is not promoted to `CODE_FAIL`. The contradictory Document Governance state is preserved rather than normalized away.

closure_gate for code regression attribution:

1. job actually starts;
2. concrete step is present;
3. failing command/step and exit/result are available;
4. failure can be tied to the PR head rather than runner provisioning/infrastructure.

Until then:

- `CI_CODE_REGRESSION = TOKEN_VAZIO`
- `RUNNER_EXECUTION = TOKEN_VAZIO`
- `DOCUMENT_GOVERNANCE_STATE = CONTRADICTION`
- `claim_allowed = false`

F_next: reconcile PR #218 with current main, then rerun the hardened proof paths on a runner that exposes actual steps/logs; keep ARM/device gates independent.
