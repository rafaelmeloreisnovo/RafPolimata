# Third-Party Cryptography & License Register V1

**Status:** `AUDIT / PARTIAL`  
**Scope:** cryptographic algorithms, reference implementations and related components relevant to RAF Hash Fabric V1.

## Rule

Algorithm/specification identity, code authorship, copyright license, patent/trademark status and runtime validation are separate facts. Referencing or independently implementing an algorithm does not transfer ownership of the algorithm or upstream implementation.

## Register

| ID | Component/reference | Relationship | Upstream/source | License/terms observed | Vendored? | Current project claim |
|---|---|---|---|---|---|---|
| CRYPTO-001 | BLAKE3 | planned/locally-tested portable oracle for RAF Hash Fabric | BLAKE3-team/BLAKE3 official repository/spec/test vectors | official upstream repository publishes implementations under `CC0-1.0 OR Apache-2.0 OR Apache-2.0 WITH LLVM-exception` | No in PR #321 at this cut | upstream identity/reference only; local working-container implementation is not yet persisted |
| CRYPTO-002 | SHA-256 | standard primitive candidate/reference | NIST FIPS 180 family | standard/specification terms differ from implementation licenses | No new vendoring in this module | algorithm reference only |
| CRYPTO-003 | SHA-3 | standard primitive candidate/reference | NIST FIPS 202 family | standard/specification terms differ from implementation licenses | No new vendoring in this module | algorithm reference only |
| CRYPTO-004 | Ed25519 | signature primitive candidate/reference | RFC/implementation-specific upstreams | implementation license must be checked before copying/vendorizing | No new vendoring in this module | concept/reference only |

## BLAKE3 control boundary

The RAF Hash Fabric architecture may implement/adapt BLAKE3-compatible computation, but must preserve:

- BLAKE3's specified mathematical behavior for any compatibility claim;
- official known-answer-vector equivalence before promotion;
- upstream notices/licenses for copied or vendorized upstream source;
- clear authorship boundaries for independently written adapter/scheduling code;
- no claim of ownership over the BLAKE3 algorithm, specification, project name or upstream source.

## GPL-scoped repository material

The repository also contains at least one document with an explicit `SPDX-License-Identifier: GPL-3.0-only`. Therefore the new RREL-1.0 policy is intentionally scoped to original material expressly marked with it and does **not** silently convert existing GPL-scoped material.

## Required before release

For every distributed crypto implementation:

```text
exact source/version/commit
+ license/SPDX
+ provenance (copied, adapted, independent implementation)
+ notices/source obligations
+ KAT/interoperability evidence
+ security boundary
+ export/sector review where applicable
```

Unresolved entries remain `TOKEN_VAZIO`.
