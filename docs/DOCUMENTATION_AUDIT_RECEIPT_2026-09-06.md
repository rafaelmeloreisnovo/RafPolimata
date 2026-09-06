# RafPolimata — Documentation Audit Receipt — 2026-09-06

**Receipt class:** `DOCS_ONLY_PROVENANCE`  
**Repository:** `rafaelmeloreisnovo/RafPolimata`  
**Base revision:** `9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
**Working branch:** `docs/documentation-audit-2026-09-06`  
**Policy:** `SOURCE_OBSERVED → DOC_ALIGNED → CROSSCHECKED → RECEIPTED`  
**Code mutation authorized:** `false`  
**Claim promotion authorized:** `false`

## Purpose

Reconcile canonical/current documentation with the actual 2026-09-06 source/evidence tree while preserving historical audit snapshots and refusing to hand-edit generated governance outputs.

## Source/evidence observations used

### Freestanding L0

Observed source:

- `freestanding/CANONICAL.md` identifies `freestanding/include/raf_fs_core.h` as the canonical first-cut core;
- `freestanding/include/raf_fs_core.h` declares no allocation/syscall/hosted runtime in its hot-core scope, caller-owned state, fixed-width primitives, explicit residual state and separately gated physical runtime evidence.

Recent main lineage also contains broader ABI/vector/codegen work. Documentation therefore distinguishes source/codegen/metadata from runtime/device proof.

### PBIP-L1

Observed:

- `docs/evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md` still described reproduction as pending;
- `evidence/pbip/PBIP_L1_CROSS_IMPLEMENTATION_REPRODUCTION_20260906.v1.json` records two implementations:
  - Vectras Java / Gradle-JDK21;
  - Rafaelia_Private C11 freestanding / Clang 18.1.3;
- canonical vectors/classifications match;
- repository, implementation, language and toolchain-family independence are true in the bounded comparison;
- CI-provider independence, Android runtime and device independence remain false/open;
- `claim_allowed=false`.

The PBIP human route was therefore updated from `REPRODUCTION_PENDING` to bounded `CROSS_IMPLEMENTATION_REPRODUCTION_PROVEN` while preserving the remaining `TOKEN_VAZIO` dimensions.

### Documentation governance

The canonical policy `docs/DOCUMENT_GOVERNANCE.md` states that generated outputs derive from executor + policy + commit and should be regenerated, not manually edited.

Observed committed generated snapshot:

```text
source commit = ff000ab0a38b7ca9ae672300f1534915dcf0e4fe
state         = REVIEW_REQUIRED
files         = 1356 governed records at that historical cut
relations     = 1265
review_queue  = 841
blockers      = 0
```

Observed at current base:

- `results/document-governance/summary.json` still references `ff000ab...`;
- `docs/generated/DOCUMENT_GOVERNANCE_INDEX.md` still references `ff000ab...`;
- `results/document-governance/catalog.jsonl` was read as empty.

This is not sufficient for current-main governance PASS.

```text
current generated governance state = TOKEN_VAZIO_REGEN_REQUIRED
manual generated-file repair       = forbidden by this transaction
```

### Urgency snapshot

`docs/URGENCY_GATE_GAP_20260906.md` is an append-only snapshot bound to source revision `820e7ea29bebbf0154ea245d401f691715fea5d8` and `CLOSURE_L1`. It already records current-day gaps including physical execution, PBIP provider/device boundaries, freestanding gaps and documentation-governance final-head revalidation.

It was preserved as a dated snapshot rather than rewritten to another source revision.

## Documentation changes

Only documentation was changed/created:

- `README.md` — current canonical router;
- `docs/CURRENT_DOCUMENTATION_STATE_2026-09-06.md` — current source/docs/evidence cut;
- `docs/INDEX.md` — routes freestanding L0, PBIP and generated-state warning;
- `docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md` — evolved evidence-aware documentation roadmap;
- `docs/evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md` — reconciled with cross-implementation evidence;
- this receipt.

Preserved unchanged:

- production/source code;
- tests;
- workflows;
- configs/schemas;
- evidence JSON receipts;
- generated governance outputs;
- dated urgency snapshot;
- historical documents.

## Change commits before this receipt

```text
227400ff255e4672fa83ee47dcccca4577cdec72  current documentation state
3ab6e2f67c7ecaff8ff7a61a3fe912b6b62e0ca9  canonical index
 d5757662866f147a8d39b38ffc2a6121c7283525 roadmap
8ebf6d0270a7cbc4ae8e42caac0d2b9fef187598  PBIP human evidence route
59c1b9c9c9d4c30d4f314c404aa9195cc5ecaf4d  README router
```

The receipt creation commit necessarily follows these entries and is not self-referentially embedded in its body.

## Non-promotions / TOKEN_VAZIO

```text
current document-governance regeneration = TOKEN_VAZIO_REGEN_REQUIRED
strict current governance PASS            = not claimed
physical freestanding/runtime proof       = TOKEN_VAZIO
current-artifact ApkC provenance          = TOKEN_VAZIO where recorded
PBIP independent-provider reproduction    = TOKEN_VAZIO
PBIP Android runtime                      = TOKEN_VAZIO
PBIP physical device                      = TOKEN_VAZIO
scientific novelty/peer review             = TOKEN_VAZIO where not independently established
claim_allowed                              = false
```

## Acceptance criteria

- final diff contains documentation only;
- generated outputs are not manually rewritten;
- historical snapshots retain their revision identity;
- PBIP documentation matches the committed comparison receipt without overclaim;
- freestanding documentation separates source/codegen from runtime/device;
- stale generated state is explicitly labeled, not silently promoted;
- final branch-vs-base diff is inspected before delivery.

## R3

- **F_ok:** current canonical routing, PBIP evidence state and freestanding source boundary are reconciled with explicit provenance.
- **F_gap:** generated governance refresh and runtime/device/provider-independent evidence require separate executable gates.
- **F_next:** inspect the final changed paths, open a docs-only PR and require the repository generator for the next generated-state transition.