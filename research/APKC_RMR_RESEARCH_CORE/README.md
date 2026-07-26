# APKC–RMR Research Core

**Coupling ID:** `APKC-RMR-RESEARCH-CORE-V1-20260726`  
**Status:** `RESEARCH_SOURCE_AVAILABLE / NONCOMMERCIAL`  
**Commercial use:** prohibited unless a separate written commercial license is executed.  
**Legal status:** `DRAFT_FOR_COUNSEL_REVIEW`.

## Purpose

This isolated nucleus connects the deterministic ApkC structural receipt to the RMR/BLAKE3 custody model without relicensing or copying third-party code.

```text
compiler source + public interface
+ normative comments
+ compiled policy stubs
+ technical documentation
+ software/documentation licenses
+ commercial and high-risk policy
→ per-file SHA-256 + Git blob provenance
→ SHA-256 artifact root
→ generated coupling header
→ freestanding compilation
→ audit receipt
```

A one-bit change in any sealed artifact invalidates the authorized build generation. An intentional change must be reviewed, versioned and resealed.

## What “comment-bound” means

C comments normally do not alter machine semantics. In this nucleus they become normative inputs because the build contract requires markers such as:

```text
Coupling-ID
Contract-Role
License-Role
Normative comment
```

The verifier checks those markers, each exact SHA-256 and the Git blob identity before generating the header consumed by the compiler. Removing or altering a required comment therefore blocks compilation.

## Compiled stubs

Unavailable capabilities are represented inside the C unit as explicit states:

```text
FORBIDDEN
TOKEN_VAZIO
NOT_AUTHORIZED
```

The compiled table covers commercial licensing, high-risk production, APK signing, installation, package launch and regulatory approval. No stub may silently return success.

## Repository boundaries

- `Apkc/`, APK/DEX/ELF validators and existing workflows remain under their existing terms.
- BLAKE3 remains the public cryptographic integrity primitive in its own repository and under its own licenses.
- RMR remains the operational custody/provenance layer.
- This directory licenses only original material expressly listed in `integrity/coupling.lock.json`.
- No third-party work is relicensed by this nucleus.

## Licensing layers

1. **Software and verifier:** PolyForm Noncommercial License 1.0.0, incorporated by reference.
2. **Documentation:** Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
3. **Commercial use:** no grant; prior communication, due diligence and an executed commercial agreement are mandatory.
4. **High-risk use:** no production authorization under the research grant.
5. **Trademarks, names and seals:** no trademark license.
6. **Third-party components:** their original licenses control.

## Core rule

```yaml
communication_received: does_not_equal_authorization
commercial_license_executed: required
academic_noncommercial_use: permitted_subject_to_license
claim_allowed: false
```

## Coupled build

```sh
make verify
make test
make coupled-build
```

The order is mandatory:

```text
verify → generate header → compile
```

The current sealed artifact root is:

```text
150105a1294fbbfa2430afd18e681da4599989eaf9b41611b521778238485046
```

BLAKE3 remains an additional release seal only when a verified canonical implementation and source commit are recorded; absence remains `TOKEN_VAZIO`.

## No production claim

This nucleus is governance and research infrastructure. It does not prove APK signing, installation, launch, medical safety, regulatory approval, commercial fitness or performance superiority.
