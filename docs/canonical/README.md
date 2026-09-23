# RafPolimata — documentação canônica

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

> [!IMPORTANT]
> Este diretório é um router. Ele aponta para cortes documentais ligados a uma revisão observada e não substitui fonte, execução, evidência ou receipt.

## Corte corrente

- Corte documental corrente: [2026-09-23](2026-09-23/README.md)
- Base de implementação observada: main@f22efc099ac530d946ff2ec34954455f75632e92
- Estado: CANONICAL_DOCUMENTATION_LAYER / REVIEW_REQUIRED
- claim_allowed: false

## Regra de leitura

SOURCE ≠ ARTIFACT ≠ EXECUTION ≠ EVIDENCE ≠ CLAIM

TOKEN_VAZIO ≠ FAIL ≠ PASS

A documentação datada permanece histórica. Um corte novo pode superseder o roteamento corrente, mas não reescreve o significado de receipts, resultados ou snapshots anteriores.

## Ordem curta

1. [Router do corte](2026-09-23/README.md)
2. [Levantamento do repositório](2026-09-23/REPOSITORY_SURVEY.md)
3. [Arquitetura](2026-09-23/ARCHITECTURE.md)
4. [Linguagens](2026-09-23/LANGUAGES.md)
5. [Build, testes e evidência](2026-09-23/BUILD_TEST_EVIDENCE.md)
6. [Catálogo de workflows](2026-09-23/WORKFLOW_CATALOG.md)
7. [Superfícies GitHub](2026-09-23/GITHUB_SURFACES.md)
8. [Evidência e claims](2026-09-23/EVIDENCE_AND_CLAIMS.md)
9. [Estilo documental](2026-09-23/DOCUMENTATION_STYLE.md)
10. [Component catalog](2026-09-23/COMPONENT_CATALOG.md)
11. [Release e versionamento](2026-09-23/RELEASE_VERSIONING.md)
12. [Runtime e targets](2026-09-23/RUNTIME_AND_TARGETS.md)
13. [Security e supply chain](2026-09-23/SECURITY_SUPPLY_CHAIN.md)
14. [Glossário](2026-09-23/GLOSSARY.md)
15. [Gaps e próximos gates](2026-09-23/GAPS_AND_NEXT.md)

R3 = ⟨F_ok: router estável criado; F_gap: promoção depende dos gates do PR; F_next: revisar o corte e regenerar somente saídas derivadas por seus próprios executores⟩.
