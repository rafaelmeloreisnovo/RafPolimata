# RafPolimata

RafPolimata é uma arquitetura semântica, tecnológica e jurídica para sistemas complexos que combinam:

- modelagem matemática e dinâmica discreta;
- engenharia de software de baixo nível;
- Android, ARM32/ARM64, APK, DEX e ELF;
- criptografia aplicada e rastreabilidade;
- semiótica, linguística e memória estruturada;
- governança de evidência, licenças e conformidade.

O repositório separa rigorosamente:

```text
conceito ≠ implementação ≠ execução ≠ evidência ≠ validação runtime
```

## Entrada canônica

A ordem de leitura é:

1. [`docs/AGENTES.md`](docs/AGENTES.md) — regras operacionais e invariantes;
2. [`docs/INDEX.md`](docs/INDEX.md) — índice curado da documentação;
3. [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md`](docs/MAPA_ESTRUTURAL_REPOSITORIO.md) — disposição física;
4. [`docs/DOCUMENT_GOVERNANCE.md`](docs/DOCUMENT_GOVERNANCE.md) — catálogo, grafo, temporalidade, risco e promoção;
5. documento técnico do subsistema em alteração.

## Fontes de verdade executáveis

| Corpo | Arquivo | Função |
|---|---|---|
| Estado material do ecossistema | [`ECOSYSTEM_RUNTIME_STATE.json`](ECOSYSTEM_RUNTIME_STATE.json) | componente, evidência, lacuna e próxima ação |
| Contrato do estado | [`contracts/ecosystem-runtime-state.schema.json`](contracts/ecosystem-runtime-state.schema.json) | schema da matriz material |
| Validação local | [`scripts/validate_runtime_truth_local.sh`](scripts/validate_runtime_truth_local.sh) | build e testes locais sem crédito de Actions |
| Validador do estado | [`scripts/validate_ecosystem_runtime_state.py`](scripts/validate_ecosystem_runtime_state.py) | coerência de estados com Python stdlib |
| Política documental | [`configs/document-governance.v1.json`](configs/document-governance.v1.json) | áreas, responsáveis, revisão e sensibilidade |
| Catálogo documental | [`scripts/document_governance.py`](scripts/document_governance.py) | identidade, relações, duplicidade e fila |
| Aprendizado científico | [`scripts/science_learning_engine.py`](scripts/science_learning_engine.py) | ORCID + Zenodo → 4 estágios → `knowledge_base/` → `vv_scan_buf()` |

> A ausência de execução não vira PASS. GitHub Actions, Safe Extended e Termux são meios de execução; a evidência precisa registrar commit, ambiente, comando, stdout/stderr e hashes.

## Arquitetura principal

| Camada | Arquivo principal | Entrada |
|---|---|---|
| Pipeline de alto nível | `raf_compile.h` | `raf_compile_file()` |
| Micro-toolchain Android | `Apkc/apkc.c` | `apkc_main()` |
| Tabela multilíngua | `Apkc/lang_profile.h` | `lang_profile_from_path()` |
| Motor cognitivo T^7 | `rafaelia/verbovivo.c` | `verbovivo_main()` |
| Runtime router | `Benchmark/raf_runtime_router.h` | seleção de backend |
| Conversation indexer | `runtime/conversation_indexer/` | codec `segment.v1` |
| Orquestração local | `raf_shell/raf_shell.c` | TUI e pipeline local |

## Disciplina de evidência

Estados usados no repositório:

| Estado | Significado |
|---|---|
| `VOID` | referência ou placeholder sem artefato |
| `PENDING` | conteúdo existe sem gate suficiente |
| `REFERENCE` | documentação ou especificação |
| `AUDIT` | relatório, contrato ou trilha |
| `RUNTIME` | corpo executável dependente de ambiente |
| `IMPLEMENTED` | código existe, ainda separado de prova runtime |
| `PASS` | gate definido foi executado e passou |
| `FAIL` | gate definido foi executado e falhou |
| `TOKEN_VAZIO` | evidência ausente ou insuficiente |

`TOKEN_VAZIO` é preferível a promover uma conclusão não demonstrada.

## Governança documental

O motor documental opera em seis níveis:

```text
L0 estrutura física
L1 identidade SHA-256
L2 grafo de referências
L3 área, responsável e temporalidade
L4 evidência, duplicidade e risco
L5 fila de revisão e promoção
```

Comandos:

```sh
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

Saídas principais:

```text
results/document-governance/catalog.jsonl
results/document-governance/relations.jsonl
results/document-governance/review-queue.json
docs/generated/DOCUMENT_GOVERNANCE_INDEX.md
docs/generated/DOCUMENT_REVIEW_QUEUE.md
```

Nenhum arquivo é movido ou apagado automaticamente.

## Gate local momentâneo

Enquanto não houver run remoto utilizável, executar:

```sh
bash scripts/validate_runtime_truth_local.sh
```

Para workflows compatíveis com o executor local:

```sh
sh safe-extended run .github/workflows/document-governance.yml
sh safe-extended run .github/workflows/apkc-first-part.yml
```

## Conversation Indexer — `segment.v1`

`runtime/conversation_indexer/` inclui:

- header explícito de 64 bytes;
- conversation record de 96 bytes;
- message record de 128 bytes;
- leitor limitado e iteração tipada;
- CRC32C de header, records, título, autor e conteúdo;
- rejeição de corrupção, truncamento, roles inválidos e ranges fora do buffer.

Evidências e limites estão em:

- [`docs/RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md`](docs/RUNTIME_TRUTH_LOCAL_VALIDATION_2026-07-18.md);
- [`docs/MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md`](docs/MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1_1.md).

Streaming extractor, writer atômico, checkpoint/resume, BLAKE3 e execução em device permanecem pendentes enquanto não houver evidência correspondente.

## ApkC — estado honesto

`Apkc/` contém um micro-toolchain experimental em C para escrita direta de ZIP/APK, AXML, DEX e ELF com backends ARM64 e ARM32.

Documentos de entrada:

- [`docs/APKC_STRUCTURE.md`](docs/APKC_STRUCTURE.md);
- [`docs/APKC_PROTOCOL.md`](docs/APKC_PROTOCOL.md);
- [`docs/APKC_FLAGS_LIMITS_AND_COMMANDS.md`](docs/APKC_FLAGS_LIMITS_AND_COMMANDS.md);
- [`docs/APKC_FIRST_PART_EXECUTION.md`](docs/APKC_FIRST_PART_EXECUTION.md);
- [`Apkc/proofs/GAPS.md`](Apkc/proofs/GAPS.md).

| Gate | Estado no corte atual |
|---|---|
| Geradores ZIP/AXML/DEX/ELF no código | `IMPLEMENTED` estrutural |
| Gate source→binary AArch64 + ARM32 | `IMPLEMENTED`; execução atual pendente |
| Reprodutibilidade do binário | `TOKEN_VAZIO` até novo run completo |
| APK atual gerado pelo mesmo commit | `TOKEN_VAZIO` |
| ELF ARM32 dentro do APK atual | `TOKEN_VAZIO` |
| ELF ARM64 dentro do APK atual | `TOKEN_VAZIO` |
| DEX do APK atual com SHA-1/Adler-32 validados | `TOKEN_VAZIO` |
| Java/Kotlin/Groovy → D8 → DEX funcional | `IMPLEMENTED` no pipeline; runtime pendente |
| Assinatura do APK atual | `TOKEN_VAZIO` |
| Instalação e lançamento Android | `TOKEN_VAZIO` |
| NativeActivity sem crash/logcat limpo | `TOKEN_VAZIO` |

Resultados históricos continuam úteis como `REFERENCE` ou `AUDIT`, mas não substituem um run coerente do commit atual.

## Qualidade e automação

Principais corpos:

- `.github/workflows/ci.yml` — gates gerais;
- `.github/workflows/formal-science.yml` — orquestração científica;
- `.github/workflows/document-governance.yml` — catálogo e fila documental;
- `.github/workflows/apkc-first-part.yml` — verdade, toolchain e prova inicial ApkC;
- `scripts/audit_repository_structure.py` — estrutura física `L0`;
- `scripts/document_governance.py` — governança documental `L1–L5`;
- `scripts/validate_apkc_formats.py` — validação independente de APK/DEX/ELF.

## Documentação de profundidade

O índice completo e curado está em [`docs/INDEX.md`](docs/INDEX.md). Entre os núcleos:

- excelência operacional, benchmark e rollback;
- protocolos de coerência e falsificabilidade;
- arquitetura semântica;
- matrizes jurídico-tecnológicas e licenças;
- convergência do ecossistema;
- ciência formal e rastreabilidade informacional.

## Aviso jurídico e científico

Este material é técnico-acadêmico. Não substitui parecer jurídico profissional, auditoria de segurança, certificação normativa ou revisão científica independente.

Afirmações de produção, conformidade, desempenho e segurança só podem ser promovidas depois dos gates específicos e da evidência reproduzível correspondente.
