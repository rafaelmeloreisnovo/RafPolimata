# Provenance & Receipts Contract v1

Purpose: create a reproducible chain of custody for authorship/origin claims, license boundaries, releases and commercial authorizations without pretending that a hash alone proves legal ownership.

## Receipt schema

```yaml
receipt_version: 1
receipt_id: RAFPOLIMATA-...
observed_at: RFC3339
repository: owner/repo
branch: ref
commit_sha: sha
tree_sha: sha
paths: []
content_digests:
  sha256: TOKEN_VAZIO
  blake3: TOKEN_VAZIO
  md5_legacy: TOKEN_VAZIO
origin:
  author_or_source: TOKEN_VAZIO
  upstream_repository: TOKEN_VAZIO
  upstream_commit: TOKEN_VAZIO
  upstream_license: TOKEN_VAZIO
license:
  effective_license: TOKEN_VAZIO
  notices: []
commercial_authorization:
  status: NOT_APPLICABLE|TOKEN_VAZIO|EXECUTED|EXPIRED|REVOKED_FOR_BREACH
  instrument_ref: TOKEN_VAZIO
evidence_refs: []
gaps: []
parent_receipt: TOKEN_VAZIO
signature:
  status: TOKEN_VAZIO
  method: TOKEN_VAZIO
  signer: TOKEN_VAZIO
```

## Digest policy

- SHA-256 or a comparably modern digest may serve as the baseline content-integrity digest.
- BLAKE3 may be recorded when the exact implementation/tool and its provenance are known.
- MD5 may be retained only for legacy interoperability/historical comparison. It MUST be paired with a modern digest and MUST NOT be the sole security, integrity, authentication, or anteriority/priority anchor.
- A missing digest is `TOKEN_VAZIO`; never fabricate it.

## Chain-of-custody rules

1. Receipt creation is append-only.
2. Every receipt points to exact repository/ref/commit/tree/path when available.
3. A corrected receipt supersedes, but does not delete, the prior receipt.
4. Upstream origin and license evidence are independent from RAFAELIA authorship claims.
5. Commercial contracts must point to the exact covered material/commit or an unambiguous version set.
6. A Git commit establishes a verifiable repository state; it does not alone establish legal authorship, originality or ownership.
7. Cryptographic signatures strengthen attribution only to the extent signer identity/key custody are themselves established.
8. Self-referential receipt hashes are not required; the receipt commit SHA is recorded externally or by a later receipt.

## Evidence ladder

`content -> digest -> Git blob/tree -> commit -> parent history -> tag/release -> signed receipt -> independent timestamp/registration (when used)`

Each layer is evidence. No layer is described as an absolute guarantee.
