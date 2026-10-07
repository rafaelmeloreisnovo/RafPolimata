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
- pure PREHOT/HOT benchmark core with 31-sample integer order statistics;
- deterministic seeded Random-Plays ordering for up to 8 variants;
- 11-layer / 32-factor interference catalog and 16 quality gates;
- explicit speed-group/unit classification and 152-byte `ZBR1` receipt codec;
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
clang -std=c11 -O0 -ffreestanding -fno-builtin -fno-stack-protector \
  -fno-vectorize -fno-slp-vectorize -Wall -Wextra -Werror -Iinclude \
  -c src/zipraf_hw.c -o /tmp/zipraf_hw.o
clang -std=c11 -O0 -ffreestanding -fno-builtin -fno-stack-protector \
  -fno-vectorize -fno-slp-vectorize -Wall -Wextra -Werror -Iinclude \
  -c src/zipraf_bench_core.c -o /tmp/zipraf_bench_core.o
nm -u /tmp/zipraf_hw.o /tmp/zipraf_bench_core.o
```

Pure benchmark route:

- contract: `include/zipraf_bench_core.h`;
- implementation: `src/zipraf_bench_core.c`;
- factor/quality catalog: `registry/benchmark_interference_catalog.v1.json`;
- deterministic vector: `vectors/benchmark_prehot_hot_v1.json`;
- detailed protocol: `docs/PURE_BENCHMARK_LAB_V1.md`.

The canonical lane is compiled at `-O0` with vectorizers disabled. An `-O2`
run is used only as semantic adversarial parity; it is not benchmark evidence.
The pure core never reads a clock or OS state. Physical collectors remain
adapters and must populate factor/quality observations explicitly.

Rust mirror:

```text
cd rust && cargo test
```

## Claim boundary

`math PASS != crypto KAT PASS != provider PASS != physical performance PASS != production-security certification`.

Physical ARMv7/AArch64 measurements and independent external security review remain separate gates.
