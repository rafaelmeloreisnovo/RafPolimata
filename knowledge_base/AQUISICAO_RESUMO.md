# AQUISICAO_RESUMO — Historical Receipt from PR #178

Generated: 2026-07-27T18:27:00.303059+00:00  
Integrity correction applied: 2026-07-27

> **Acquisition history only.** The original labels overstated the evidence level. Under the v2 integrity contract, every row has `claim_allowed=false`.

## Preserved counts

| Domain | Stage 1 discovery | Stage 2 candidate | Stage 3 repository-qualified | Stage 4 cross-domain candidate | State |
|---|---:|---:|---:|---:|---|
| physics | 80 | 41 | 11 | 0 | TOKEN_VAZIO |
| chemistry | 80 | 38 | 11 | 0 | TOKEN_VAZIO |
| biology | 80 | 45 | 18 | 0 | TOKEN_VAZIO |
| mathematics | 80 | 45 | 15 | 0 | TOKEN_VAZIO |

## Corrected interpretation

- Stage 3 proves only repository/access qualification: Zenodo community metadata, open licence and downloadable file.
- Stage 3 does not prove peer review, methodological validity, absence of retraction, reproducibility or scientific truth.
- Stage 4 would indicate retrieval overlap of the same normalized DOI in multiple configured domains; it would not establish canonicity.
- No DOI reached stage 4 in this historical execution.

```text
claim_allowed=false
```

## Next verifiable gate

Regenerate with `scripts/science_learning_engine.py` v2 and attach a reproducibility receipt containing the executed command, timestamp, source counts and content hashes. Scientific promotion requires an independent review receipt beyond the acquisition engine.
