# Índice Canônico de Documentação — RafPolimata

> **Observed base:** `main@9ed0b8aa93dfb5350b7dbeb9ea1e712ee1298388`  
> **Estado:** `CANONICAL`  
> **Regra:** `README → CURRENT_DOCUMENTATION_STATE → INDEX → área → source/evidence`

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

## 7. Ciência, pesquisa e publicação

- bibliography/science engine routes described by current README and subsystem docs;
- `RAFAELIA_PAPER_MARKET_7_VECTORS.md`;
- `PROTOCOLO_FALSIFICABILIDADE_PK.md`;
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