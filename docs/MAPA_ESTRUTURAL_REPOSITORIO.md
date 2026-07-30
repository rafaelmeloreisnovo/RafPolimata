# Mapa Estrutural e Governança do Repositório RafPolimata

> **Entrada canônica:** docs/AGENTES.md §1 (leitura rápida — o que este repo faz) e §8 (entradas canônicas por subsistema). Mapa físico e lógico do repositório — modelo em seis níveis, estados canônicos, ciclo de vida documental e rotas para arquivos soltos.

Este documento organiza a leitura técnica do repositório e separa estrutura física de governança documental.

```text
estrutura observada ≠ uso conhecido ≠ evidência ≠ validação runtime
```

Quando a função de um caminho não está comprovada, o estado correto permanece `VOID`, `PENDING`, `REFERENCE`, `AUDIT`, `RUNTIME` ou `TOKEN_VAZIO`.

## 1. Modelo em seis níveis

| Nível | Corpo | Executor | Resultado |
|---|---|---|---|
| `L0` | estrutura física | `scripts/audit_repository_structure.py` | diretórios, raiz, links e RAF index |
| `L1` | identidade | `scripts/document_governance.py` | SHA-256, tamanho, tipo e hash normalizado |
| `L2` | relações | mesmo executor | grafo dirigido e referências quebradas |
| `L3` | governança | política documental | área, dono lógico, classificação e temporalidade |
| `L4` | qualidade e risco | motor documental | evidência, duplicidade, sensibilidade e notas |
| `L5` | operação | catálogo e fila | indexar, reparar, revisar, quarentenar ou promover |

A política é `configs/document-governance.v1.json`; o contrato de registro é `schemas/document-record.v1.schema.json`.

## 2. Estados canônicos de uso

| Estado | Uso | Critério de saída |
|---|---|---|
| `VOID` | conteúdo ausente ou referência sem artefato | criar artefato mínimo ou remover em mudança dedicada |
| `PENDING` | conteúdo existe sem gate suficiente | adicionar teste, manifesto ou validação |
| `REFERENCE` | documentação, especificação ou roteiro | ligar à implementação, evidência ou limite |
| `AUDIT` | contrato, matriz ou relatório | manter origem, data e hash |
| `RUNTIME` | código ou rota executável | compilar/executar no ambiente declarado |
| `IMPLEMENTED` | corpo técnico presente | produzir teste e evidência runtime |
| `PASS` | gate definido executado com sucesso | preservar comando e artefato |
| `FAIL` | gate definido falhou | corrigir ou registrar rollback |
| `TOKEN_VAZIO` | evidência ausente ou insuficiente | executar ou manter a lacuna explícita |

## 3. Ciclo de vida documental

| Estado | Significado |
|---|---|
| `CANONICAL` | entrada oficial, política ou índice |
| `ACTIVE` | em uso e sob manutenção |
| `SUPPORTING` | ativo auxiliar |
| `AUDIT` | trilha ou contrato |
| `EVIDENCE` | prova, teste ou resultado |
| `GENERATED` | derivado de ferramenta; não editar manualmente |
| `ARCHIVE_CANDIDATE` | sem uso atual demonstrado |
| `QUARANTINE` | risco de exposição ou integridade |

## 4. Disposição por diretórios

| Tag | Caminho | Conteúdo | Responsável lógico | Estado-base |
|---|---|---|---|---|
| `ci` | `.github/workflows/` | workflows e gates | `ci-governance` | `RUNTIME` |
| `apk-android` | `Apkc/` | C/ASM, APK, DEX, ELF, ZIP e provas | `apkc-maintainer` | `ACTIVE` |
| `benchmark` | `Benchmark/` | router, benchmark e primitivas | `runtime-maintainer` | `RUNTIME` |
| `configs` | `configs/` | políticas e manifestos operacionais | `schema-governance` | `AUDIT` |
| `schemas` | `schemas/`, `contracts/` | contratos de dados | `schema-governance` | `AUDIT` |
| `manifests` | `manifests/` | escopo e cadeia de implementação | `schema-governance` | `AUDIT` |
| `data` | `data/` | entradas observadas | `evidence-custodian` | `EVIDENCE` |
| `docs` | `docs/` | documentação curada e gerada | `documentation-governance` | `REFERENCE` |
| `results` | `results/` | saídas de teste e catálogos | `evidence-custodian` | `EVIDENCE` |
| `scripts` | `scripts/` | validação e automação | `automation-maintainer` | `RUNTIME` |
| `tests` | `tests/` | regressão positiva e negativa | `quality-assurance` | `EVIDENCE` |
| `tools` | `tools/` | utilitários de validação | `automation-maintainer` | `RUNTIME` |
| `runtime` | `runtime/` | codecs e componentes runtime | `runtime-maintainer` | `ACTIVE` |
| `rafaelia` | `rafaelia/` | verbovivo e Fiber-H | `runtime-maintainer` | `ACTIVE` |
| `assets` | `assets/` | ativos auxiliares | `documentation-governance` | `SUPPORTING` |
| `root-methods` | `RAF_001_*.c` … `RAF_056_*.c` | métodos C de baixo nível | `low-level-methods-maintainer` | `ACTIVE/PENDING` |
| `root-compiler` | `raf_*.c`, `raf_*.h`, `raiz_*` | compilador e otimizadores | `compiler-maintainer` | `RUNTIME` |

## 5. Entradas obrigatórias

| Arquivo | Papel | Estado |
|---|---|---|
| `README.md` | entrada pública e verdade resumida | `CANONICAL` |
| `docs/INDEX.md` | navegação humana curada | `CANONICAL` |
| `docs/AGENTES.md` | regras de sessão e não-colisão | `CANONICAL` |
| `docs/DOCUMENT_GOVERNANCE.md` | política operacional de documentos | `CANONICAL` |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | mapa físico e lógico | `CANONICAL` |
| `ECOSYSTEM_RUNTIME_STATE.json` | estado material por componente | `CANONICAL/AUDIT` |

Cadeia de entrada:

```text
README.md → docs/INDEX.md → docs/AGENTES.md → documento de área → implementação/evidência
```

## 6. Rotas para arquivos soltos

| Rota | Condição | Ação |
|---|---|---|
| `QUARANTINE_REVIEW` | chave privada/keystore potencial | bloquear e investigar |
| `SENSITIVITY_REVIEW` | sinal de dado sensível | revisão humana |
| `DUPLICATE_REVIEW` | SHA-256 repetido | comparar origem, licença e dependências |
| `ROOT_REVIEW` | arquivo de raiz fora da política | indexar ou mover em PR dedicado |
| `OWNER_REQUIRED` | área desconhecida | atribuir responsável lógico |
| `REFERENCE_REPAIR` | link local quebrado | corrigir referência |
| `LINK_REQUIRED` | sem entrada no grafo | vincular a índice ou justificativa |
| `REVIEW_STALE` | prazo de revisão ultrapassado | revisar estado e validade |
| `INDEXED` | classificado e referenciado | manter sob drift |

`MOVE_OR_INDEX` do mapa legado passa a ser decomposto em rotas específicas. Nenhuma rota autoriza exclusão automática.

## 7. Comandos canônicos

### Estrutura L0

```sh
python3 scripts/audit_repository_structure.py --depth 5
```

### Governança L1–L5

```sh
python3 -m unittest tests.test_document_governance
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
```

### Gate local por workflow

```sh
sh safe-extended run .github/workflows/document-governance.yml
```

## 8. Saídas de governança

```text
results/document-governance/summary.json
results/document-governance/catalog.jsonl
results/document-governance/relations.jsonl
results/document-governance/duplicates.json
results/document-governance/review-queue.json
docs/generated/DOCUMENT_GOVERNANCE_INDEX.md
docs/generated/DOCUMENT_REVIEW_QUEUE.md
```

O catálogo completo é máquina-legível. `docs/INDEX.md` permanece curado para humanos.

## 9. Política de refatoração

1. **Mapear antes de mover:** registrar origem, hash, relações, área e rota.
2. **Mover em PR dedicado:** atualização de links, índices, workflow e rollback no mesmo conjunto.
3. **Não apagar duplicidade automaticamente:** conteúdos iguais podem ter proveniência ou licença distintas.
4. **Não promover pelo nome:** “proof”, “runtime” ou “certified” no nome não cria evidência.
5. **Preservar falhas:** `FAIL`, `TOKEN_VAZIO` e referências históricas devem permanecer distinguíveis.
6. **Fallback primeiro:** otimizações só substituem baseline após comparação e rollback.
7. **Dados mínimos:** relatórios de sensibilidade nunca reproduzem o valor detectado.
8. **Documentação e código juntos:** mudança técnica atualiza a entrada correspondente no mesmo PR.

## 10. Critério de excelência

Uma reorganização só pode ser promovida quando:

```text
estrutura íntegra
∧ catálogo regenerável
∧ referências reparadas
∧ responsável lógico definido
∧ risco crítico zerado
∧ testes passam
∧ rollback possível
```

A condição melhora governança e confiança, mas não substitui prova funcional ou científica do conteúdo.


## § README Files — Artefatos Governados

| Arquivo | Estado | Proprietário lógico |
|---|---|---|
| `README.md` | CANONICAL | documentation-governance |
| `README_RAFAELIA_ROOT_OPTIMIZER.md` | ACTIVE | compiler-maintainer |
| `Apkc/proofs/README.md` | EVIDENCE | apkc-maintainer |
| `build/README.md` | EVIDENCE | ci-governance |
| `docs/CI_COMPILER_EXCELLENCE/README.md` | CANONICAL | ci-governance |
| `docs/excelencia_operacional/README.md` | CANONICAL | documentation-governance |
| `fractal_core/README.md` | ACTIVE | low-level-methods-maintainer |
| `native/rafaelia_omega_v32/README.md` | EVIDENCE | runtime-maintainer |
| `proofs/run-arm64-full-chain/README.md` | EVIDENCE | apkc-maintainer |
| `research/APKC_RMR_RESEARCH_CORE/README.md` | REFERENCE | apkc-maintainer |
| `research/APKC_RMR_RESEARCH_CORE/receipts/README.md` | EVIDENCE | evidence-custodian |
| `research/MULTILINGUAL_SCRIPTURE_SEMANTIC_CORE/README.md` | REFERENCE | semantic-research |
| `tests/fixtures/README.md` | EVIDENCE | quality-assurance |
| `tools/rafbbs/README.md` | ACTIVE | automation-maintainer |
| `tools/rafbbs/tests/README.md` | EVIDENCE | quality-assurance |
