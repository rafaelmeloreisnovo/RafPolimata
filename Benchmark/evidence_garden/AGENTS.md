# AGENTS.md — Evidence Garden scope

Applies to `Benchmark/evidence_garden/**`.

Governance anchor: `CLOSURE_L2` (runtime-evidence uncertainty remains a gap, never a promoted claim).

Read the repository root `AGENTS.md` and `docs/AGENTES.md` first. This file adds benchmark-specific constraints.

## Invariants

```text
source != artifact != execution != evidence != claim
measurement != causality
missing probe != zero
optional unavailable probe = TOKEN_VAZIO
single run != reproduction
```

- Never promote a timing difference into universal superiority.
- Never label a declared factor as causal merely because it is the only metadata field changed.
- Correctness equality is bounded to the executed vectors unless a stronger reference gate is supplied.
- Keep observability separate from the low-observer performance lane.
- Preserve raw samples, failed attempts and unavailable probes.
- Do not hide a required probe/test failure with `|| true`, skipped exits or rewritten statuses.
- Commands in experiment configs are argv arrays; do not add shell-string execution to the runner.
- New receipt/config layouts require a schema/version change and compatibility decision.
- Generated runtime receipts belong under `build/` or CI artifacts, not as hand-authored PASS evidence in source.

## Minimum gates after changes

```sh
python3 -m unittest -v tests.test_evidence_garden
python3 scripts/evidence_garden.py run \
  --config Benchmark/evidence_garden/demo_experiment.json \
  --out build/evidence-garden/local/receipt.json
```

A successful framework self-test proves the runner contract in that environment. It does not validate any unrelated benchmark target.
