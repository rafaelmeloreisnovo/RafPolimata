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

A fonte de verdade para a fila completa de `LINK_REQUIRED`, `ROOT_REVIEW` e `SENSITIVITY_REVIEW` é o catálogo gerado por `scripts/document_governance.py`. Como snapshots gerados podem ficar atrás do `main`, suas contagens são históricas até regeneração sobre o commit corrente.

## 5. Observação de governança pós-criação do índice

O primeiro gate remoto de Document Governance disparado pela criação deste índice passou e regenerou o catálogo sem drift, mantendo `claim_allowed=false` e estado `REVIEW_REQUIRED`. Naquele run foram observados 1.089 arquivos catalogados, 1.023 relações e 676 itens na fila; como o repositório continuou recebendo merges em seguida, essas contagens permanecem um **snapshot temporal**, não uma contagem canônica eterna.

A auditoria estrutural do mesmo corte encontrou duas referências Markdown quebradas por percent-encoding e sete métodos físicos M057–M063 ainda ausentes do índice RAF. Este documento e `RAF_INDEX.md` corrigem exatamente esses dois deltas; o resultado final depende do rerun sobre o novo head.

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

Antes de reorganizar fisicamente a documentação:

```sh
python3 scripts/audit_repository_structure.py --depth 5
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
python3 scripts/validate_root_file_decisions.py
python3 scripts/apkc_first_part_gate.py \
  --write results/apkc-first-part-gate.json \
  --write-map docs/generated/REPOSITORY_LOOSE_FILES_MAP.md
```

A promoção só ocorre depois da observação desses gates no commit correspondente.

## 8. Leitura rápida

Para entender o problema dos arquivos soltos sem percorrer todo o repositório:

1. este índice;
2. [`ROOT_LOOSE_FILES_REVIEW.md`](ROOT_LOOSE_FILES_REVIEW.md);
3. [`generated/DOCUMENT_GOVERNANCE_INDEX.md`](generated/DOCUMENT_GOVERNANCE_INDEX.md);
4. [`generated/DOCUMENT_REVIEW_QUEUE.md`](generated/DOCUMENT_REVIEW_QUEUE.md);
5. [`INDEX.md`](INDEX.md) para retornar à navegação canônica.

---

**F_ok:** existe governança documental, decisões explícitas e preservação de proveniência; o índice RAF agora distingue inventário atual de custódia histórica.  
**F_gap:** snapshots gerados continuam temporais; M057–M063 ainda precisam de gates próprios para qualquer claim de build/runtime/hardware/benchmark.  
**F_next:** executar os gates no novo head, regenerar mapa/catálogo e só então promover ou mover arquivos.