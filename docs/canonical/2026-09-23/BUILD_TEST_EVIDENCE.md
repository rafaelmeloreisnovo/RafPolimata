# Build, testes, CI e evidência

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

**Base observada:** main@f22efc099ac530d946ff2ec34954455f75632e92

## 1. Entradas locais principais

O Makefile raiz expõe:

| Target | Papel |
|---|---|
| make help | listar rotas canônicas |
| make apkc-hardened-source | gerar e falsificar TU ApkC hardened |
| make syntax | syntax check freestanding AArch64 |
| make verbovivo | build do pipeline T7 |
| make encoders | golden tests ARM32/ARM64 |
| make proof | clean reproducible proof run |
| make audit | auditoria freestanding |
| make language-contract | contrato M063 |
| make compile | strict compiler por RAF_LANG/RAF_ARCH |
| make compile-plan | plano JSON determinístico sem execução |
| make compiler-contract | validar station inventory |
| make compiler-selftest | gates transacionais/adversariais |
| make hotfix-audit | composição de gates do compilador |
| make library-audit | assimilar/auditar biblioteca |
| make strict-elf | auditar perfil ELF |
| make report | mostrar provas curadas e último run |
| make clean | limpar artefatos locais conhecidos |

## 2. Gates especializados

### M063

.github/workflows/m063-language-freestanding.yml:
- valida inventário legível por máquina;
- executa o audit do contrato M063;
- constrói ELF mínimo e aplica strict ELF audit;
- publica relatório como artifact.

### Semantic Language Core

.github/workflows/semantic-ir-core.yml:
- executa gate offline de semântica, arquitetura e léxico;
- publica receipt do semantic language core.

### Document Governance

.github/workflows/document-governance.yml:
- valida contratos JSON;
- executa testes de governança;
- valida decisões de arquivos soltos;
- audita ZIP versionado;
- gera catálogo/grafo/fila;
- verifica regeneração determinística;
- publica outputs derivados.

> [!WARNING]
> docs/generated e results/document-governance são saídas derivadas. Devem ser regeneradas pelo executor; editar manualmente para aparentar atualidade viola o contrato documental.

## 3. Estado recente do CI principal

Para main@f22efc099ac530d946ff2ec34954455f75632e92 foi observado:

- CI run 35574321825: **failure**;
- Internal Custody Ledger, no mesmo push: **success**;
- Formal Science Orchestrator, no mesmo push: **success**.

No job principal do CI:
- o source-contract binding do RAF Hash Fabric passou;
- os testes unitários do validador TOKEN_VAZIO: **19/19 PASS**;
- o scan estrito de changed lines encontrou **5 errors**, **0 warnings** e encerrou o job;
- as etapas posteriores do job foram skipped;
- artifacts de diagnóstico foram preservados.

Hash reportado pelo validador nesse run:
7833c15d5c0be5cbc667895a74275632dbf2c84d0eeabc95d48d641a7dd66a14

Isso deve ser lido como:

~~~text
validator unit tests = PASS
strict changed-lines validation = FAIL
overall CI job = FAIL
subsequent gates = NOT_RUN / skipped
~~~

Não é correto converter esse run em PASS parcial do pipeline completo.

## 3A. Validação da própria camada documental

A primeira materialização documental, commit 8607ad69b70c880184aacc865cc3c6d21177f9d8, acionou CI run 35825593320 e falhou corretamente no gate de unknown-state: 31 ocorrências novas sem closure.

A correção não alterou o validador. Os documentos passaram a declarar CLOSURE_L11 como binding de governança.

No commit 191df4026e789b9285a97860186610bded35557a:

- CI run: 35825729691;
- overall job: PASS;
- validator unit tests: 19/19 PASS;
- changed-lines diff base: 8607ad69b70c880184aacc865cc3c6d21177f9d8;
- errors: 0;
- warnings: 1;
- gate status: PASS;
- validator report hash: a262ddd13ab0c925fba64d518a374c2130719738321e2d9399bf2c9306996af1.

Esse run prova a correção incremental entre os dois commits documentais. A validação do conjunto completo contra main fica para o evento de pull request.

## 4. Negative tests do source contract

No mesmo CI aparecem mensagens SOURCE_CONTRACT_FAIL durante mutações deliberadas de um bit em README/LICENSE/scope do módulo RAF Hash Fabric. Essas mensagens são o comportamento esperado do falsificador. A etapa GitHub correspondente concluiu success porque as mutações foram rejeitadas.

## 5. Scheduled workflows

Na amostra dos 100 runs recentes:
- Repository commit and fork tracker mostrou sucessos recorrentes;
- Daily Bibliography Evolution mostrou falhas recorrentes recentes.

Essa observação é temporal e não deve ser convertida em estado permanente.

## 6. Escada de evidência

| Nível | Exemplo | Não prova |
|---|---|---|
| source | arquivo versionado | build |
| static | parser/lint/syntax gate | link/runtime |
| build | objeto/ELF/DEX/APK produzido | execução |
| execution | processo rodou | device específico |
| device | receipt no hardware | reprodução independente |
| reproduction | outro executor/provider repete | novidade científica |
| claim gate | falsificadores e evidência satisfeitos | certificação externa |

## 7. Regra de receipt

Um receipt útil deve, quando aplicável, registrar:
- commit;
- ambiente/toolchain;
- comando;
- inputs e hashes;
- outputs e hashes;
- resultado;
- falha/gap;
- ligação ao claim;
- timestamp;
- executor/provider;
- rollback ou supersessão.

## 8. Comandos de documentação

A governança documental já define:

~~~sh
python3 scripts/document_governance.py --write --print-summary
python3 scripts/document_governance.py --check --print-summary
python3 -m unittest tests.test_document_governance
~~~

Este corte não declara que a regeneração foi executada no main atual; portanto freshness dos generated outputs continua um gate independente.

R3 = ⟨F_ok: rotas de build/CI e resultado corrente separados por estado; F_gap: CI principal falha antes dos gates posteriores e generated plane precisa rerun próprio; F_next: corrigir os 5 findings do TOKEN_VAZIO, rerodar CI, depois regenerar governança com receipt ligado ao commit⟩.
