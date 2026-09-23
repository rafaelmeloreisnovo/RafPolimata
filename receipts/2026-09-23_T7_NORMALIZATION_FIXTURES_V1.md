# Receipt — T7 Normalization Registry + Migration Fixtures V1

- parent_main: `3c888145b98a6f7f179fc7ddd1aa3b0f2dbfc0b0`
- kind: T7/NORMALIZATION/PRE-MIGRATION
- governance_closure: `CLOSURE_L9`
- claim_allowed: false
- runtime_migration_authorized: false

## Material delta

- defines normalization ID 1 only: pre-normalized Q16 identity, `q_out=q_in&0xFFFF`;
- preserves semantic presence separately through `present_mask`;
- keeps external source→semantic-axis calibration as `TOKEN_VAZIO`;
- adds three deterministic migration fixtures;
- binds fixture values to the real legacy formulas and to the existing `Pi_wf_V1` equality-mask semantics;
- does not modify `Benchmark/raf_toroid.h::t7_map_input`.

## Reference fixture

`data_hash=0x12345678`, `entropy=0x00001234`, `hw_state=0x89ABCDEF`.

Expected legacy raw coordinates:

`[22136,4660,4660,0,52719,43981,39831]`

Expected state after canonical `t7_init` followed by one legacy `t7_map_input`:

`[35912,12768,43145,23205,17610,45803,25990]`

Expected semantic equality masks:

- identity/all-present: `127`;
- u differs by one: `126`;
- chi absent while its numeric slot remains zero: `119`.

## Boundary

A normalization registry entry is not evidence that a real external observable has been scientifically calibrated to a semantic axis. ID 1 accepts values that are already normalized to the Q16 grid. Any external metric→axis rule requires its own source, units, transform, uncertainty model and falsifier before becoming a new registry entry.

## Gate

- `python3 scripts/validate_t7_migration_fixtures_v1.py`
- existing hosted Pi_wf C regression
- existing freestanding x86-64/ARMv7/AArch64 syntax matrix
- exact-head repository workflows

## R3

F_ok: deterministic normalization/fixture contract materialized without runtime migration.  
F_gap: external metric→axis normalizers; runtime migration policy/compatibility decision; stronger dynamical proof.  
F_next: exact-head CI; only after PASS may a Pi_wf_V2/application bridge be proposed.
