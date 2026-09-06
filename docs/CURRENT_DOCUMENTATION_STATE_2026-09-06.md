# RafPolimata — Current Documentation State — 2026-09-06

**Repository:** `rafaelmeloreisnovo/RafPolimata`  
**Observed base:** `main@9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
**State:** `CANONICAL_CURRENT_ROUTER / REVIEW_REQUIRED`  
**Scope:** documentation only  
**claim_allowed:** `false`

## Core invariant

```text
concept != implementation != execution != evidence != runtime/device proof
TOKEN_VAZIO != FAIL != PASS
historical snapshot != current generated state
```

## Current source/document surfaces that must be reflected

### Freestanding L0

The current tree contains the OS-agnostic `freestanding/` layer. The canonical first-cut header is documented as `freestanding/include/raf_fs_core.h`.

The source contract explicitly states:

- no allocation/syscall/hosted runtime in the L0 hot core;
- caller-owned `void *state` stages;
- fixed-width primitives without a synthetic source loop shell;
- explicit residual lane count/mask rather than a hidden scalar tail;
- physical runtime remains separately closure-gated.

Recent merged work extends source/codegen coverage across x86_64/i686, ARMv7/AArch64 and RV32/RV64, with fixed/scalable vector and selected matrix-register profiles. Metadata-only coverage for other architectures must not be described as executor/runtime proof.

### PBIP-L1

The current evidence tree goes beyond the first Vectras receipt. A cross-implementation reproduction record now compares:

- Vectras Java/Gradle-JDK21;
- Rafaelia_Private C11 freestanding/Clang 18.1.3.

For the three canonical vectors, the record states equal numerical vectors/classification and closes repository, implementation, language and toolchain independence at the bounded implementation/reproduction layer.

It explicitly leaves open:

```text
TOKEN_VAZIO_PBIP_PROVIDER_INDEPENDENT_REPRODUCTION
TOKEN_VAZIO_PBIP_DEVICE_PROOF
TOKEN_VAZIO_PBIP_ANDROID_RUNTIME
claim_allowed=false
```

Therefore `docs/evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md` must describe cross-implementation reproduction as proven while preserving provider/device/runtime gaps.

### Urgency/gate/gap snapshot

`docs/URGENCY_GATE_GAP_20260906.md` is an append-only audit snapshot bound to source revision `820e7ea29bebbf0154ea245d401f691715fea5d8` and `CLOSURE_L1`.

It is useful evidence but must not be rewritten into a later revision. The current base includes the corrective merge; current navigation should point to the snapshot plus this current-state router.

## Documentation governance state

The canonical policy in `docs/DOCUMENT_GOVERNANCE.md` defines generated outputs as derived from script + policy + commit and forbids treating index presence as execution proof.

The currently committed generated summary is stale:

```text
docs/generated/DOCUMENT_GOVERNANCE_INDEX.md
  source commit = ff000ab0a38b7ca9ae672300f1534915dcf0e4fe
  state         = REVIEW_REQUIRED
  files         = 1356 governed records at that historical cut
  relations     = 1265
  review_queue  = 841
```

`results/document-governance/summary.json` carries the same historical commit. At the observed current base, `results/document-governance/catalog.jsonl` was read as empty while the historical generated summary reports a populated catalog. That inconsistency means the generated governance plane is **not current-proof**.

Because this transaction is docs-only and generated files must be produced by `scripts/document_governance.py`, no generated output is manually edited.

Current classification:

```text
generated governance regeneration = TOKEN_VAZIO_REGEN_REQUIRED
current strict governance PASS     = not claimed
state                               = REVIEW_REQUIRED
```

## Canonical reading order

1. `README.md` — compact repository router;
2. `docs/CURRENT_DOCUMENTATION_STATE_2026-09-06.md` — current docs/evidence cut;
3. `docs/INDEX.md` — curated thematic index;
4. `docs/AGENTES.md` — operational invariants;
5. `docs/DOCUMENT_GOVERNANCE.md` — lifecycle/generator contract;
6. current subsystem documents;
7. `docs/URGENCY_GATE_GAP_20260906.md` — dated audit snapshot;
8. generated outputs only when their commit matches the revision being assessed;
9. historical receipts/evidence according to their exact revision.

## Current documentation priorities

1. keep canonical routers aligned with exact `main` source;
2. route freestanding L0 and PBIP cross-implementation state;
3. regenerate document-governance outputs through their generator, never by hand;
4. preserve historical snapshots/receipts append-only;
5. maintain provider/device/scientific gaps as `TOKEN_VAZIO` until separately closed.

## R3

- **F_ok:** current source/evidence cut is now explicit; freestanding and PBIP state are routed; generated snapshot staleness is named instead of hidden.
- **F_gap:** current document-governance regeneration and physical/provider-independent evidence remain open.
- **F_next:** execute the repository's own document-governance generator against the exact current revision, then review its queue without promoting mere indexability into proof.