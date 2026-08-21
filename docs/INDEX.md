# Índice Canônico de Documentação — RafPolimata

> **Entrada canônica:** docs/AGENTES.md §1 (leitura rápida) e §8 (entradas canônicas por subsistema). Índice curado de toda a documentação — regra de leitura README→INDEX→área→código e 10 seções temáticas com estados CANONICAL/REFERENCE/AUDIT.

**Papel:** entrada curada para documentação técnica, operacional, científica, jurídica e de governança.  
**Estado:** `CANONICAL`  
**Catálogo completo:** `generated/DOCUMENT_GOVERNANCE_INDEX.md`

## Regra de leitura

```text
README.md
→ docs/INDEX.md
→ documento de área
→ código/configuração/teste/evidência relacionado
```

A presença neste índice significa que o documento possui rota conhecida. Não significa que todas as afirmações estejam validadas em runtime.

## 1. Entrada e governança de trabalho

| Documento | Função | Estado de uso |
|---|---|---|
| [README.md](README.md) | router curto da árvore `docs/` | `CANONICAL_ROUTER` |
| [AGENTES.md](AGENTES.md) | guia operacional para agentes humanos e IA | `CANONICAL` |
| [AGENTES_CHECKLIST.md](AGENTES_CHECKLIST.md) | checklist de início, execução e encerramento | `REFERENCE` |
| [AGENTES_DECISAO_LOG.md](AGENTES_DECISAO_LOG.md) | decisões, conflitos e escalações | `AUDIT` |
| [DOCUMENT_GOVERNANCE.md](DOCUMENT_GOVERNANCE.md) | catálogo, indexação, ciclo de vida, risco e promoção | `CANONICAL` |
| [ROOT_LOOSE_FILES_REVIEW.md](ROOT_LOOSE_FILES_REVIEW.md) | análise e rota de cada arquivo solto da raiz | `AUDIT/REVIEW_REQUIRED` |
| [MAPA_ESTRUTURAL_REPOSITORIO.md](MAPA_ESTRUTURAL_REPOSITORIO.md) | disposição física e estados por diretório | `CANONICAL` |
| [SAFE_EXTENDED_LOCAL_CI.md](SAFE_EXTENDED_LOCAL_CI.md) | execução local de workflows no Termux | `REFERENCE` |

## 2. Arquitetura e metodologia

| Documento | Função |
|---|---|
| [ARQUITETURA_21_NIVEIS.md](ARQUITETURA_21_NIVEIS.md) | arquitetura semântica em 21 níveis |
| [RAFAELIA_MULTIFILAMENT_EXECUTION_CONTRACT_V1.md](RAFAELIA_MULTIFILAMENT_EXECUTION_CONTRACT_V1.md) | contrato produtor RafPolimata → Vectras, com proveniência, handoff, rollback, fail-safe e watchdog |
| [LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01.md](LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01.md) | matriz multilíngue, operadores e promoção 0,1 por evidência |
| [LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01_ERRATA_V1_1.md](LINGUAGEM/MATRIZ_POLIMATA_TOKEN_VAZIO_01_ERRATA_V1_1.md) | corrige a escada universal: teto 0,5 e perfis próprios acima dele |
| [LINGUAGEM/COERENCIA_COMMIT_FRACTAL_OMEGA.md](LINGUAGEM/COERENCIA_COMMIT_FRACTAL_OMEGA.md) | commits, fixture SHA-256, sete direções e fechamento Ω |
| [LINGUAGEM/ATLAS_ARCOS_FLUXOS_LIVROS_ATOS_CRENCAS_MATEMATICA.md](LINGUAGEM/ATLAS_ARCOS_FLUXOS_LIVROS_ATOS_CRENCAS_MATEMATICA.md) | posições metodológicas, árvore canônica aberta, arcos textuais e semântica dos números |
| [DEZ_DIMENSOES_SEMANTICAS.md](DEZ_DIMENSOES_SEMANTICAS.md) | dimensões e dinâmicas do sentido |
| [CONVERGENCIA_UNICA_METODOLOGICA.md](CONVERGENCIA_UNICA_METODOLOGICA.md) | convergência metodológica auditável |
| [CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md](CONVERGENCIA_ECOSSISTEMA_RMR_RAFAELIA.md) | relações com repositórios irmãos |
| [INFO_DYNAMICS_INTERNAL_TRACEABILITY.md](INFO_DYNAMICS_INTERNAL_TRACEABILITY.md) | rastreabilidade interna de dinâmica informacional |
| [ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md](ROADMAP_CODIGO_DOCUMENTACAO_CONSCIENTE.md) | evolução conjunta de código e documentação |

## 3. Excelência operacional e evidência

| Documento | Função | Estado |
|---|---|---|
| **[RISCO_GESTAO_FRAMEWORK_CANONICAL.md](RISCO_GESTAO_FRAMEWORK_CANONICAL.md)** | **Framework canônico de gestão de riscos em 4 camadas (acadêmica, normativa, autoral, claims), 7 gates de prevenção (G0-G7), 5 gates de detecção (D1-D5), 4 gates de remediação (R0-R3), 4 gates de melhoria (I0-I3)** | **`CANONICAL`** |
| **[RISCO_MATRIZ_SUBSISTEMAS.md](RISCO_MATRIZ_SUBSISTEMAS.md)** | **Matriz de riscos específicos por subsistema (ApkC, Indexer, T^7, Router, Governança) com gates operacionalizáveis, remediação e priorização** | **`CANONICAL`** |
| [OPERATIONAL_GAP_TOPOLOGY_V1.md](OPERATIONAL_GAP_TOPOLOGY_V1.md) | grafo canônico de gaps, urgências, incertezas, owners, proveniência, closures e relações técnico-comerciais | |
| [closures/CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.md](closures/CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.md) | liga `TOKEN_VAZIO` estruturado ao grafo sem promover gaps materiais a PASS | |
| [REPOSITORY_COMMIT_TRACKER_OMEGA.md](REPOSITORY_COMMIT_TRACKER_OMEGA.md) | rastreamento bounded de repositórios, forks, commits, shards e snapshots | |
| [REPOSITORY_PR_CONTEXT_SIDECAR.md](REPOSITORY_PR_CONTEXT_SIDECAR.md) | sidecar de contexto temporal e semântico dos pull requests | |
| [EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md](EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md) | governança de otimização, fallback e rollback | |
| [ROTINA_OPERACIONAL_BENCHMARKS.md](ROTINA_OPERACIONAL_BENCHMARKS.md) | rotina de benchmark e estatística operacional | |
| [PROTOCOLO_FALSIFICABILIDADE_PK.md](PROTOCOLO_FALSIFICABILIDADE_PK.md) | falsificabilidade mínima em P(k) | |
| [PROTOCOLO_CANONICO_COHERENCIA.md](PROTOCOLO_CANONICO_COHERENCIA.md) | coerência, invariantes e gates | |
| [PROTOCOLO_DOIS_CICLOS_OMEGA.md](PROTOCOLO_DOIS_CICLOS_OMEGA.md) | dois ciclos, fail-safe, failover e rollback | |
| [DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md](DEEPRafa2_PROTOCOLO_EVIDENCIA_MULTIDOMINIO.md) | claim, fonte, ensaio, incerteza e IP | |
| [CONCEPT_STRUCTURAL_AUDIT.md](CONCEPT_STRUCTURAL_AUDIT.md) | conceitos ligados a arquivos e lacunas | |
| [ORQUESTRADOR_FORMAL_CIENTIFICO.md](ORQUESTRADOR_FORMAL_CIENTIFICO.md) | orquestração científica e gates dimensionais | |

## 4. ApkC e baixo nível Android

| Documento | Função | Limite atual |
|---|---|---|
| [APKC_STRUCTURE.md](APKC_STRUCTURE.md) | estrutura do micro-toolchain | implementação estrutural |
| [APKC_PROTOCOL.md](APKC_PROTOCOL.md) | contrato de APK/AXML/DEX/ELF | runtime parcial |
| [APKC_VALUE_AND_GAPS.md](APKC_VALUE_AND_GAPS.md) | valor e lacunas | não é valuation certificado |
| [APKC_FLAGS_LIMITS_AND_COMMANDS.md](APKC_FLAGS_LIMITS_AND_COMMANDS.md) | flags, limites e comandos | validar por versão |
| [APKC_FIRST_PART_EXECUTION.md](APKC_FIRST_PART_EXECUTION.md) | fechamento inicial de toolchain/ELF/DEX | execução Termux pendente |
| [LACUNAS_PROFUNDAS_MVP_PRODUTO.md](LACUNAS_PROFUNDAS_MVP_PRODUTO.md) | mapa de lacunas técnicas | estados precisam reconciliar artefatos |
| [CI_COMPILER_EXCELLENCE/README.md](CI_COMPILER_EXCELLENCE/README.md) | gates de compilador | evidência por run |

## 5. Runtime e segmentação

| Documento | Função |
|---|---|
| [RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md](RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md) | validação local registrada |
| [MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md](MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md) | contrato de segmentação e evidência |

## 6. Jurídico, licenças, segurança e padrões

| Documento | Função |
|---|---|
| [MATRIZ_JURIDICO_TECNOLOGICA.md](MATRIZ_JURIDICO_TECNOLOGICA.md) | matriz jurídico-tecnológica |
| [LICENCAS_COMPARADAS.md](LICENCAS_COMPARADAS.md) | comparação de licenças e termos |
| [LICENSE_DECISION_RECORD.md](LICENSE_DECISION_RECORD.md) | decisão P0 fail-closed sobre licença, terceiros e redistribuição |
| [SECURITY.md](../.github/SECURITY.md) | política de disclosure e fronteira explícita do canal privado |
| [ATRATORES_42_JURIDICOS.md](ATRATORES_42_JURIDICOS.md) | framework de atratores jurídicos |
| [BASES_SUPRALEGAIS_E_PADROES.md](BASES_SUPRALEGAIS_E_PADROES.md) | bases supralegais e padrões técnicos |
| [IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md](IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md) | protocolo IA↔humanos |

> Estes documentos não substituem parecer jurídico profissional, auditoria independente ou certificação normativa.

## 7. Comunicação, mercado e publicação

| Documento | Função |
|---|---|
| [RAFAELIA_PAPER_MARKET_7_VECTORS.md](RAFAELIA_PAPER_MARKET_7_VECTORS.md) | tradução técnica e mercadológica |
| [RELEASE_NOTES_PENDING.md](RELEASE_NOTES_PENDING.md) | pendências de release |
| [LOGOTIPO_RAFAELIA_60COL.md](LOGOTIPO_RAFAELIA_60COL.md) | representação ASCII |

## 8. Índices e filas geradas

| Saída | Função | Edição manual |
|---|---|---|
| [generated/DOCUMENT_GOVERNANCE_INDEX.md](generated/DOCUMENT_GOVERNANCE_INDEX.md) | resumo material do catálogo | proibida; regenerar |
| [generated/DOCUMENT_REVIEW_QUEUE.md](generated/DOCUMENT_REVIEW_QUEUE.md) | fila ordenada por risco | proibida; regenerar |
| [generated/REPOSITORY_LOOSE_FILES_MAP.md](generated/REPOSITORY_LOOSE_FILES_MAP.md) | compatibilidade com mapa da primeira parte | regenerar pelo gate original |

## 9. Estados de leitura

| Estado | Interpretação |
|---|---|
| `CANONICAL` | entrada oficial de governança |
| `REFERENCE` | explicação ou especificação |
| `AUDIT` | trilha e evidência documental |
| `EVIDENCE` | resultado ligado a teste/comando |
| `RUNTIME` | código ou rota executável |
| `PENDING` | conteúdo existente sem gate suficiente |
| `TOKEN_VAZIO` | evidência ausente ou insuficiente |

## 10. Comando de atualização

```sh
python3 -m unittest \
  tests.test_document_governance \
  tests.test_audit_repository_structure \
  tests.test_validate_root_file_decisions \
  tests.test_audit_zip_artifact \
  tests.test_language_matrix \
  tests.test_language_commit_evidence \
  tests.test_repo_commit_tracker \
  tests.test_repo_pr_context_tracker \
  tests.test_operational_gap_topology

python3 scripts/validate_root_file_decisions.py
python3 scripts/validate_operational_gap_topology.py
python3 scripts/language_matrix.py --state data/language/language-matrix-state.v1.json
python3 scripts/language_commit_evidence.py
python3 scripts/repo_commit_tracker.py --validate-only
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

Mudanças em documentação devem atualizar este índice quando adicionarem nova entrada canônica. O catálogo gerado cobre todos os arquivos; este arquivo permanece curado para navegação humana.
