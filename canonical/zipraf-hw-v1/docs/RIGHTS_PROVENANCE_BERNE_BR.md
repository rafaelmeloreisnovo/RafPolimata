# Rights, provenance and authorship — Berne/Brazil boundary

**Purpose:** auditable authorship/provenance metadata for original expression in this directory. This document is not legal advice and does not guarantee enforceability in any jurisdiction.

## Invariants

```text
SOURCE != AUTHORSHIP != COPYRIGHT != LICENSE != PATENT != TRADEMARK
PUBLIC_REPOSITORY != PUBLIC_DOMAIN
NOTICE != REGISTRATION
HASH != OWNERSHIP_JUDGMENT
UPSTREAM_LICENSE_CONTROLS_IMPORTED_CODE
```

## Berne

The Berne Convention uses automatic/formality-free protection as a baseline in contracting states. A copyright notice can help provenance and identification, but is not what creates Berne protection.

## Brazil

Computer programs are governed specifically by Lei 9.609/1998 together with applicable copyright rules. Protection does not depend on registration; voluntary registration can still have evidentiary/transactional value. Lei 9.610/1998 distinguishes protected expression from ideas, procedures, methods and mathematical concepts as such.

## File header policy

For new original source authored within this project:

```text
Copyright (c) 2024-2026 Rafael Melo Reis.
Provenance: <repository>@<commit>:<path>
License: follow repository/per-file license; no relicensing implied.
Upstream: list imported/adapted sources and their licenses.
```

Do not stamp this header onto third-party code merely because it is present in the repository. Do not delete upstream copyright/license notices.

## Evidence bundle

For a release-worthy artifact preserve: source commit, path, build recipe, compiler/toolchain versions, source hash, artifact hash, KAT/test receipt, authorship/provenance note, upstream license inventory and release timestamp.
