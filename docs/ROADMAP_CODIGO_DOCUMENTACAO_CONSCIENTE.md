# Roadmap — Código & Documentação Consciente

> **Current base:** `main@9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
> **Updated:** 2026-09-06  
> **Invariant:** `SOURCE != EXECUTION != EVIDENCE != RUNTIME/DEVICE`; `TOKEN_VAZIO != PASS`.

## Goal

Maintain source, technical explanation, scientific/evidence boundaries and audit navigation as one coherent system without rewriting historical evidence or manually falsifying generated state.

## Phase A — Current canonical routing

**State:** `ACTIVE`.

1. Keep `README.md`, `CURRENT_DOCUMENTATION_STATE_2026-09-06.md` and `docs/INDEX.md` aligned with exact main source.
2. Route newly material source domains in the same cycle.
3. Preserve dated audit/evidence files as snapshots rather than overwriting their source revision.
4. Use `TOKEN_VAZIO_DOC_ROUTE` when a source domain exists but its canonical documentation owner is not yet proven.

Current required routing includes freestanding L0 and PBIP-L1 cross-implementation evidence.

## Phase B — Generated governance regeneration

**State:** `TOKEN_VAZIO_REGEN_REQUIRED`.

The committed generated governance plane is historical (`ff000ab...`) and its summary does not represent the current `9ed0b8aa...` cut.

Required gate:

```text
exact checkout
→ scripts/document_governance.py --write
→ tests
→ --check
→ generated index + review queue + catalog + relations
→ commit-bound receipt
```

Generated files must not be edited manually to remove drift.

## Phase C — Freestanding documentation coherence

**State:** `SOURCE_ADVANCED / RUNTIME_GATED`.

Maintain explicit separation:

```text
OS-agnostic L0
!= optional syscall layer
!= target codegen evidence
!= physical execution
```

Documentation must distinguish:

- implemented executors;
- codegen-only gates;
- metadata-only architecture profiles;
- matrix-register primitives vs complete matrix datapaths;
- implementation gaps under their closure IDs;
- physical/runtime gaps under their own closures.

No architecture may be promoted from metadata compile to executor/runtime status by prose.

## Phase D — PBIP evidence ladder

**State:** `CROSS_IMPLEMENTATION_REPRODUCED / PROVIDER_DEVICE_PENDING`.

Current bounded evidence establishes matching canonical vectors/classification across two implementations/languages/toolchain families. Remaining steps are independent dimensions:

1. independent CI/provider reproduction;
2. Android runtime if claimed;
3. physical device evidence if claimed;
4. wider mathematical/scientific claims only through their own falsifiers/review.

```text
cross implementation != provider independence != device proof != scientific novelty
```

## Phase E — ApkC/current-artifact provenance

**State:** `TOKEN_VAZIO_CURRENT_ARTIFACT_CHAIN`.

Build one coherent chain:

```text
source revision
→ compiler/toolchain identity
→ ELF/DEX/ZIP/APK outputs
→ hashes/format checks
→ signing identity
→ physical install/launch if claimed
```

Historical format/build evidence remains useful but does not automatically cover a new current artifact.

## Phase F — Scientific/documentary reproducibility

**State:** domain-specific.

For every formal/scientific claim, separate:

- equation/definition;
- executable implementation;
- deterministic vectors;
- dataset/input identity;
- provider/run receipt;
- independent reproduction;
- literature/citation state;
- novelty/review state.

`engineering PASS != scientific novelty`.

## Phase G — Release/legal/governance maturity

1. Reconcile root/license metadata through explicit provenance/attribution review.
2. Keep legal/service documents scoped; no README text creates certification or signed commercial terms.
3. Require revision-bound provider governance readback where protection/enforcement is claimed.
4. Keep physical/device/release claims closed until same-artifact evidence exists.

## Metrics

Use metrics only with a declared revision and denominator:

- **documentation routing coverage** = source domains with current canonical route / audited source domains;
- **generated-state freshness** = generated outputs matching assessed commit / required generated outputs;
- **evidence-link coverage** = claims with exact source/run/artifact pointers / claims audited;
- **TOKEN_VAZIO closure rate** = correctly closed gaps / explicitly tracked gaps;
- **historical preservation rate** = snapshots preserved without silent rewrite / audited snapshots.

An unexecuted denominator is `TOKEN_VAZIO`, not an estimated percentage.

## Permanent no-regression rules

- generated artifact → regenerate, do not hand-edit;
- historical receipt → preserve exact meaning;
- new source domain → add route or `TOKEN_VAZIO_DOC_ROUTE`;
- codegen → never describe as device runtime;
- cross-implementation reproduction → never describe as provider/device independence unless separately proven;
- documentation PASS → never imply scientific/legal/security certification.

## R3

- **F_ok:** roadmap now reflects the actual 2026-09-06 freestanding/PBIP/governance landscape.
- **F_gap:** generated governance refresh, current-artifact ApkC chain, provider/device evidence and independent review remain open by domain.
- **F_next:** close generator freshness first on the documentation plane, while execution teams close physical/scientific gates independently.