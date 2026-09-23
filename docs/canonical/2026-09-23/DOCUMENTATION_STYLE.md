# Padrão de documentação — GitHub-native e auditável

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

## 1. Objetivo

Produzir documentação que seja simultaneamente:
- navegável;
- legível no GitHub;
- versionada;
- reconstructível;
- ligada à fonte;
- conservadora quanto a evidência;
- fácil de revisar em diff.

## 2. Cabeçalho mínimo

Documentos de estado devem declarar, quando aplicável:

~~~text
title
status
observed base / commit
scope
authority/source
claim_allowed
~~~

Documentos atemporais de método podem omitir commit apenas quando não descrevem estado corrente.

## 3. Markdown GitHub

Preferir:
- headings hierárquicos;
- tabelas para matrizes pequenas;
- listas curtas para requisitos;
- Mermaid para arquitetura/fluxos;
- admonitions GitHub para NOTE, IMPORTANT, WARNING e CAUTION;
- details/summary para material secundário extenso;
- links relativos para arquivos do mesmo repositório;
- permalinks ou commit SHA quando a evidência precisa ser imutável.

Evitar:
- screenshots quando texto/diagrama versionável é suficiente;
- tabelas gigantes que deveriam ser JSON/TSV;
- badges que sugerem PASS permanente quando o workflow é temporal;
- duplicar conteúdo já canônico em várias páginas.

## 4. Fonte humana e fonte de máquina

Quando uma política possui execução, manter duas superfícies complementares:

~~~text
human-readable Markdown
<-> machine-readable JSON/YAML/TSV/schema
<-> validator/test
<-> workflow
<-> receipt
~~~

O Markdown explica. O contrato de máquina restringe. O teste falsifica. O workflow executa. O receipt registra.

## 5. Diagramas

Mermaid é adequado para:
- flowchart;
- sequence diagram;
- state diagram;
- dependency graph.

O diagrama deve ser explicativo, não evidência de que um componente existe ou funciona.

## 6. Estados visuais

Usar texto explícito, por exemplo:
- PASS;
- FAIL;
- NOT_RUN;
- TOKEN_VAZIO;
- PENDING;
- AUDIT;
- REVIEW_REQUIRED.

Emoji pode acompanhar, mas nunca substituir o estado legível por máquina.

## 7. Documentos generated

Todo documento derivado deve indicar:
- gerador;
- política/schema;
- commit;
- comando;
- regra de edição.

Se o arquivo é GENERATED:
- não corrigir manualmente o resultado;
- corrigir fonte/gerador/política;
- regenerar;
- verificar determinismo.

## 8. Snapshots datados

Snapshots de auditoria ou evidência devem permanecer imutáveis quanto ao significado. Mudança posterior:
- cria novo snapshot;
- aponta supersedes/parent;
- atualiza router;
- preserva o anterior.

## 9. Links e nomes

Preferir nomes que indicam função:
- INDEX;
- ARCHITECTURE;
- BUILD_TEST_EVIDENCE;
- GAPS_AND_NEXT;
- RECEIPT;
- POLICY;
- SPEC;
- ADR.

Data no nome é útil para snapshots, mas não para routers estáveis.

## 10. ADRs

Decisões arquiteturais materiais deveriam usar um registro curto:

~~~text
ADR-NNNN
Context
Decision
Alternatives
Consequences
Evidence
Rollback
Status
~~~

Não criar ADR retroativo para toda decisão histórica sem proveniência suficiente.

## 11. Changelog e release notes

CHANGELOG descreve mudanças versionadas. Release notes descrevem uma release. Nenhum dos dois substitui:
- commit history;
- receipt;
- migration guide;
- security advisory.

## 12. Documentar falhas

Falhas devem registrar:
- gate;
- revisão;
- mensagem essencial;
- causa conhecida ou TOKEN_VAZIO;
- artefato/log;
- próximo teste.

Não apagar resultado vermelho apenas para melhorar apresentação.

## 13. Densidade semântica

Reusar referências em vez de copiar grandes blocos. Quando a informação já existe:
- linkar fonte;
- resumir delta;
- registrar por que a fonte é relevante.

R3 = ⟨F_ok: padrão GitHub-native definido; F_gap: lint/link-check automático de docs ainda não foi adicionado por este corte; F_next: aplicar o padrão em novos documentos e automatizar apenas depois que a política for revisada⟩.
