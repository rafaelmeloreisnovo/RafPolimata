# Academic Falsifiability Skill — RAFAELIA V1

**Status:** `METHOD_DEFINED`  
**Scope:** analytical, scientific, engineering and research claims across RAFAELIA.  
**Default:** `claim_allowed=false`.  
**Parent canon:** `docs/AGENTES.md` + `docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md`.

## 1. Purpose

Turn an idea, measurement, model, simulation or interpretation into an auditable epistemic object without letting one evidence class impersonate another.

The first answer has no privilege. A claim survives only while its explicit falsifier has not defeated it and the evidence supports the exact stated domain.

## 2. Type before operation

Every object MUST declare `claim_kind` before evaluation:

| claim_kind | What it is | Valid gate |
|---|---|---|
| `DEMONSTRATION` | formal derivation/proof | definitions + derivation + checker/review |
| `CONVENTION` | chosen contract, encoding, threshold or naming | coherence + versioned contract |
| `HYPOTHESIS` | empirical/computational proposition that may fail | H0/H1 + falsifier + frozen protocol + evidence |
| `PARABLE` | explanatory/symbolic mapping | interpretive usefulness; never empirical proof |
| `TOKEN_VAZIO` | type/evidence is not yet sufficient | preserve context; do not infer conclusion |

A `PARABLE` cannot promote an empirical claim. A `CONVENTION` cannot prove a physical fact. A `DEMONSTRATION` cannot substitute for measurement when the proposition is empirical.

## 3. Existing epistemic states remain canonical

Use the states from `docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md`:

`METAPHOR → FORMALIZED → METHOD_DEFINED → SIMULATED → EVIDENCE_LINKED → REPLICATED → CLAIM_ALLOWED`

with side states `TOKEN_VAZIO` and `CONTRADICTION`.

Promotion is monotonic only with evidence. Negative evidence is append-only and may demote a claim.

## 4. Mandatory claim record

A scientific/technical claim record MUST include:

- `claim_id`: stable identifier;
- `proposition`: exact falsifiable sentence;
- `claim_kind`;
- `state`;
- `domain`: where the proposition is supposed to hold;
- `assumptions`;
- `h0` and `h1` for hypotheses;
- `falsifier`: observation/test that would defeat or materially limit the proposition;
- `protocol`: executable or unambiguous procedure;
- `metric` and `threshold`: fixed before reading the final result when applicable;
- `data_provenance`: source, version and content hash;
- `code_provenance`: repository, commit and relevant artifact hash;
- `environment`: architecture/runtime/tool versions when execution-dependent;
- `evidence`: commands, outputs, receipts and hashes;
- `negative_evidence`: failures, counterexamples and contradictory observations;
- `replication`: local and independent replication state;
- `limitations`;
- `token_vazio`: unresolved evidence-bearing gaps;
- `claim_allowed`.

## 5. Falsifiability gate

For an empirical/execution claim, `claim_allowed=true` is permitted only when all applicable gates pass:

```text
G = G_type
  & G_domain
  & G_falsifier
  & G_protocol
  & G_metric
  & G_provenance
  & G_evidence
  & G_uncertainty
  & G_negative_control
  & G_reproduction
  & G_token_vazio
```

Where:

- `G_type`: object type is explicit;
- `G_domain`: scope and assumptions are bounded;
- `G_falsifier`: a realistic defeating observation/test exists;
- `G_protocol`: protocol is frozen/versioned before promotion;
- `G_metric`: metric and decision rule are explicit;
- `G_provenance`: data/code/environment are traceable;
- `G_evidence`: outputs/receipts are linked and hashed where material;
- `G_uncertainty`: uncertainty/sensitivity is handled where applicable;
- `G_negative_control`: counterexample/negative control/adversarial path exists where applicable;
- `G_reproduction`: required reproduction level passed;
- `G_token_vazio`: no unresolved *critical* `TOKEN_VAZIO` remains.

If any applicable gate is `FAIL` or critical evidence is `TOKEN_VAZIO`, promotion is blocked.

## 6. Reproduction levels

Do not conflate these levels:

1. `VERIFIED_INTERNAL` — artifact/log/hash is internally consistent.
2. `REPRODUCED_SAME_ENV` — rerun in the same controlled environment.
3. `REPRODUCED_CLEAN_ENV` — clean checkout/environment reproduces result.
4. `REPLICATED_INDEPENDENT` — independent executor/data path reproduces the bounded result.
5. `EXTERNAL_CLAIM_READY` — provenance, uncertainty, limitations and applicable independent replication support the exact public claim.

A local PASS is evidence of local execution, not automatically independent scientific validation.

## 7. Adversarial analytic loop

For every material hypothesis, execute the strongest applicable subset:

```text
baseline
→ positive test
→ negative control
→ boundary test
→ malformed/adversarial input
→ ablation
→ sensitivity analysis
→ counterexample search
→ clean-environment reproduction
→ independent replication
```

Record failures with equal or greater fidelity than passes.

## 8. Multi-domain routing

Before testing, route the claim to its domain adapter:

- mathematics/formal proof;
- software/runtime;
- engineering/physical system;
- statistics/data science;
- cosmology/observational inference;
- biology/biomedicine;
- legal/governance;
- symbolic/parabolic interpretation.

Each adapter MAY add domain-specific gates but MUST NOT remove the core provenance/falsifier/type distinction.

## 9. Append-only longitudinal record

Each evaluation appends an event:

```text
claim_id | timestamp | parent_event_hash | state_before | action |
protocol_version | evidence_hashes | result | state_after | reason
```

Never overwrite a FAIL with a PASS. Append the later PASS and preserve the causal path.

`recurrence_of_form != repetition_of_history`.

## 10. TOKEN_VAZIO rule

When evidence is not yet produced, accessible or discriminating enough:

```text
state=TOKEN_VAZIO
claim_allowed=false
missing=<exact missing object>
next_verifiable_action=<smallest executable test>
```

A useful incomplete answer is valid. An unsupported complete-looking answer is not.

## 11. Stop conditions

Stop promotion and preserve the record when:

- result depends on an unstated convention;
- falsifier cannot be stated;
- data/code origin is unknown;
- only positive examples were tested;
- metric/threshold was chosen after seeing the result without disclosure;
- a critical contradiction is unresolved;
- independent replication is required by the intended claim but absent;
- the same evidence is being counted more than once under different labels.

## 12. Operational output

Every analytical run ends with:

```text
F_ok   = evidence/gates that passed
F_gap  = failures, contradictions and TOKEN_VAZIO
F_next = smallest falsifiable next action
```

The aim is not to make every hypothesis pass. The aim is to make every surviving claim reconstructable, attackable and proportionate to its evidence.
