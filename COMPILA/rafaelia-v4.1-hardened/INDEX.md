# RAFAELIA V4.1 Hardened — RafPolimata / COMPILA

Append-only hardening and reconstruction bundle.

- `SOURCE_REF.md` — content-addressed identity of the exact V4 input.
- `PATCH_PARTS/00.part` … `04.part` — ordered fragments of the deterministic V4→V4.1 unified patch.
- `MATERIALIZE_AND_VERIFY.sh` — source SHA gate -> patch reassembly/hash gate -> extract -> apply -> fail-closed verification.
- `EVIDENCE_MANIFEST.json` — machine-readable evidence/claim boundary.
- `TEST_RECEIPT.md` — observed reference-environment receipt.
- `HARDENING_NOTES.md` — P0 fixes and open gates.
- `SHA256SUMS` — integrity map for the committed evidence bundle.

Reassembled patch SHA-256: `739d0123f953c3bde2546095788dc2fc62143fcf0e016997b2ce2615378c4c68`.

Invariant: **source identity -> deterministic delta -> verify -> receipt -> claim gate**.

`STATUS=VERIFIED_LIMITED`  
`CLAIM_ALLOWED=false`
