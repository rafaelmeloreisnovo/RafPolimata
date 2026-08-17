# RAFAELIA Formula Registry 01-20

**Status:** `RESEARCH_GOVERNED`  
**Lifecycle:** `REFERENCE -> FORMALIZED -> METHOD_DEFINED -> SIMULATED -> EVIDENCE_LINKED -> REPLICATED -> CLAIM_ALLOWED`  
**Current claim boundary:** `claim_allowed=false` for the registry as a whole and for every individual record.  
**Canonical scientific contract:** `docs/ORQUESTRADOR_FORMAL_CIENTIFICO.md`  
**T^7 correction dependency:** `docs/closures/CLOSURE_L9_T7_CONVERGENCE.md`

## Purpose

This directory converts formulas F01-F20 from a session-level hypothesis list into a versioned, auditable research object without conflating concept, implementation, execution, evidence or validated claim.

The governing invariant is:

```text
concept != implementation != execution != evidence != validated_claim
```

Missing domain, units, data, provenance, falsifier, replication or physical adapter remains `TOKEN_VAZIO` and blocks promotion.

## Files

- `registry.v1.json` — 20 governed records with family, urgency, epistemic state, dimensional gate, correction and falsifier.
- `reference.py` — stdlib-only executable references for the mathematically scoped corrected subset.
- `../../scripts/validate_formula_registry_01_20.py` — fail-closed registry validator.
- `../../tests/test_formula_registry_01_20.py` — deterministic regression tests.

## Urgency order

### P0 — correctness/claim blockers

`F01 F04 F06 F09 F11 F13 F16`

These contain a type/dimensional/interpretive defect or a probability/convergence issue that must be corrected before stronger claims.

### P1 — executable formalization

`F03 F05 F07 F08 F10 F12 F14 F20`

These have a tractable formal or computational core but still require declared domains, measurements or adapters.

### P2 — semantic/phenomenological research

`F02 F15 F17 F18 F19`

These remain useful as research models, but interpretation must not outrun operational definitions.

## Executable subset

`reference.py` currently implements only corrections that can be exercised without pretending to validate an external physical/biological claim:

- **F01:** seven-dimensional port-Hamiltonian-style drift `(J-G) grad(H)` with antisymmetry check for `J`;
- **F03:** exact coefficients of the truncated odd-derivative series;
- **F07:** normalized 42-component Hilbert-basis amplitude vector, explicitly **not** 42 proved attractors;
- **F08:** exact auxiliary-memory step for `M_t=H-lambda*M` under piecewise-constant `H`;
- **F09:** corrected convergent Fibonacci-Rafael series and closed form;
- **F12:** periodic Fourier character on `T^2`;
- **F16:** bounded probability via a logistic outer map;
- **F20:** separation of discrete decision, calibrated confidence and raw margin.

## Non-regression constraints

1. Do not change the existing T^7 runtime to force F01/F07 claims.
2. Do not relabel F09 original as convergent; its original form remains `CONTRADICTION_ORIGINAL`.
3. F06 remains dimensional `FAIL_AS_WRITTEN` until a consistent theory is supplied.
4. F11 remains `BLOCKED_AS_WRITTEN` until scalar/tensor source typing and conservation are defined.
5. No record may set `claim_allowed=true` without a separate evidence-bearing promotion change.
6. Negative evidence and `TOKEN_VAZIO` are append-only research facts, not cleanup targets.

## Gates

```sh
python3 scripts/validate_formula_registry_01_20.py
python3 -m unittest tests/test_formula_registry_01_20.py -v
```

Expected registry report boundary:

```text
record_count = 20
claim_allowed = false
verdict = PASS
```

`PASS` means the registry contract and deterministic reference tests passed. It does **not** mean the physical, biological, cognitive or ethical hypotheses were experimentally validated.

## Rollback

The subsystem is additive. Rollback is deletion/revert of this directory plus its validator/test/workflow wiring; no persisted runtime schema or existing T^7 transition is changed.

## F_ok / F_gap / F_next

- `F_ok`: governed registry and executable mathematical subset exist.
- `F_gap`: experimental datasets, independent replication and several dimensional/domain adapters remain `TOKEN_VAZIO`.
- `F_next`: run CI on the exact branch commit; only then promote the scoped gate result from `IMPLEMENTED` to `PASS`.
