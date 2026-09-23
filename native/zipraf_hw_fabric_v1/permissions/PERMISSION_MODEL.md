# ZIPRAF HW Fabric V1 — mixed FS/capability permissions

**Status:** `REFERENCE / IMPLEMENTED_CONTROL_BITS / ENFORCEMENT_SCOPE=router_only`

The permission plane resembles a filesystem without pretending Unix mode bits are sufficient for cryptographic authority.

Base rights are `READ | WRITE | EXEC`. Extended rights are `VERIFY | SIGN | DERIVE | EXPORT | ADMIN | PRIVATE | NETWORK | METADATA | BENCH`.

```text
effective = allow & ~deny
```

Deny wins. There is no implicit root bypass in the fabric. Host OS privilege is a separate layer.

Sensitive operations require:
- sign: `READ + EXEC + SIGN + PRIVATE`;
- key exchange/derivation: `READ + EXEC + DERIVE + PRIVATE`;
- verification: `READ + EXEC + VERIFY`;
- encryption/decryption: `READ + WRITE + EXEC + PRIVATE`.

`ADMIN` does not silently imply `SIGN`, `PRIVATE` or `EXPORT`.

This is an authorization surface, not a key vault. Secret storage/erasure, side-channel hardening, process isolation and device attestation require independent gates.
