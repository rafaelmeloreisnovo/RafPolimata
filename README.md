# RafPolimata

**State:** `CANONICAL / REVIEW_REQUIRED`  
**Observed documentation base:** `main@9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
**Documentation cut:** 2026-09-06  
**claim_allowed:** `false`

RafPolimata is a semantic, mathematical, low-level engineering, evidence-governance and research repository. Its core discipline is to keep concepts, implementation and proof levels separate:

```text
concept != implementation != execution != evidence != runtime/device proof
TOKEN_VAZIO != FAIL != PASS
```

## Canonical reading order

1. [`docs/CURRENT_DOCUMENTATION_STATE_2026-09-06.md`](docs/CURRENT_DOCUMENTATION_STATE_2026-09-06.md) — current source/document/evidence cut;
2. [`docs/INDEX.md`](docs/INDEX.md) — curated canonical index;
3. [`docs/AGENTES.md`](docs/AGENTES.md) — operational invariants;
4. [`docs/DOCUMENT_GOVERNANCE.md`](docs/DOCUMENT_GOVERNANCE.md) — lifecycle, evidence grades and generated-state contract;
5. subsystem document;
6. exact source/test/evidence bound to the assessed revision.

Dated audit/evidence snapshots retain their original revision meaning and are not silently rewritten into current state.

## Current source domains

The repository currently spans, among other domains:

- mathematical/formal models and falsifiability routes;
- ApkC and low-level Android/DEX/ELF/ZIP work;
- OS-agnostic freestanding L0 plus a separated optional syscall layer;
- multi-architecture ABI/vector/codegen profiles;
- conversation/indexing and data-governance surfaces;
- scientific bibliography/acquisition and evidence orchestration;
- risk, legal/license, provenance and operational-governance documentation;
- PBIP-L1 implementation/evidence federation.

Source presence never upgrades runtime/device/scientific claims automatically.

## Freestanding L0

The current source tree contains `freestanding/` with `freestanding/include/raf_fs_core.h` documented as the canonical first-cut core.

Its source contract keeps the hot core OS-agnostic and records:

- no allocator/syscall/hosted-runtime dependency in L0;
- caller-owned state;
- fixed-width primitives without a synthetic source-loop shell;
- explicit residual lane geometry rather than a hidden scalar cleanup tail;
- physical runtime as a separate evidence gate.

Recent merged work extends source/codegen profiles across major x86, ARM and RISC-V targets and includes selected SIMD/scalable-vector/matrix-register routes. A compiled metadata profile is not an executor claim; codegen is not physical runtime.

## PBIP-L1

The PBIP evidence chain now includes two implementation families for the same three canonical vectors:

- Vectras-VM-Android — Java / Gradle-JDK21;
- Rafaelia_Private — C11 freestanding / Clang 18.1.3.

The committed comparison record states equal canonical vectors/classifications and bounded independence across repository, implementation, language and toolchain family.

Still open:

```text
TOKEN_VAZIO_PBIP_PROVIDER_INDEPENDENT_REPRODUCTION
TOKEN_VAZIO_PBIP_ANDROID_RUNTIME
TOKEN_VAZIO_PBIP_DEVICE_PROOF
claim_allowed=false
```

See [`docs/evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md`](docs/evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md).

## ApkC and runtime boundaries

ApkC contains experimental/engineering source for direct artifact construction and Android-oriented validation. Current-artifact provenance, signing and physical execution must remain one coherent revision/artifact chain before any release/runtime promotion.

Historic/local build evidence remains useful only within its exact scope.

## Documentation governance

`docs/DOCUMENT_GOVERNANCE.md` defines the repository's six-level governance model:

```text
L0 physical structure
L1 file identity
L2 reference graph
L3 area / owner / temporal governance
L4 evidence / duplication / risk
L5 review queue / promotion
```

Generated outputs are derived artifacts. They must be regenerated from script + policy + commit and must not be hand-edited to create apparent freshness.

### Current generated-state warning

The committed `docs/generated/DOCUMENT_GOVERNANCE_INDEX.md` and `results/document-governance/summary.json` are bound to historical commit `ff000ab0a38b7ca9ae672300f1534915dcf0e4fe` and report:

```text
state        = REVIEW_REQUIRED
files        = 1356 governed records at that historical cut
relations    = 1265
review_queue = 841
blockers     = 0
```

At current base, `results/document-governance/catalog.jsonl` was observed empty while that historical summary reports a populated catalog. Therefore the current generated plane is classified:

```text
TOKEN_VAZIO_REGEN_REQUIRED
```

No generated file was manually rewritten in this docs-only audit.

## Evidence states

| State | Meaning |
|---|---|
| `CANONICAL` | official governance entry |
| `ACTIVE` | in-use source/document |
| `REFERENCE` | explanation/specification |
| `AUDIT` | decision/provenance trail |
| `EVIDENCE` | result tied to command/test/receipt |
| `GENERATED` | derived; regenerate, do not hand-edit |
| `PENDING` | content exists without sufficient gate |
| `TOKEN_VAZIO` | evidence absent or insufficient |

## Current high-priority gaps

1. regenerate the documentation-governance outputs against exact current main;
2. close current-artifact ApkC provenance where claimed;
3. close physical runtime/device gates with same-artifact receipts;
4. obtain independent-provider PBIP reproduction before provider-independence claims;
5. keep metadata/codegen/implementation/scientific novelty levels separate;
6. reconcile license/provider metadata through explicit provenance review rather than inference.

See [`docs/URGENCY_GATE_GAP_20260906.md`](docs/URGENCY_GATE_GAP_20260906.md) for the dated append-only audit snapshot and [`docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md`](docs/ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md) for the current documentation/evidence roadmap.

## Generated governance command

The documented canonical route remains:

```sh
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

This README does not claim those commands were executed on `9ed0b8aa...` during this documentation-only transaction.

## Legal and scientific boundary

Repository documentation does not substitute for legal advice, external security certification, scientific peer review or independent reproduction. Public visibility does not itself settle license/attribution rights.

## Documentation audit receipt

The provenance record for this reconciliation is:

[`docs/DOCUMENTATION_AUDIT_RECEIPT_2026-09-06.md`](docs/DOCUMENTATION_AUDIT_RECEIPT_2026-09-06.md)

## R3

- **F_ok:** README now routes the current freestanding/PBIP/governance state without promoting stale generated outputs.
- **F_gap:** current generator refresh, physical/device evidence and provider-independent reproduction remain open.
- **F_next:** regenerate derived governance through its own tool, then close runtime/scientific gates independently with revision-bound receipts.