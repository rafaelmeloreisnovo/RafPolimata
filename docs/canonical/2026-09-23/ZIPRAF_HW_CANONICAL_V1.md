# ZIPRAF Hardware / Crypto / FS Canonical V1

**Implementation route:** `canonical/zipraf-hw-v1/`  
**State:** `IMPLEMENTED_LOCAL_CORE / REMOTE_CI_PENDING / PHYSICAL_TOKEN_VAZIO`  
**claim_allowed:** false

This route integrates three existing authorities without replacing them:

- `ZIPRAF_OMEGA_FULL`: container/runtime writer-reader;
- `ZIPRAF_CORE`: binary ABI;
- `BLAKE3` RMR crypto runtime line: algorithm/provider/architecture matrix.

RafPolimata owns the integration contract: hardware-capability math, C/Rust/ASM interfaces, ZIPRAF-FS permission/custody policy, BBS operator surface, provenance boundary and promotion gates.

Local evidence is recorded under `canonical/zipraf-hw-v1/receipts/`.

No physical-performance or production-security claim is promoted from cross-compilation.
