# ZIPRAF HW Fabric V1 — μReceipt — 2026-09-23

**Repository:** `rafaelmeloreisnovo/RafPolimata`  
**Review branch:** `feature/zipraf-hw-fabric-v1-20260923`  
**Observed branch head before this receipt:** `8a422f615f9560058585fba18e00e334ba76e302`  
**Base main:** `a0190f89bce25cb4a9eef278fac48819c8c3f41a`  
**State:** `IMPLEMENTED_SOURCE / LOCAL_GATE_EVIDENCE / CI_PENDING`  
**claim_allowed:** `false`

## Material delta

Created canonical module `native/zipraf_hw_fabric_v1/` with:

- deterministic hardware-capability geometry and Q16 workload score;
- mixed FS-like RWX + cryptographic capability permissions with deny precedence;
- typed C adapter ABI for externally validated cryptographic implementations;
- C11 control-plane implementation;
- Rust `no_std` mirror;
- ARMv7, AArch64 and x86_64 diagnostic ASM paths;
- 13-family primitive registry;
- hardware-profile registry;
- BBS-style terminal bootstrap;
- scoped copyright/license/Berne-orientation provenance notices;
- CI source gates and repository documentation routes.

## Key Git object identities

```text
4638b145d0b7c144b699b91522371b47c142bd6a  include/zipraf_hw_fabric.h
b200b229f393432b777a6b4cffd2a0631e0322a0  include/zipraf_crypto_adapter.h
41606cfef7a2ed4c444fecf3e67b2d26b668f102  c/zipraf_hw_fabric.c
306e08bdc0662f4400a35db728dc813efc57c85a  rust/lib.rs
96759f2808b64601bb018be7eaab015f2fd13623  tests/selftest.c
0435c5366b1847175d0a5415e1f32383a16482e3  registry/primitive_registry.v1.json
0a268978f476adae3398680f617cabbcac2c6c26  profiles/hardware_profiles.v1.json
73fc77dd50887a2262fbb998f14432babee2ec57  install/bbs_install.sh
8e1e967a4b9f2b36c18f8f445406d0ddf2e5909a  .github/workflows/zipraf-hw-fabric-v1.yml
```

These are Git blob identities, not security certifications or external timestamps.

## Executed local gates

- C selftest using `cc`: **PASS**
- C selftest using `clang`: **PASS**
- ARMv7 ASM syntax/object generation: **PASS**
- AArch64 ASM syntax/object generation: **PASS**
- x86_64 ASM syntax/object generation: **PASS**
- primitive/profile/license JSON parsing: **PASS**
- BBS bootstrap: **PASS**
- Rust compile in local executor: **TOKEN_VAZIO** because `rustc` was unavailable

Local execution occurred before persistence; the key repository blobs above bind the persisted source identity. Exact-commit CI is still required before promoting the branch to a current-commit PASS.

## Security and cryptographic boundary

The registry names SHA-2/SHA-3/BLAKE2/BLAKE3/ChaCha20/Poly1305/XChaCha20-Poly1305/Ed25519/X25519/CRC32C/MD5, but V1 does not implement those primitives itself. Adapter state is `TOKEN_VAZIO` until a vetted implementation is connected and passes official/reference vectors and equivalence gates.

`MD5 = LEGACY_INSECURE`; `CRC32C = CHECKSUM`.

## Authority boundary

```text
RafPolimata module = capability / permission / dispatch control
ZIPRAF_CORE        = binary / ABI codec authority
ZIPRAF_OMEGA_FULL  = codec / round-trip authority
```

## R3

`F_ok = module persisted on review branch; C+ASM local gates passed; adapters/profiles/permissions/legal scope/CI routed.`

`F_gap = exact-commit CI; Rust compile; real crypto adapters/KATs; physical ARM/x86/RISC-V/WASM benchmarks; side-channel scope; ZIPRAF codec integration test; counsel review.`

`F_next = open draft PR, let exact-commit CI execute, then add standard-vector adapters one family at a time without altering primitive mathematics.`
