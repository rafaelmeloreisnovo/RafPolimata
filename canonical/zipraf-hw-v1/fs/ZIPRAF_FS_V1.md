# ZIPRAF-FS V1 — mixed permission and custody contract

ZIPRAF-FS is a **container policy**, not a kernel filesystem claim. It maps archive entries to deterministic metadata, access policy and custody receipts while the host filesystem remains authoritative for physical I/O.

## Permission word

Use a 32-bit policy word:

```text
bits  0..8  = POSIX-like rwx: owner/group/other
bits  9..15 = service capabilities
bits 16..23 = object class
bits 24..31 = policy version / reserved
```

Service capabilities:

```text
VERIFY  = 1<<9   read bytes + verify digest, no mutation
APPEND  = 1<<10  create successor generation only
EXPORT  = 1<<11  materialize an authorized derivative
KEYUSE  = 1<<12  request key operation through provider; key bytes never stored here
AUDIT   = 1<<13  read receipts/provenance
ADMIN   = 1<<14  policy update through successor record
SEALED  = 1<<15  deny plaintext export unless an explicit higher-level policy allows it
```

`APPEND` never implies overwrite. A changed object receives a new generation and a parent digest. Deletion is represented as a tombstone receipt, not silent history erasure.

## Object classes

`DATA`, `MANIFEST`, `INDEX`, `DICTIONARY`, `RECEIPT`, `KEYREF`, `POLICY`, `TOMBSTONE`.

`KEYREF` contains provider/key identifiers only. Secrets, passwords, raw private keys and access tokens are forbidden in manifests and receipts.

## Integrity chain

For generation `g`:

```text
entry_digest = H(canonical_header || payload)
receipt_g = H(receipt_{g-1} || entry_digest || policy_digest || timestamp || producer_ref)
```

`H` must be selected from the current registry. MD5/SHA-1 may only appear as legacy compatibility fields and never as the sole custody digest.

## Fail-closed rules

- missing provider => `TOKEN_VAZIO_PROVIDER`;
- missing permission => deny;
- missing parent for mutation => deny;
- digest mismatch => quarantine;
- unsupported algorithm => deny;
- unknown physical write granule => performance claim stays `TOKEN_VAZIO`;
- no key bytes inside ZIPRAF-FS metadata.
