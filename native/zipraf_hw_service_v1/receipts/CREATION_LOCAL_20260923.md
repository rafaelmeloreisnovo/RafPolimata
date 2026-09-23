# ZIPRAF Hardware Service V1 — Local Creation Receipt — 2026-09-23

**State:** `LOCAL_C_PASS / C_FREESTANDING_OBJECT_PASS / BBS_PASS / JSON_PASS / RUST_TOKEN_VAZIO_TOOLCHAIN / MULTI_ISA_PHYSICAL_TOKEN_VAZIO`

## Environment

`cc`: cc (Debian 14.2.0-19) 14.2.0  
`uname`: Linux 56fb4ae868a1 6.18.44 #1 SMP Sat Sep 12 15:35:21 UTC 2026 x86_64 GNU/Linux

## Commands actually executed

`make clean` -> PASS  
`make test` -> PASS (exit 0)  
`make freestanding` -> PASS (C11 objects, `-ffreestanding -fno-builtin`)  
`make bbs` -> PASS  
`/tmp/zipraf_hw_bbs crypto` -> PASS; 13 registry entries rendered  
`python3 -m json.tool manifests/crypto_registry_v1.json` -> PASS  
`command -v rustc` -> no path observed; Rust runtime/build evidence remains TOKEN_VAZIO

## Artifact SHA-256

`/tmp/zipraf_hw_selftest` = `2947e3c22b10d57b2bc35c70853cfccabd93a0ba33ab550c6c512c8d82f21d5c`  
`/tmp/zipraf_hw_service.o` = `c054407b5905785abbd43823375bc79f052897ffca47b960966d8be6fca08b92`  
`/tmp/zipraf_hw_bbs` = `5fd859e8977ed33946f09800e352afb5f536da1b49d8df393de9b8b1e9c7acd3`  
source-tree rolling SHA-256 = `86cef87c70e97bc15713529aa6cc7e333801f9c338dd1078f2a39d5a2658fd79`

## Proven local assertions

- registry has exactly 13 algorithm identities;
- MD5 default state is `LEGACY_ONLY`;
- weighted Q16 score refuses incomplete required metrics;
- capability/validation/security eligibility gates behave deterministically;
- append-only object denies generic write;
- ACL explicit deny wins even over a matching capability token;
- capability scope and expiry are enforced by the policy core;
- immutable flag blocks mutation;
- C11 core compiles as a freestanding object with the observed host compiler;
- ANSI BBS adapter builds and enumerates the registry.

## Not proven

- physical ARMv7/AArch64/x86-64/RISC-V performance;
- Rust `no_std` compilation in this environment;
- production implementation of BLAKE3/ChaCha20/Ed25519/AES;
- constant-time execution;
- independent cryptographic/security review;
- OS filesystem enforcement;
- legal enforceability in any particular dispute/jurisdiction;
- universal ZIPRAF compression or performance superiority.

`claim_allowed=false` outside the exact local gates above.
