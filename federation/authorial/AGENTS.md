# AGENTS.md — Authorial federation source archive

Scope: `federation/authorial/**`.

Governance closure: `CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY` for structural, authorship, rights and source-identity unknowns.

This subtree is a **provenance-preserving source archive**, not a canonical implementation surface.

Before touching any file here:

1. Read `../../data/federation/authorial_sources.v1.json`.
2. Read `../../docs/federation/AUTHORIAL_FEDERATION_V1.md`.
3. Read the nearest source-specific provenance and license file.
4. Resolve the exact source repository/ref/path/blob.
5. If authorship, license, authority or destination role is missing, stop with `TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)`.

Hard invariants:

```text
REPO_OWNER != AUTHOR
HEADER_ADDED != SOLE_AUTHORSHIP
IMPORTED != INTEGRATED != TESTED != PASS
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
```

Do not edit an imported snapshot to make it look native to RafPolimata. If functionality is useful, create a separately documented successor/adapter outside this archive, preserve attribution/license requirements, and prove dependency/build/runtime gates independently.

Do not copy a neighboring source merely because it is in the same repository. Rights and authorship are path-specific.

Current notable boundaries:

- Vectras `conjunto_de_conceitos` is archived under its explicit Rafael clean-room provenance and GPL-2.0-only terms; its raw Linux syscall layer is **not** a no-syscall L0 proof.
- Rafaelia_Private material is evaluated per subtree/file; custom or modified license text must remain a `LicenseRef` unless it exactly matches an SPDX license.
- Gaia and unresolved aliases remain pointer-only while rights/identity are `TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)`.

Promotion requires a separate successor receipt.