# Índice Canônico de Documentação — RafPolimata

> **Observed base:** `main@9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
> **Estado:** `CANONICAL`
> **Evidence-gap governance:** `CLOSURE_L2`  
> **Regra:** `README → CURRENT_DOCUMENTATION_STATE → INDEX → área → source/evidence`


## 0A. Suplemento de maturidade corrente — 2026-09-28

**Observed main base:** f840fde2de65edacf8b2abbee8ca9d1f9c912b6d  
**Predecessor review:** PR#358@f35fc8402ea790a8dead96817c0847bbaf9c1a98

| Rota | Papel |
|---|---|
| [2026-09-28/README](canonical/2026-09-28/README.md) | router do suplemento |
| [MATURITY_AND_PRODUCTIZATION_V1](canonical/2026-09-28/MATURITY_AND_PRODUCTIZATION_V1.md) | oito superfícies de maturidade |
| [THREAT_MODEL_RAFPOLIMATA_V1](security/THREAT_MODEL_RAFPOLIMATA_V1.md) | ameaças, fronteiras e não-objetivos |
| ../contracts/rafpolimata_api_abi_v1.json | API 0.1.0 / ABI 1 |
| ../sdk/rafpolimata_v1/ | primeiro slice de SDK estático |

O workflow de maturidade executa o mesmo bundle em checkout limpo de Ubuntu 22.04 e 24.04. Isso é reprodução cross-environment, não reprodução independente de provider.

## 0. Camada documental ampla — 2026-09-23

**Implementation base observada:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Router estável:** [canonical/README.md](canonical/README.md)

| Rota | Papel |
|---|---|
| [canonical/2026-09-23/README.md](canonical/2026-09-23/README.md) | corte corrente e fontes |
| [REPOSITORY_SURVEY](canonical/2026-09-23/REPOSITORY_SURVEY.md) | inventário físico e GitHub |
| [ARCHITECTURE](canonical/2026-09-23/ARCHITECTURE.md) | arquitetura e fronteiras |
| [LANGUAGES](canonical/2026-09-23/LANGUAGES.md) | contrato de linguagem vs fonte observada |
| [BUILD_TEST_EVIDENCE](canonical/2026-09-23/BUILD_TEST_EVIDENCE.md) | build, gates e estado do CI |
| [WORKFLOW_CATALOG](canonical/2026-09-23/WORKFLOW_CATALOG.md) | 50 workflows |
| [GITHUB_SURFACES](canonical/2026-09-23/GITHUB_SURFACES.md) | recursos GitHub e gaps |
| [EVIDENCE_AND_CLAIMS](canonical/2026-09-23/EVIDENCE_AND_CLAIMS.md) | níveis de prova |
| [DOCUMENTATION_STYLE](canonical/2026-09-23/DOCUMENTATION_STYLE.md) | padrão documental |
| [COMPONENT_CATALOG](canonical/2026-09-23/COMPONENT_CATALOG.md) | componentes e entradas principais |
| [RELEASE_VERSIONING](canonical/2026-09-23/RELEASE_VERSIONING.md) | releases/versionamento/histórico |
| [RUNTIME_AND_TARGETS](canonical/2026-09-23/RUNTIME_AND_TARGETS.md) | estado temporal, targets e arquiteturas |
| [SECURITY_SUPPLY_CHAIN](canonical/2026-09-23/SECURITY_SUPPLY_CHAIN.md) | segurança, licença e supply chain |
| [GLOSSARY](canonical/2026-09-23/GLOSSARY.md) | vocabulário comum |
| [GAPS_AND_NEXT](canonical/2026-09-23/GAPS_AND_NEXT.md) | backlog tipado |

Os snapshots e receipts anteriores permanecem válidos no seu escopo histórico; não são reescritos silenciosamente.


## 1. Entrada canônica corrente

| Documento | Papel | Estado |
|---|---|---|
| [`../README.md`](../README.md) | router curto do repositório | `CANONICAL` |
| [`CURRENT_DOCUMENTATION_STATE_2026-09-06.md`](CURRENT_DOCUMENTATION_STATE_2026-09-06.md) | corte atual source↔docs↔evidence | `CANONICAL_CURRENT_ROUTER` |
| [`AGENTES.md`](AGENTES.md) | invariantes operacionais | `CANONICAL` |
| [`DOCUMENT_GOVERNANCE.md`](DOCUMENT_GOVERNANCE.md) | identidade, lifecycle, risco, fila e geração | `CANONICAL` |
| [`MAPA_ESTRUTURAL_REPOSITORIO.md`](MAPA_ESTRUTURAL_REPOSITORIO.md) | disposição estrutural | `CANONICAL` |
| [`URGENCY_GATE_GAP_20260906.md`](URGENCY_GATE_GAP_20260906.md) | snapshot append-only de gates/gaps | `AUDIT` |
| [`DOCUMENTATION_AUDIT_RECEIPT_2026-09-06.md`](DOCUMENTATION_AUDIT_RECEIPT_2026-09-06.md) | proveniência da reconciliação docs-only | `AUDIT` |

```text
indexado != validado
source != execution != evidence != runtime/device proof
TOKEN_VAZIO != FAIL != PASS
```

## 2. Freestanding L0 / arquitetura de baixo nível

| Documento/rota | Papel | Limite |
|---|---|---|
| [`../freestanding/CANONICAL.md`](../freestanding/CANONICAL.md) | aponta o first-cut core canônico | source contract |
| `freestanding/include/raf_fs_core.h` | header L0 canônico observado em fonte | physical runtime separado |
| [`ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md`](ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md) | roadmap docs↔source e gates atuais | não é receipt runtime |
| `freestanding/AGENTS.md` | contrato de manutenção L0 | escopo freestanding |
| `syscall/AGENTS.md` | fronteira separada de syscall/OS ABI | não fundir com L0 |

Estado editorial corrente: source/codegen avançou para múltiplos perfis de ABI/vetor, mas metadata-only não vira executor e codegen não vira device proof.

## 3. PBIP-L1 / evidência formal e reprodução

| Documento/artefato | Papel | Estado bounded |
|---|---|---|
| [`evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md`](evidence/PBIP_L1_EVIDENCE_ROUTE_V1.md) | rota humana de evidência | `CROSS_IMPLEMENTATION_REPRODUCED / PROVIDER_DEVICE_PENDING` após reconciliação |
| `../evidence/pbip/PBIP_L1_CI_RECEIPT_20260906_RUN34025698586.v1.json` | primeiro receipt Vectras executado | provider-bound |
| `../evidence/pbip/PBIP_L1_CROSS_IMPLEMENTATION_REPRODUCTION_20260906.v1.json` | comparação Java vs C11 freestanding | cross-implementation reproduction proven; provider/device open |
| [`URGENCY_GATE_GAP_20260906.md`](URGENCY_GATE_GAP_20260906.md) | fechamento e gaps atuais do snapshot | `claim_allowed=false` |

Não promover `provider_independent_reproduction`, Android runtime ou device proof sem receipt próprio.

## 4. Arquitetura, metodologia e linguagem

- `ARQUITETURA_21_NIVEIS.md`
- `RAFAELIA_MULTIFILAMENT_EXECUTION_CONTRACT_V1.md`
- `LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01.md`
- `LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01_ERRATA_V1_1.md`
- `LINGUAGEM/COERENCIA_COMMIT_FRACTAL_OMEGA.md`
- `LINGUAGEM/ATLAS_ARCOS_FLUXOS_LIVROS_ATOS_CRENCAS_MATEMATICA.md`
- `DEZ_DIMENSOES_SEMANTICAS.md`
- `CONVERGENCIA_UNICA_METODOLOGICA.md`
- `CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md`
- `INFO_DYNAMICS_INTERNAL_TRACEABILITY.md`

## 5. Excelência operacional, risco e evidência

- `RISCO_GESTAO_FRAMEWORK_CANONICAL.md`
- `RISCO_MATRIZ_SUBSISTEMAS.md`
- `OPERATIONAL_GAP_TOPOLOGY_V1.md`
- `REPOSITORY_COMMIT_TRACKER_OMEGA.md`
- `REPOSITORY_PR_CONTEXT_SIDECAR.md`
- `EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md`
- `ROTINA_OPERACIONAL_BENCHMARKS.md`
- `PROTOCOLO_FALSIFICABILIDADE_PK.md`
- `contracts/AUTHORIAL_OMEGA_ASSURANCE_V1.md` — assurance/falsificadores da federação autoral Ω; Mapa/RafGitTools por exact pin; runtime/device permanece CLOSURE_L12
- `PROTOCOLO_CANONICO_COHERENCIA.md`
- `PROTOCOLO_DOIS_CICLOS_OMEGA.md`
- `DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md`
- `CONCEPT_STRUCTURAL_AUDIT.md`
- `ORQUESTRADOR_FORMAL_CIENTIFICO.md`

## 6. ApkC / Android / runtime

- `APKC_STRUCTURE.md`
- `APKC_PROTOCOL.md`
- `APKC_VALUE_AND_GAPS.md`
- `APKC_FLAGS_LIMITS_AND_COMMANDS.md`
- `APKC_FIRST_PART_EXECUTION.md`
- `LACUNAS_PROFUNDAS_MVP_PRODUTO.md`
- `CI_COMPILER_EXCELLENCE/README.md`
- `RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md`
- `MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md`

Current-artifact ApkC provenance and physical runtime remain evidence-gated as recorded in the urgency matrix.

## 6.1 Fórmulas — referência executável e autoridade

Governance: `CLOSURE_G1`  

| Rota | Papel | Limite |
|---|---|---|
| [FORMULA_AUTHORITY_BRIDGE_V1.md](FORMULA_AUTHORITY_BRIDGE_V1.md) | entrada humana/IA para fórmula → produtor → vetor → gate | não promove autoridade global |
| `../configs/formula-authority-index.v1.json` | índice máquina de autoridade/producer/gap | `TOKEN_VAZIO` preservado |
| `../research/recurrence_matrix_reference/reference.py` | Fibonacci/Tribonacci matricial + Trinity633 escalar | sem fusão semântica |
| `../data/formulas/recurrence-matrix-vectors.v1.json` | vetores dourados cross-language | referência, não device proof |

## 7. Ciência, pesquisa e publicação

- bibliography/science engine routes described by current README and subsystem docs;
- `RAFAELIA_PAPER_MARKET_7_VECTORS.md`;
- `PROTOCOLO_FALSIFICABILIDADE_PK.md`;
- `EVIDENCE_GARDEN_V1.md` — profiles falsificável/enterprise/industrial/fullstack, DMAIC/PDCA, claim gate e TOKEN_VAZIO;
- `receipts/EVIDENCE_GARDEN_BLAKE3_IMPORT_20260929.md` — receipt bounded do primeiro source-audit BLAKE3;
- PBIP route in section 3.

Engineering reproduction does not automatically establish scientific novelty or independent peer review.

## 8. Jurídico, licença, segurança e padrões

- `legal/README.md`
- `MATRIZ_JURIDICO_TECNOLOGICA.md`
- `LICENCAS_COMPARADAS.md`
- `LICENSE_DECISION_RECORD.md`
- `../.github/SECURITY.md`
- `ATRATORES_42_JURIDICOS.md`
- `BASES_SUPRALEGAIS_E_PADROES.md`
- `IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md`

These documents are technical/governance material and do not constitute external certification or legal advice.

## 9. Generated governance outputs — strict handling

| Output | Historical state at observed base | Editing rule |
|---|---|---|
| `generated/DOCUMENT_GOVERNANCE_INDEX.md` | generated from commit `ff000ab...`; `REVIEW_REQUIRED` | **do not edit manually** |
| `generated/DOCUMENT_REVIEW_QUEUE.md` | derived | regenerate |
| `../results/document-governance/summary.json` | historical `ff000ab...` summary | regenerate |
| `../results/document-governance/catalog.jsonl` | observed empty at current base | generator reconciliation required |

The historical generated summary reports 1,356 governed records, 1,265 relations and 841 review-queue items. Those values describe the historical generated cut, not current-main completeness.

```text
current generated governance state = TOKEN_VAZIO_REGEN_REQUIRED
strict governance PASS             = not claimed
```

## 10. Lifecycle states

| State | Meaning |
|---|---|
| `CANONICAL` | official governance entry |
| `ACTIVE` | in-use document/source |
| `REFERENCE` | explanation/specification |
| `AUDIT` | decision/evidence trail |
| `EVIDENCE` | result tied to test/command/receipt |
| `GENERATED` | derived; regenerate, do not hand-edit |
| `PENDING` | content exists without sufficient gate |
| `TOKEN_VAZIO` | evidence absent/insufficient |

## 11. Generator route

The documented authoritative route remains:

```sh
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

This documentation transaction does not claim those commands were executed against `9ed0b8aa...`; therefore current generated state stays `TOKEN_VAZIO_REGEN_REQUIRED`.

## R3

- **F_ok:** canonical index now routes current freestanding/PBIP state and explicitly separates historical generated outputs.
- **F_gap:** full generator refresh/current review queue remains unexecuted in this docs-only transaction.
- **F_next:** regenerate through the repository tool, then review/publish the resulting queue without manually editing derived files.