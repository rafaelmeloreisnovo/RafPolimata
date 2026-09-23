# Release and versioning state

**Governance binding: CLOSURE_L11** — unresolved historical/current-state conflicts stay explicitly gated.  
**Base observed:** main@f22efc099ac530d946ff2ec34954455f75632e92

## GitHub release surface

One GitHub release was observed:

| Field | Value |
|---|---|
| tag | V1.0.0 |
| name | Compilador de APK em C |
| state | prerelease |
| published | 2026-06-14 |
| assets | none |
| release body | empty |

This release object is historical and does not describe the current main branch.

## Version-label conflict requiring care

The repository also contains:
- CHANGELOG.md with a v1.0.0 entry dated 2026-06-20;
- RELEASE_NOTES.md with a v1.0.0 entry dated 2026-06-17;
- GitHub prerelease V1.0.0 published 2026-06-14.

These surfaces use the same semantic label around different dates and scopes. They should not be silently collapsed into one current release statement.

## Historical strong claims

CHANGELOG.md and RELEASE_NOTES.md contain strong historical statements such as:
- complete maturity;
- 56/56 methods;
- 96/96 checklist;
- full custody language;
- broad freestanding invariants.

Current governance already requires historical claims to be tied to their original evidence and not promoted to current HEAD automatically.

A particularly important historical phrase in RELEASE_NOTES.md encodes hardware absence using a numeric interpretation of TOKEN_VAZIO. Current repository governance explicitly says TOKEN_VAZIO is not a universal numeric value. Therefore that release note must be treated as a historical snapshot whose semantics require review before reuse.

This cut does **not** rewrite the historical release notes. The correct repair is an errata/supersession record when provenance is closed.

## Release-ready evidence chain

A future release should, as applicable, bind:

~~~text
tag
-> exact source commit
-> toolchain identity
-> required CI gates
-> artifact build
-> artifact hashes
-> SBOM/provenance if produced
-> signing identity
-> install/runtime evidence when claimed
-> release notes
-> known gaps
~~~

A release may deliberately omit device/runtime proof, but then its notes must state that boundary.

## Versioning policy gap

No current repository-wide semantic-versioning policy was observed in the surveyed canonical routers.

Recommended decision object:
- what increments major/minor/patch;
- whether research snapshots use prerelease identifiers;
- artifact compatibility policy;
- schema-version compatibility;
- deprecation window;
- release branch/tag rules.

Until such a policy is adopted, version semantics beyond the historical artifact are REVIEW_REQUIRED.

## Release assets

The observed GitHub V1.0.0 prerelease has no attached assets. Repository source archives generated automatically by GitHub are not equivalent to a project-produced APK/ELF/evidence bundle.

R3 = ⟨F_ok: GitHub release, CHANGELOG and release-note scopes separated; F_gap: one v1.0.0 label spans inconsistent dates/claims and no current versioning policy was observed; F_next: create an explicit release policy/errata only after linking historical claims to revision-bound evidence⟩.
