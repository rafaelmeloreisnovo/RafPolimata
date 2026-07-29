# Voynich Dataset Contract — 2026-07-29

## Scope

A user-supplied ZIP was audited locally as input for `experiments/voynich_repro`. Corpus bytes are **not committed** while source license, external provenance and publication authority remain unresolved.

```text
status=LOCAL_DATASET_STRUCTURAL_PASS_SCIENTIFIC_GATE_FAIL
claim_allowed=false
scientific_claims_reproduced=false
```

## Frozen input identity

- uploaded basename: `voynich_dataset.zip`
- size: `280625` bytes
- entries: `19`
- ZIP SHA-256: `b26aa0bc99a062aef7b05f609cc58640b9b6927c8898ba265f58379cf9572437`
- content Merkle root: `3ce702c3243a697e30e9ae7a357d4bd41fdc0c006ea288b473f8fd9ff19f5178`

The archive passed path-traversal and expansion checks.

## Reproduced structure

- `37139` Voynich token lines and `37139` control token lines were read.
- Onset-family support counts reproduce when bare two-character tokens are excluded.
- Observed third-character entropy values reproduce within CSV rounding.
- A deterministic file-level `T^7 → Ψ → FFT` run completed over all 19 members.
- Local audit implementation: `10/10 PASS`.

## Blocking findings

1. The cleaned word list starts with transcription-header words (`evat`, `extracted`, `from`, `lsi`, `ivtff`, `txt`, `version`, `of`, `modified`).
2. `family_entropy_vs_baseline.csv` uses `baseline_mean == observation` and `z_score == 0` for every listed family; the null comparison is degenerate.
3. `paper_outline.md` claims approximately `rho=0.82` and slope `0.96`, while the JSON artifact records `rho=0.9998991884669046` and slope `0.04442231125855387`.
4. The package contains no generator or analysis code, no fixed seed, and no random-state contract.
5. The so-called Arabic control is a Latin consonant-like token corpus without a source corpus, transliteration contract, morphology generator or provenance.
6. The folio summary contains no `cluster_id`, centroid, `k` or seed, so the stated k-means result is not reproducible.
7. Candidate Arabic root/gloss parallels have no blinded labels, effect-size contract or falsifier.
8. Source URL, license, retrieval timestamp and externally verified transcription hash are absent.

## Structural run

```text
cardio_spectral_correlation_r = -0.30455665140492627
entropy_milli_axis0_q8        = 4248
bits_geom_bins16              = 4.247927513443585
cycle_hashing_q16_10000       = NOT_FOUND
```

These values describe the uploaded **file inventory trajectory**, not manuscript language or meaning.

## TOKEN_VAZIO

- `TOKEN_VAZIO_SOURCE_LICENSE`
- `TOKEN_VAZIO_EXTERNAL_SOURCE_HASH`
- `TOKEN_VAZIO_CONTROL_GENERATOR_AND_SEED`
- `TOKEN_VAZIO_MONTE_CARLO_NULL_MODEL`
- `TOKEN_VAZIO_KMEANS_PARAMETERS_AND_OUTPUT`
- `TOKEN_VAZIO_SEMANTIC_BLIND_VALIDATION`
- `TOKEN_VAZIO_INDEPENDENT_REPLICATION`

## Next gate

Freeze provenance and license, add the exact generator and analysis code with seeds, regenerate every derived table from the raw transcription, and run negative controls before any claim of Arabic morphology, semantics or decipherment.
