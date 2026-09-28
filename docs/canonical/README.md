# RafPolimata — documentação canônica

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

> [!IMPORTANT]
> Este diretório é um router. Ele aponta para cortes documentais ligados a uma revisão observada e não substitui fonte, execução, evidência ou receipt.

## Suplemento de engenharia corrente — 2026-09-28

- [Maturity/Productization V1](2026-09-28/MATURITY_AND_PRODUCTIZATION_V1.md)
- [Router do suplemento](2026-09-28/README.md)
- Base observada do main: f840fde2de65edacf8b2abbee8ca9d1f9c912b6d
- Predecessor de revisão: PR#358@f35fc8402ea790a8dead96817c0847bbaf9c1a98
- Escopo: equivalência, propriedades, benchmark receipts, ABI/API, supply chain, threat model, SDK e reprodução cross-environment.

O corte amplo de 2026-09-23 permanece como base documental histórica; o suplemento de 2026-09-28 supersede apenas o estado das superfícies de maturidade acima.

## Corte amplo anterior

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
