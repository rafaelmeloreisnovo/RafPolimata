# ZIPRAF Hardware / Crypto / FS Canonical V1

**State:** `IMPLEMENTED_LOCAL_CORE / CLAIM_BLOCKED_PENDING_REPO_CI_AND_PHYSICAL_RECEIPTS`  
**Authority:** RafPolimata integrates; it does not replace producer repositories.

## Authority map

```text
RafPolimata                  -> orchestration, math contract, policy, docs
ZIPRAF_OMEGA_FULL            -> container/runtime writer-reader authority
ZIPRAF_CORE                  -> binary ABI authority
BLAKE3 (RMR branch/PR line)  -> crypto runtime/provider matrix evidence
hardware/device              -> physical performance authority
```

## What this directory adds

- C freestanding-compatible math/control core with no allocator and no libc dependency;
- Rust `no_std` mirror for the pure math primitives;
- ASM masked-patch primitives for x86-64, AArch64 and ARMv7;
- evidence-driven backend selection by measured p95 cycles/useful byte;
- 15-algorithm crypto registry, separating algorithm/provider/architecture;
- ZIPRAF-FS mixed permission/custody contract;
- ANSI BBS-style operator shell;
- Berne/Brazil provenance boundary without claiming automatic relicensing or legal immunity.

## Crypto set

BLAKE3, MD5 (legacy only), SHA-1 (legacy only), SHA-256, SHA-512, SHA3-256, BLAKE2b-512, HMAC-SHA256, HMAC-SHA512, HKDF-SHA256, PBKDF2-HMAC-SHA256, Ed25519, X25519, ChaCha20-Poly1305 and AES-256-GCM.

These names do not mean the algorithms are reimplemented here. Standard algorithms stay behind validated providers; this directory only defines selection, capability and custody contracts.

## Build smoke

Hosted selftest:

```text
cc -std=c11 -Wall -Wextra -Werror -Iinclude src/zipraf_hw.c tests/selftest.c -o /tmp/zipraf-hw-selftest
/tmp/zipraf-hw-selftest
```

Freestanding object gate:

```text
cc -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -Iinclude -c src/zipraf_hw.c -o /tmp/zipraf_hw.o
```

Rust mirror:

```text
cd rust && cargo test
```

## Claim boundary

`math PASS != crypto KAT PASS != provider PASS != physical performance PASS != production-security certification`.

Physical ARMv7/AArch64 measurements and independent external security review remain separate gates.
