# Índice de Documentação Solta — RafPolimata

**Estado:** `AUDIT / INDEX`  
**Proprietário lógico:** `documentation-governance`  
**Escopo:** documentos e artefatos documentais fora da rota canônica principal ou ainda com rota de promoção/migração pendente.  
**Regra:** este índice não move, apaga, promove nem valida arquivos por existir referência a eles.

## Objetivo

Este índice oferece uma rota humana curta para materiais que cresceram fora da árvore documental canônica ou que ainda precisam de reconciliação semântica.

A regra de leitura permanece:

```text
README.md
→ docs/INDEX.md
→ documento de área
→ código/configuração/teste/evidência relacionado
```

Este arquivo é uma ponte de auditoria. Ele complementa, sem substituir:

- [`ROOT_LOOSE_FILES_REVIEW.md`](ROOT_LOOSE_FILES_REVIEW.md) — revisão técnica e decisões propostas para arquivos soltos da raiz;
- [`DOCUMENT_GOVERNANCE.md`](DOCUMENT_GOVERNANCE.md) — política de catálogo, relações, risco e promoção;
- [`generated/DOCUMENT_GOVERNANCE_INDEX.md`](generated/DOCUMENT_GOVERNANCE_INDEX.md) — snapshot gerado do catálogo;
- [`generated/DOCUMENT_REVIEW_QUEUE.md`](generated/DOCUMENT_REVIEW_QUEUE.md) — fila gerada de revisão;
- [`generated/REPOSITORY_LOOSE_FILES_MAP.md`](generated/REPOSITORY_LOOSE_FILES_MAP.md) — mapa gerado de arquivos sem rota confirmada.

## Invariantes

```text
arquivo existe != implementação
implementação != execução
execução != evidência
histórico != estado atual
ganho estimado != benchmark observado
índice != conteúdo
TOKEN_VAZIO != FAIL
```

Nenhum arquivo listado abaixo deve ser apagado ou movido automaticamente. Migrações físicas devem preservar histórico Git, links, identidade do blob quando aplicável, proveniência e rollback.

## 1. Documentação solta da raiz com decisão existente

| Arquivo | Natureza | Estado de leitura | Rota atual/proposta |
|---|---|---|---|
| [`README_RAFAELIA_ROOT_OPTIMIZER.md`](../README_RAFAELIA_ROOT_OPTIMIZER.md) | documentação de componente | `ACTIVE_REFERENCE` | mover em PR dedicado para `docs/components/RAFAELIA_ROOT_OPTIMIZER.md` após gates |
| [`RAFAELIA_MASTER_DOC.txt`](../RAFAELIA_MASTER_DOC.txt) | documento master histórico | `HISTORICAL_REFERENCE / REVIEW_REQUIRED` | arquivar preservando integralidade e separar claims de estado atual |
| [`Arduíno.txt`](../Arduíno.txt) | dossiê AVR/registradores | `TECHNICAL_REFERENCE_DRAFT` | refatorar para laboratório de hardware com datasheet + build + medição |
| [`Arm64 Mixer leve criptografia.md`](<../Arm64 Mixer leve criptografia.md>) | transcrição/experimento AArch64 NEON | `EXPERIMENTAL_REVIEW` | separar código, teste e benchmark; não promover como criptografia sem prova |
| [`L1.md`](../L1.md) | fonte + shell + resultados misturados | `UNSTRUCTURED_EXPERIMENT` | dividir em fixture, runner e receipt de benchmark |
| [`RASBERY.MD`](../RASBERY.MD) | brainstorm/roadmap | `BACKLOG_SOURCE` | converter em backlog tipado com critérios de aceite e evidência |

As decisões detalhadas e os gates correspondentes estão em [`ROOT_LOOSE_FILES_REVIEW.md`](ROOT_LOOSE_FILES_REVIEW.md) e `configs/root-file-decisions.v1.json`.

## 2. Família RAF na raiz

Estes arquivos formam um subsistema documental coerente, apesar de estarem fisicamente na raiz. Não devem ser tratados como lixo ou duplicata do índice geral.

| Arquivo | Função | Estado recomendado | Observação |
|---|---|---|---|
| [`RAF_INDEX.md`](../RAF_INDEX.md) | índice físico atual dos 63 métodos M001–M063 | `DOMAIN_INDEX` | distinto de `docs/INDEX.md`; mantém separada a custódia histórica 56/56 de 17/06/2026 |
| [`RAF_40_STRATEGIES.md`](../RAF_40_STRATEGIES.md) | princípios de engenharia low-level | `REFERENCE / ENGINEERING_POLICY` | estratégia não equivale a evidência de execução |
| [`RAF_56_METHODS.md`](../RAF_56_METHODS.md) | catálogo conceitual histórico dos primeiros 56 métodos | `REFERENCE / CLAIM_REVIEW` | não cobre M057–M063; ganhos estimados continuam estimativas até benchmark observável |
| [`RAF_BENCHMARK_MATRIX.md`](../RAF_BENCHMARK_MATRIX.md) | matriz estimado vs real | `PENDING_MEASUREMENT` | `_medir_` deve ser interpretado como `TOKEN_VAZIO_MEASUREMENT`; extensão M057–M063 permanece pendente |
| [`RAF_CHECKLIST_96_ITEMS.md`](../RAF_CHECKLIST_96_ITEMS.md) | checklist histórico 40 estratégias + 56 métodos | `AUDIT / RECONCILE_STATES` | não deve ser lido como cobertura dos 63 métodos; `[x]` mistura tipos de evidência |
| [`RAF_VALIDATION_PROTOCOL.md`](../RAF_VALIDATION_PROTOCOL.md) | níveis L0–L5 de validação | `REFERENCE` | deve coexistir como dimensão de maturidade, não substituir estados documentais |
| [`RELEASE_NOTES.md`](../RELEASE_NOTES.md) | snapshot da release histórica | `HISTORICAL_RELEASE_SNAPSHOT` | resultados históricos não implicam estado do `main` atual nem hardware físico atual |

### Delta M057–M063

Os sete métodos posteriores ao catálogo de 56 já existem fisicamente e agora possuem rota explícita no `RAF_INDEX.md`:

- `RAF_057_eeprom_wear_leveling.c`;
- `RAF_058_can_bus_mcp2515_spi.c`;
- `RAF_059_rtos_minimal_no_heap.c`;
- `RAF_060_bootloader_ota_uart.c`;
- `RAF_061_vectra_neon_arm32.c`;
- `RAF_062_quaternary_anchor_eight_gate.c`;
- `RAF_063_language_completion_freestanding.c`.

A indexação desse delta prova presença/identidade documental, não valida automaticamente os claims internos. Em especial, M063 declara explicitamente que não executa runtimes estrangeiros: valida política, masked replacement e plano de dependências/lowering.

### Modelo recomendado para os métodos RAF

```text
artifact_state
compile_state
host_runtime_state
target_runtime_state
physical_hardware_state
benchmark_state
evidence_ref
claim_allowed
```

Um único checkbox não deve substituir esse vetor.

## 3. Artefatos de raiz relacionados, mas não documentação principal

| Arquivo | Natureza | Estado |
|---|---|---|
| `runtime-lock.json` | manifest de integração/runtime | `KEEP_AT_ROOT / TOKEN_VAZIO_EXTERNAL_PINS_AND_TOOLCHAIN` até materialização dos pins |
| `RAFAELIA_COMPLETE_v4.zip` | arquivo binário histórico | `BINARY_ARTIFACT_REVIEW` até inventário, hashes, origem e licença por entrada |
| `safe-extended` | entrypoint CLI | `KEEP_AT_ROOT` |
| `big_test.sh` | orquestrador de testes | migração para `scripts/` somente após atualização de chamadas e gates |

## 4. Documentação semanticamente solta

Um arquivo também pode estar no diretório correto e continuar sem rota semântica.

Definições:

```text
PHYSICAL_LOOSE  = arquivo no lugar físico inadequado ou excepcional
SEMANTIC_LOOSE  = arquivo sem aresta suficiente no índice/grafo documental
```

A fonte de verdade para a fila completa de `LINK_REQUIRED`, `ROOT_REVIEW` e `SENSITIVITY_REVIEW` é o catálogo gerado por `scripts/document_governance.py`. Como snapshots gerados podem ficar atrás do `main`, suas contagens são históricas até regeneração sobre o commit correspondente.

## 5. Reconciliação observada — PR #269

O primeiro gate remoto após a criação deste índice havia encontrado dois links quebrados por percent-encoding, sete métodos físicos M057–M063 ainda fora do `RAF_INDEX.md` e um `REFERENCE_REPAIR` espúrio causado por uma expressão regular de shell interpretada como link Markdown.

A tranche corretiva do PR #269 reconciliou esses três deltas e preservou a semântica dos gates. No head validado `2d733058234a38574e6c1539ca182b687353fb35`, os quatro workflows de pull request concluíram `success`:

```text
CI                                      run 31857725586  PASS
Document Governance                     run 31857725525  PASS
Formal Science Orchestrator              run 31857725555  PASS
M063 language completion freestanding    run 31857725580  PASS
```

### Auditoria estrutural observada

```text
root_raf_method_files = 63
broken_markdown_links = 0
raf_existing_not_indexed = 0
raf_indexed_not_existing = 0
```

### Document Governance observada

A regeneração determinística no merge-ref testado `5ed48fb68eae066cff23972533eec4c3e3430918` registrou:

```text
state = REVIEW_REQUIRED
blockers = 0
broken_reference_sources = 0
missing_canonical_indexes = []
files = 1099
relations = 1037
review_queue = 678
CANONICAL = 6
INDEXED = 415
LINK_REQUIRED = 654
REVIEW_STALE = 12
ROOT_REVIEW = 10
SENSITIVITY_REVIEW = 2
REFERENCE_REPAIR = 0
```

O estado `REVIEW_REQUIRED` permanece intencional: existem 10 arquivos soltos de raiz com decisões explícitas, mas `unmapped=0`, `stale_hashes=0`, `critical=0` e `claim_allowed=false`.

O artefato `document-governance-evidence` do run 31857725525 foi materializado com artifact id `9239555059`, 273038 bytes e SHA-256 do ZIP `60b5b2c10b4bdb66a144578b8fe2ccede3b4a5851c4a97a355f838b51905ff3b`.

### Preservação do gate M063

A correção do falso `REFERENCE_REPAIR` apenas dividiu textualmente a construção de uma regex de headers hospedados; o contrato M063 continuou PASS, incluindo:

- headers hosted proibidos: PASS;
- objeto M063 sem símbolos indefinidos: PASS;
- matriz de arquitetura e linguagem: PASS;
- auditoria de biblioteca estrangeira: PASS;
- rewrite fail-closed: PASS;
- ARM64 + ARM32 + C++ export + hosted kernel: PASS;
- reprodutibilidade byte a byte: PASS.

Isso não promove `APK_SIGNATURE`, `APK_INSTALL`, `ANDROID_RUNTIME` nem `DEVICE_ACCELERATOR_RUNTIME`.

## 6. Critério para promoção ou movimentação

```text
hash/identidade atual confere
∧ natureza do arquivo está classificada
∧ relações de entrada e saída são conhecidas
∧ destino está definido
∧ estado epistemológico está explícito
∧ claims possuem evidência ou TOKEN_VAZIO
∧ links/dependências foram encontrados
∧ testes/gates aplicáveis passam
∧ rollback está descrito
∧ revisão humana existe quando exigida
```

## 7. Gates de reconciliação

A tranche atual executou e fechou os três primeiros gates abaixo sobre o head reconciliado:

```sh
python3 scripts/audit_repository_structure.py --depth 5
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
python3 scripts/validate_root_file_decisions.py
```

O mapa abaixo é produzido pelo workflow `ApkC First Part Closure`:

```sh
python3 scripts/apkc_first_part_gate.py \
  --write results/apkc-first-part-gate.json \
  --write-map docs/generated/REPOSITORY_LOOSE_FILES_MAP.md
```

Esse workflow aceita `workflow_dispatch`, mas o conector GitHub usado nesta operação não expõe ação de dispatch e o PR #269 não altera paths que o disparem automaticamente. Assim, neste corte:

```text
REPOSITORY_LOOSE_FILES_MAP_current_head = TOKEN_VAZIO_NOT_EXECUTED
```

Nenhum arquivo ApkC foi alterado artificialmente apenas para forçar o workflow.

## 8. Leitura rápida

Para entender o problema dos arquivos soltos sem percorrer todo o repositório:

1. este índice;
2. [`ROOT_LOOSE_FILES_REVIEW.md`](ROOT_LOOSE_FILES_REVIEW.md);
3. [`generated/DOCUMENT_GOVERNANCE_INDEX.md`](generated/DOCUMENT_GOVERNANCE_INDEX.md);
4. [`generated/DOCUMENT_REVIEW_QUEUE.md`](generated/DOCUMENT_REVIEW_QUEUE.md);
5. [`INDEX.md`](INDEX.md) para retornar à navegação canônica.

---

**F_ok:** links quebrados = 0; RAF físico não indexado = 0; RAF indexado inexistente = 0; `REFERENCE_REPAIR` = 0; governança e M063 passaram no mesmo head de evidência.  
**F_gap:** `REPOSITORY_LOOSE_FILES_MAP` do head corrente permanece `TOKEN_VAZIO_NOT_EXECUTED`; M057–M063 não recebem claim global de build/runtime/hardware/benchmark apenas por estarem indexados.  
**F_next:** materializar o mapa pelo workflow ApkC quando houver dispatch observável e, depois, iniciar migrações físicas em PRs dedicados seguindo as decisões já registradas.