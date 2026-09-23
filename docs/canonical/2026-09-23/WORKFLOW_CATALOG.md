# Catálogo de GitHub Actions

**Base observada:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Quantidade:** 50 arquivos .github/workflows/*.yml

> [!NOTE]
> Os grupos abaixo são de navegação derivados dos nomes e de workflows-chave lidos neste corte. Pertencer a um grupo não implica que o workflow esteja verde, completo ou executado nesta revisão.

## 1. ApkC / Android / artifact truth

- android-proof-bundle.yml
- android-runtime-evidence-v1.yml
- apkc-canonical-lang-dispatch.yml
- apkc-first-part.yml
- apkc-physical-env-provenance.yml
- apkc-physical-gate-codebinding.yml
- apkc-provider-physical-recursive-chain.yml
- apkc-recursive-receipt-tree.yml
- apkc-rmr-coupled-research-core.yml
- apkc-semantic-return-oracle.yml
- apkc-stage11-integrity-truth.yml
- apkc-stage9-signing-truth.yml

## 2. Compilador / freestanding / ABI / semântica

- compiler-three-target-gate.yml
- freestanding-l0.yml
- m062-anchor.yml
- m063-language-freestanding.yml
- segment-v1-abi-contract.yml
- semantic-ir-core.yml
- stage0-fluent-event-v1.yml

## 3. Governança / evidência / custódia

- audit-readiness-r1-r10.yml
- document-governance.yml
- epistemic-claim-gate.yml
- internal-custody-ledger.yml
- operational-gap-topology.yml
- rafaelia-handoff-gate.yml
- repository-commit-tracker.yml
- runtime-truth-receipt.yml
- workflow-graph-audit.yml

## 4. Ciência / pesquisa / bibliografia

- arxiv-production-strategy-validation.yml
- arxiv-query-recall-probe.yml
- daily-bibliography-evolution.yml
- formal-science.yml
- neurobio-claim-contract.yml
- science-learning-integrity.yml
- signed-area-geometry.yml

## 5. Conversação / contexto / conhecimento

- cognitive-operator-reference.yml
- contextual-relational-tensor-v1.yml
- conversation-indexer-ci.yml
- conversation-pair-binder.yml
- conversation-permutation-feedback.yml
- genesis-8x3-materializer.yml
- matrix-compose-v1.yml
- multifilament-envelope-gate.yml

## 6. Integração / segurança / ecossistema

- canari-integration.yml
- ecosystem-build-doctor.yml
- forensic-deception-lab.yml
- phase-cd-truthful-gates.yml
- rafcodephi-bootstrap-evidence.yml

## 7. CI geral

- ci.yml

Total listado: 50.

## 8. Workflows lidos em detalhe neste corte

### ci.yml

Gate amplo, em push e pull request. Faz composição de:
- source-contract;
- TOKEN_VAZIO;
- compiler station;
- M063;
- repository structure;
- Android ABI plan;
- encoders;
- ApkC;
- P(k);
- artifacts.

O main corrente falhou no strict TOKEN_VAZIO changed-lines gate antes das etapas posteriores.

### document-governance.yml

É a rota autoritativa para regenerar outputs documentais derivados e verificar determinismo.

### m063-language-freestanding.yml

É a rota especializada do contrato de linguagens/freestanding.

### semantic-ir-core.yml

É a rota especializada para Semantic IR, registry de arquiteturas e semântica lexical.

## 9. Regra de leitura

Um workflow define uma possibilidade de execução. Para documentar resultado corrente, exigir:

~~~text
workflow file
+ exact commit
+ run id
+ job/steps
+ conclusion
+ artifact/log when material
~~~

Sem esses elementos, o estado é WORKFLOW_PRESENT ou TOKEN_VAZIO_EXECUTION, não PASS.

R3 = ⟨F_ok: 50/50 workflows inventariados por rota; F_gap: conclusão individual de cada workflow não foi reexecutada neste corte; F_next: usar este catálogo como router e abrir o run exato apenas quando o claim depender dele⟩.
