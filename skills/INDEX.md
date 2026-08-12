# Skills Index — RafPolimata

**Role:** navigational entry for operational skills.  
**Governance:** `docs/AGENTES.md` + `docs/INDEX.md`.  
**Promotion rule:** skill presence does not imply scientific `CLAIM_ALLOWED`.

| Skill | Purpose | State | Runtime |
|---|---|---|---|
| [academic-falsifiability](academic-falsifiability/SKILL.md) | type-before-operation, explicit falsifier, provenance, negative controls, reproduction ladder and fail-closed claim promotion | `METHOD_DEFINED / LOCAL_REFERENCE_PASS / CI_INFRA_BLOCKED` | `scripts/epistemic_claim_gate.py` |

## Related artifacts

- `configs/epistemic-claim.v1.schema.json` — governed claim contract.
- `configs/epistemic-domain-adapters.v1.json` — domain-specific gates for mathematics, software/runtime, statistics/data science, cosmology, engineering physical systems, biology/biomedicine, legal/governance, provenance/memory and symbolic/parabolic work.
- `templates/epistemic-claim.v1.template.json` — safe fail-closed starting state; begins as `TOKEN_VAZIO` and `claim_allowed=false`.
- `scripts/epistemic_claim_gate.py` — fail-closed procedural gate.
- `tests/test_epistemic_claim_gate.py` — adversarial unit tests.
- `.github/workflows/epistemic-claim-gate.yml` — remote execution gate.
- `data/governance/epistemic-skill-events.v1.jsonl` — append-only implementation/evidence events.

`CI_REMOTE_PASS` remains `TOKEN_VAZIO` while GitHub Actions cannot allocate a runner.
