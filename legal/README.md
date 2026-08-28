# RafPolimata — Legal, Provenance & Access Layer v1

Status: DRAFT_FOR_REVIEW / operationally enforceable by repository gates only after merge.

This directory separates five concerns that MUST NOT be conflated: ownership, license permission, provenance evidence, security evidence, and commercial authorization.

## Invariants

1. `TOKEN_VAZIO != 0` and never means approval.
2. Unknown author, origin, implementation, license, signature, or authority blocks the related claim until resolved by evidence.
3. Third-party material remains governed by its upstream license; RAFAELIA terms never narrow or expand third-party grants without legal authority.
4. Commercial permission for Original RAFAELIA Material requires a completed formal Commercial Authorization Instrument identifying the exact material/version/commit and scope.
5. Educational and community noncommercial access is fee-free under the dedicated policy; no student registration or literacy/socioeconomic proof is a condition of this RAFAELIA license.
6. Receipts are append-only evidence. They strengthen traceability but do not create an absolute legal guarantee of authorship, priority, patentability, validity, or enforceability.
7. MD5 is legacy-only and MUST NOT be the sole integrity, authentication, security, or priority/anteriority anchor.

## Files

- `RAFAELIA-RESEARCH-COMMUNITY-NONCOMMERCIAL-LICENSE-1.0.md` — custom source-available/noncommercial license for explicitly designated Original RAFAELIA Material.
- `COMMERCIAL_AUTHORIZATION_CONTRACT_TEMPLATE.md` — no commercial grant exists until a complete instrument is executed by authorized parties.
- `EDUCATION_AND_SOCIAL_ACCESS_POLICY.md` — free noncommercial education/community access and data-minimization rules.
- `THIRD_PARTY_BOUNDARY.md` — upstream license and attribution boundary.
- `TOKEN_VAZIO_AND_GAPS.md` — auditable gap lifecycle.
- `PROVENANCE_AND_RECEIPTS.md` — chain-of-custody/receipt contract.
- `NON_REGRESSION_POLICY.md` — append-only and anti-regression invariants.
- `LICENSE_REGISTRY.yaml` — machine-readable status and gates.
- `LEGAL_BASIS_BR.md` — Brazilian normative references; not a legal opinion.

## Scope activation

The custom RAFAELIA license applies only when a file/module is explicitly marked with `LicenseRef-RAFAELIA-RCNC-1.0` or is explicitly listed as Original RAFAELIA Material in the registry. Repository visibility alone does not relicense third-party material.

## Legal review gate

Before commercial deployment or reliance in litigation/registration, obtain review by qualified counsel for the exact jurisdiction, ownership chain, contributors, third-party components and contract facts. Unreviewed legal conclusions remain `TOKEN_VAZIO`.
