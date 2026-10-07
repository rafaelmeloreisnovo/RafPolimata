# Authorial source archive

This directory is a rights-gated source archive for **verified authored material**.

It is not a second canonical implementation tree and is not part of the default RafPolimata build unless a later successor explicitly promotes a module.

## Read order

1. `../../data/federation/authorial_sources.v1.json`
2. `../../docs/federation/AUTHORIAL_FEDERATION_V1.md`
3. source-specific provenance file
4. source-specific license copy
5. imported source

## Invariant

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`

`IMPORTED != INTEGRATED != TESTED != PASS`

Every imported file remains bound to its source repository, exact ref, original blob and governing license.

## Layout

- `vectras/conjunto_de_conceitos/` — Rafael-authored clean-room low-level bundle, GPL-2.0-only.
- `rafaelia_private/ordernar_ra/` — individually verified Rafael-authored source with its subtree-specific LicenseRef.
- Gaia, PCR and unresolved aliases are represented in the federation manifest rather than copied here when their current destination or rights boundary does not permit copying.

## Promotion rule

A copied source snapshot must not be edited in place to erase its origin. If it becomes useful to RafPolimata, create a separate successor implementation or adapter and prove its dependency, build, execution and evidence gates independently.
