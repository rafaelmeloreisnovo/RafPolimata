# Science Learning Engine — Integrity Contract v2

> **Status:** acquisition pipeline with explicit epistemic gates.  
> **Invariant:** every generated record carries `claim_allowed=false`.

## Purpose

The engine acquires public metadata from Zenodo and, when an ORCID public API token is available, resolves ORCID search results into work summaries. It organizes candidates for later review; it does **not** declare scientific truth, peer review, replication or canonicity.

The stable command remains:

```bash
python3 scripts/science_learning_engine.py --output knowledge_base/
```

The compatibility entrypoint delegates to `scripts/science_learning_engine_v2.py`.

## Acquisition stages

| Stage | Name | Automated criterion | Scientific meaning |
|---:|---|---|---|
| 1 | `discovery` | Metadata result with a title | Located only |
| 2 | `candidate` | DOI, abstract with at least 100 characters, keywords and positive domain relevance | Eligible for review |
| 3 | `repository_qualified` | Stage 2 plus Zenodo community, recognized open licence and downloadable file | Repository/access qualification only |
| 4 | `cross_domain_candidate` | Same normalized DOI retrieved under at least two configured domains | Retrieval overlap only |

No automated stage permits a scientific claim. Promotion beyond acquisition requires a separate review receipt covering, at minimum:

- provenance and authorship;
- methodology and data availability;
- peer-review status;
- retraction/correction check;
- falsifiability and limitations;
- conflicts of interest;
- replication when applicable.

Until those gates are evidenced:

```text
claim_allowed=false
```

## ORCID integrity path

ORCID search results are treated as identifiers, not as publication objects:

```text
search(query)
  -> ORCID iD
  -> /{orcid}/works
  -> work-summary
  -> normalized DOI/title
```

Set the public API token through the environment:

```bash
export ORCID_ACCESS_TOKEN='...'
python3 scripts/science_learning_engine.py --domains physics
```

When the token is absent, the ORCID path returns an explicit `TOKEN_VAZIO` and performs no unauthenticated ORCID request. Zenodo acquisition remains available unless `--offline` is set.

## Deterministic offline gate

```bash
python3 -m py_compile \
  scripts/science_learning_engine.py \
  scripts/science_learning_engine_v2.py \
  tests/test_science_learning_engine.py

python3 tests/test_science_learning_engine.py

python3 scripts/science_learning_engine.py \
  --dry-run --offline \
  --domains physics \
  --query 'integrity fixture' \
  --max-per-stage 1
```

The dedicated workflow `.github/workflows/science-learning-integrity.yml` does not use `|| true`, fallback `echo`, or network success as a substitute for deterministic tests.

## Output structure

```text
knowledge_base/
  <domain>/
    stage_1_discovery/
    stage_2_candidate/
    stage_3_repository_qualified/
    stage_4_cross_domain_candidate/
  AQUISICAO_RESUMO.md
```

Stage 4 synthesis files may be ingested experimentally by `vv_scan_buf()`, but ingestion is not evidence. Each synthesis therefore declares that it is neither proof nor canonical truth.

## Historical artifacts from PR #178

The previously generated dataset is preserved as acquisition history. Its former stage labels must be interpreted under this corrected mapping:

- former “validated” output -> `repository_qualified`;
- former “canonical” output -> `cross_domain_candidate`;
- scientific claim permission -> `false`.

Regeneration under v2 writes the corrected directory names and receipts. Legacy generated files should not be silently rewritten without a reproducibility receipt containing command, timestamp, source counts and content hashes.

## TOKEN_VAZIO

`TOKEN_VAZIO` is emitted when a required source or promotion condition is absent. It has evidence weight zero for the missing claim, remains auditable, and records the next verifiable gate instead of inventing completion.
