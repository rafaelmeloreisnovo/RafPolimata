# Runtime Doctor Ω — Coherence V2 Index — 2026-08-14

State: `VERIFIED_LIMITED_REFERENCE / claim_allowed=false`

## Delta

1. Correct Python `false`/`False` defect in `build_report()` claim boundary.
2. Deduplicate identical read-only probes by `(repository root, exact command)` and preserve fan-out provenance.
3. Expose downstream reachable `route_graph_used` from selected skill seeds.
4. Ingest Ecosystem Build Doctor JSON reports by schema gate + exact SHA-256 binding.
5. Materialize structured `gap_ledger` with urgency, state, provenance and `F_next`.
6. Extend the report schema non-destructively with cache/evidence/gap fields.
7. Add regression tests for end-to-end `build_report()`, cache, graph, Build Doctor evidence and Frida P0 gap.

## Reference evidence

- Runtime Doctor implementation commit: `777424dc980953fcaf386a26d625de9966365394`
- Runtime Doctor Git blob: `b04c6d478c0e39127a84ff0ada7292695a2e75d0`
- Runtime Doctor SHA-256 in reference runtime: `3c47efe81d710b3e8e355bf1ee276d40d654d6c16f30cbe5b967630fac017dbc`
- Tests commit: `f94e7e8a33a898cc3a7c7244ff10440b68f9d035`
- Contract commit: `06b4db1466f902bf0d6464e6c3065c964d4f3027`
- Reference receipt commit: `2352ffebc44e90e243602cb6ce256e4cc977b28a`
- Gap ledger commit: `543041baf90e9952316cbeb008e5752ee90d23e8`
- Frontier-count errata commit: `c6f3c957025bfc74416af0affb113020ae85557d`

## Reference harness

Environment: Debian 13 x86_64 / Python 3.13.5, not Android/Termux.

Result: `PASS` on 11 structural checks against a byte-identical Runtime Doctor source blob.

Observed summary:

```text
selected_skills=5
probe_results=5
probes_executed=1
probe_cache_hits=1
failed_probes=0
incomplete_probes=3
runtime_routes=4
build_doctor_reports=1
open_gaps=6
state=PASS_LIMITED
```

`PASS_LIMITED` is intentional: three selected skills had no runtime probe in the synthetic harness. Partial evidence is not promoted to full runtime execution.

## Negative evidence

Fresh Git clone in the reference runtime failed with DNS resolution failure for `github.com`. Therefore full repository pytest remains `TOKEN_VAZIO`; the reference harness is not mislabeled as the repository test suite.

## Current frontier

Apply `data/governance/runtime-doctor-gap-ledger.v2.errata-20260814.json` to the v2 frontier counts:

- closed reference: 4
- open/external/unknown: 9
- P0: 3
- P1: 3
- P2: 3

P0 order:

1. full repository/provider pytest;
2. physical Termux Runtime Doctor receipt;
3. physical Frida readiness receipt.

## Reconstruction route

`Master Navigation Registry -> Runtime Doctor base -> Frida L2.5 -> Coherence V2 index -> exact commits/blobs -> reference receipt -> gap ledger+errata -> current F_next`

## Invariants

- `MERGED != TESTED`
- `REFERENCE_HARNESS != FULL_REPOSITORY_TEST_SUITE`
- `CACHE_REUSE != SECOND_EXECUTION`
- `STATIC_EVIDENCE_INPUT != CURRENT_BUILD_EXECUTION`
- `CONFIGURATION != PHYSICAL_RUNTIME`
- `TOKEN_VAZIO` remains open until direct evidence closes it.

## F_ok

Four structural gaps are closed at reference scope and machine-readable urgency/provenance routing now exists.

## F_gap

Repository/provider pytest and physical Android/Termux/Frida evidence remain open; LLaMA and QEMU evidence remain downstream.

## F_next

Trigger/observe repository tests from the isolated branch; if provider execution is unavailable, preserve that blocker. Then move to the physical Termux read-only receipt without merging unexecuted claims.
