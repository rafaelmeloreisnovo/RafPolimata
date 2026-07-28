# PR #178 — Science Learning Integrity HOTFIX Receipt

Date: 2026-07-27  
Branch: `hotfix/pr178-science-engine-integrity-clean`  
Base: `main`

## Origin

PR #178 introduced useful acquisition infrastructure but contained three material integrity defects:

1. ORCID search results were parsed directly as works instead of resolving each returned ORCID iD through the works endpoint.
2. Repository/access metadata was labelled as scientific validation.
3. Repeated DOI retrieval across configured domains was labelled as canonicity.

## Corrective delta

| Area | Correction |
|---|---|
| ORCID | Two-phase `search -> ORCID iD -> works -> work-summary` flow |
| Authentication | `ORCID_ACCESS_TOKEN` required; absence becomes explicit `TOKEN_VAZIO` |
| Media type | ORCID requests use `application/vnd.orcid+json` |
| Stage 3 | Renamed to `repository_qualified` |
| Stage 4 | Renamed to `cross_domain_candidate` |
| Claims | `claim_allowed=false` is invariant at every automated stage |
| Relevance | Stage 2 requires positive configured-domain relevance |
| Deduplication | Normalized DOI merge preserves the strongest relevance evidence |
| Output compatibility | `bibliography.bib` and Markdown acquisition artifacts remain generated |
| Mutation safety | Cross-domain promotion deep-copies records instead of mutating source-domain records |
| CI | Dedicated offline workflow; no `|| true`, fallback success or network dependency |
| Historical data | Original counts preserved, evidence interpretation corrected |

## Deterministic evidence

Executed locally against the branch-equivalent files:

```text
python3 -m py_compile ...                         PASS
python3 tests/test_science_learning_engine.py     9/9 PASS
legacy command --dry-run --offline                PASS
forbidden-claim grep                              PASS
```

The regression suite covers:

- DOI normalization;
- ORCID identifier extraction;
- ORCID two-phase work resolution;
- ORCID vendor JSON media type;
- zero-network behavior without token;
- Stage 3 repository qualification semantics;
- relevance gate;
- deduplication relevance preservation;
- BibTeX output compatibility;
- Stage 4 non-mutation and `claim_allowed=false`.

## Epistemic state

```text
F_ok   = acquisition, normalization, deterministic tests, explicit receipts
F_gap  = independent scientific review, retraction check, replication evidence
F_next = run CI, review diff, merge only after green checks

claim_allowed = false
```

This HOTFIX does not delete historical artifacts and does not promote any scientific conclusion. It corrects the machinery that decides what may be claimed.
