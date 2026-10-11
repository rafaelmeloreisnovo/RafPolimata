# MU Freestanding Falsifier Run Receipt — 38166390762

mu_id: `MU-FREESTANDING-FALSIFIER-RUN-38166390762`  
timestamp_local: `2026-10-11T16:03:45-03:00`  
repo: `rafaelmeloreisnovo/RafPolimata`  
branch: `codex/freestanding-contract-v1-20261011-1540`  
pr: `#429`  
head_sha: `5c1e315f6b098b2f50bfbb62e7fda4f6513ebb44`  
run: `https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/38166390762`  
job: `https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/38166390762/job/114549720653`  
status: `completed`  
conclusion: `success`  
evidence_state: `MEASURED_LOCAL`  
claim_allowed: `false`

## What was measured

GitHub Actions executed the workflow `Freestanding Reconstruction Falsifiers` at exact head `5c1e315f6b098b2f50bfbb62e7fda4f6513ebb44`.

The job `source-side falsifier gates` completed successfully. Its steps completed successfully:

- checkout exact source;
- baseline and authority receipt;
- license/provenance probe;
- manifest JSON parse;
- source-side falsifier gates;
- receipt summary.

The workflow source is fail-closed: the `Run source-side falsifier gates` step exits nonzero if any gate command exits nonzero. Therefore the successful terminal job status supports a scoped PASS for the commands declared in the workflow:

- `make authorial-freestanding`
- `make authorial-zero-dependency`
- `make language-contract`
- `make evidence-garden-falsifiers`

## Limits

This is not a global PASS.

Still open:

- raw command logs were not embedded in this receipt;
- root license map remains partial/TOKEN_VAZIO until per-surface license inventory is bound;
- physical ARM32/ARM64 execution remains open;
- independent reproduction remains open;
- other workflows on the same head include failures and must be triaged separately.

Observed non-promoted sibling workflow states on the same head:

- `CI`: failure;
- `Workflow Graph Audit`: failure;
- `Internal Custody Ledger`: success;
- `Document Governance`: success;
- `Formal Science Orchestrator`: success.

## R3

F_ok:
- new falsifier workflow reached terminal success on GitHub provider for exact head `5c1e315f6b098b2f50bfbb62e7fda4f6513ebb44`;
- four source-side gates can now be marked `MEASURED_LOCAL_PASS` for this exact run.

F_gap:
- global PR status still has unrelated failing workflows;
- raw logs, full license map, physical runtime and independent reproduction remain open.

F_next:
- triage `CI` and `Workflow Graph Audit` failures without changing source unless a source-side falsifier is identified.
