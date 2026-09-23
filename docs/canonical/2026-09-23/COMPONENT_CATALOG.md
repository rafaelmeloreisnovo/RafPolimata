# Component catalog — RafPolimata

**Governance binding: CLOSURE_L11** — gaps and unknown-state markers are governed by the operational topology; this does not promote evidence.  
**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92

This catalog is a navigation layer. It does not replace each subsystem's own README, AGENTS or protocol.

## Core engineering

| Component | Canonical/primary entry | Role |
|---|---|---|
| Compiler high-level pipeline | raf_compile.h + raf_main.c | language/CPU/flag pipeline and output orchestration |
| ApkC | Apkc/apkc.c + Apkc/PROTOCOL.md | Android artifact construction and language dispatch |
| Language profiles | Apkc/lang_profile.h | declarative frontend dispatch |
| Freestanding L0 | freestanding/include/raf_fs_core.h | OS-neutral CPU/register/memory primitives |
| Syscall layer | syscall/ | optional Linux ABI bindings kept outside L0 |
| Semantic IR | scripts/raf_semantic_ir.py | canonical computational semantics |
| Architecture registry | compiler/architectures.v2.json | seven-active-architecture policy |
| Compiler target registry | compiler/targets.v1.json | reproducible target/build routes |
| Runtime router | Benchmark/raf_runtime_router.h | capability-based runtime selection |
| Verbovivo / T7 | rafaelia/verbovivo.c | research/runtime semantic pipeline with separate evidence gates |

## Data, indexing and knowledge

| Component | Entry | Role |
|---|---|---|
| Conversation indexer | runtime/conversation_indexer/ | versioned records/codecs and indexing runtime |
| Data | data/ | fixtures, registries, receipts and datasets by subsystem |
| Knowledge base | knowledge_base/ | generated/research knowledge outputs; generated rules apply |
| Research | research/ | formal/research cores and registries |
| Experiments | experiments/ | bounded experiments, not automatically production paths |
| Schemas | schemas/ | machine-readable contracts |

## Quality and evidence

| Region | Role |
|---|---|
| tests/ | positive, negative, regression and falsifier tests |
| scripts/ | build/audit/validation/orchestration |
| ci/ | contracts, profiles and reports consumed by CI |
| evidence/ | evidence objects and federated proof material |
| proofs/ | proof-oriented artifacts by subsystem |
| receipts/ and docs/receipts/ | append-only execution/decision records |
| results/ | derived execution outputs; provenance required |
| auditoria/ | audit-specific records |

## Packaging and integration

| Region | Role |
|---|---|
| native/ | self-contained native modules, including RAF Hash Fabric |
| packages/ | educational/industrial packaging surfaces |
| termux/ | Termux-oriented integration |
| configs/ | versioned policies/configuration |
| contracts/ | cross-component contracts |
| manifests/ | artifact/process manifests |
| federation/ | federation/integration material |
| network_guard/ | network/security controls where applicable |

## Documentation and governance

| Entry | Role |
|---|---|
| README.md | root router |
| AGENTS.md | common operational entry |
| docs/AGENTES.md | detailed unified protocol |
| docs/INDEX.md | curated documentation index |
| docs/DOCUMENT_GOVERNANCE.md | L0–L5 document governance |
| docs/canonical/ | revision-bound current documentation cuts |
| .github/ | collaboration, workflows and GitHub-native policy surfaces |

## Ownership rule

A directory name is not enough to determine authority. When a subsystem has a local AGENTS.md, protocol, schema or Makefile, that closer contract governs implementation details unless a higher-priority human instruction overrides it.

## Evidence rule

Component presence grants, at most, SOURCE_PRESENT/IMPLEMENTED status appropriate to that source. Build, execution, device, reproduction and claim eligibility remain separate.

R3 = ⟨F_ok: major repo domains routed to primary entries; F_gap: not every leaf file receives a human catalog row because document-governance already inventories files; F_next: use this component layer for navigation and the generated governance catalog for file-level reachability⟩.
