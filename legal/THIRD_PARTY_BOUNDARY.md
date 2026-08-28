# Third-Party Boundary v1

## Rule zero

A repository containing RAFAELIA-authored material and third-party material is a **license federation**, not a single-rights object.

## Required classification

Each distributable component should be classified as one of:

- `ORIGINAL_RAFAELIA`: rights controlled sufficiently to apply an explicit RAFAELIA license.
- `THIRD_PARTY_VERIFIED`: upstream source and license are evidenced.
- `THIRD_PARTY_TOKEN_VAZIO`: origin/license is not yet sufficiently evidenced.
- `MIXED`: original wrapper/changes plus separately licensed upstream material; boundaries must be documented.
- `PUBLIC_DOMAIN_OR_STATUTORY`: only when the legal status is actually evidenced; never infer from age, visibility or algorithm publication alone.

## Upstream invariants

1. Preserve all copyright/license/attribution notices required by upstream terms.
2. Do not represent third-party material as exclusively owned by RAFAELIA.
3. Do not apply a noncommercial restriction to third-party code when the upstream license grants commercial rights and RAFAELIA lacks authority to narrow them.
4. Do not use repository-level wording to obscure per-file/per-component rights.
5. A permissively licensed upstream implementation may remain commercially usable under its upstream terms even when a separate RAFAELIA wrapper/document is RCNC. State that boundary clearly.
6. If a derived work has reciprocal/copyleft obligations, those obligations must be evaluated before distribution. Unknown compatibility is `TOKEN_VAZIO` and may trigger `HOLD`.
7. Algorithm names/specifications and implementation copyright are distinct questions; identifying an algorithm does not establish the license of a particular implementation.

## Minimum third-party record

```yaml
component: ...
classification: THIRD_PARTY_VERIFIED|THIRD_PARTY_TOKEN_VAZIO|MIXED
upstream_name: TOKEN_VAZIO
upstream_url: TOKEN_VAZIO
upstream_commit_or_version: TOKEN_VAZIO
copyright_notice: TOKEN_VAZIO
license_spdx_or_ref: TOKEN_VAZIO
license_text_path: TOKEN_VAZIO
modifications_by_rafaelia: []
rafaelia_license_scope: NONE|WRAPPER_ONLY|MODIFICATIONS_ONLY|TOKEN_VAZIO
redistribution_gate: PASS|HOLD|TOKEN_VAZIO
receipt_ref: TOKEN_VAZIO
```

## Fail-closed licensing claim

If upstream rights are unknown, do not claim that the affected upstream code is covered by `LicenseRef-RAFAELIA-RCNC-1.0`. This does not mean the code is legally forbidden in every circumstance; it means **this repository lacks sufficient evidence to make the requested relicensing/distribution claim**.
