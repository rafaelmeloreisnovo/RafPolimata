# RAFAELIA Vertical Slice V1 — RafPolimata evidence adapter

**Canonical authority:** `rafaelmeloreisnovo/Mapa` draft PR #95.  
**Role:** deterministic evidence lab; never authority for automatic claim promotion.  
**Policy:** `claim_allowed=false`.

## Priority matrix cells

```text
N04×V09 → provenance
N08×V09 → execution
N09×V09 → evidence
N10×V12 → indexing
N12×V12 → selective feedback
```

For each cell RafPolimata must receive source IDs, exact hashes, claim IDs and falsifiers, then emit:

- deterministic test result;
- metric with derivation;
- immutable receipt;
- errata/contradictions;
- limitations;
- `F_ok`, `F_gap`, `F_next`.

The current Mapa reference receipt records 9/9 sources and 3/3 bounded structural claims in a container runtime. This repository must not rename that result as Android, independent replication, scientific validity or production readiness.

A future RafPolimata execution should append a distinct receipt whose `origin`, runtime, command, input hash and commit are explicit. Missing evidence remains `TOKEN_VAZIO`.
