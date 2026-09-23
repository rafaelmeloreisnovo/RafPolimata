# ZIPRAF Hardware Service V1

**State:** `IMPLEMENTED_LOCAL / CI_TOKEN_VAZIO / PHYSICAL_MULTI_ISA_TOKEN_VAZIO`  
**Canonical implementation owner:** `rafaelmeloreisnovo/RafPolimata`  
**Canonical documentation route:** Google Drive `ZIPRAF` → `ZIPRAF_HW_SERVICE_CANONICAL_V1`  
**Model:** portable-first C11 + Rust `no_std` mirror + measured-only ASM.

ZIPRAF Hardware Service V1 is a thin canonical integration layer. It deliberately does **not** duplicate mature RafPolimata subsystems. It binds four existing authorities into one auditable service contract:

```text
crypto/work scheduling -> native/raf_hash_fabric_v1/
hardware capability    -> Apkc/hw_dispatch.h + freestanding/
filesystem policy      -> freestanding/include/raf_fs_caps.h + fs/zipraf_cap_fs.h
operator surface       -> tools/rafbbs/ + bbs/zipraf_hw_bbs.c
```

The project invariant is:

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
```

## Why this module exists

Hardware is heterogeneous. A cryptographic or archival backend that is fast on AVX2 may be wrong for ARMv7; an ASM path that looks clever may be slower than compiler-generated scalar code; a hardware capability bit does not prove a backend is correct. The module therefore selects only among **compatible** backends and only after evidence gates.

The mathematical selection contract is a bounded fixed-point cost function:

```text
score(b) = Σ_i weight_i * normalized_cost_i(b)
Σ_i weight_i = 1.0 in Q16.16
lower score is better
```

Current dimensions are latency, cycles, inverse throughput, code size, stack cost and energy proxy. A required unknown measurement makes the score `TOKEN_VAZIO`, never zero. Compatibility and validation are gates before score comparison.

## Directory

```text
native/zipraf_hw_service_v1/
├── README.md
├── SPEC.md
├── LEGAL_BERNE_IP_PROFILE.md
├── Makefile
├── install.sh
├── include/zipraf_hw_service.h
├── c/zipraf_hw_service.c
├── rust/lib.rs
├── asm/README.md
├── fs/zipraf_cap_fs.h
├── bbs/zipraf_hw_bbs.c
├── manifests/crypto_registry_v1.json
├── tests/selftest.c
└── receipts/
```

## Cryptographic registry: 13 identities

The registry names algorithms; it does not silently invent implementations or alter standards:

1. SHA-256
2. SHA-512
3. SHA3-256
4. BLAKE2s-256
5. BLAKE2b-512
6. BLAKE3-256
7. ChaCha20
8. ChaCha20-Poly1305
9. Ed25519
10. HMAC-SHA256
11. AES-128-GCM
12. AES-256-GCM
13. MD5 — `LEGACY_ONLY`

`MD5` exists strictly for legacy identification/interoperability. It is forbidden by this module for signatures, custody roots, collision-resistant identity or new protocol design.

Each real implementation must independently supply: exact source/version/commit, license/SPDX, provenance, KAT/interoperability evidence, constant-time/security analysis where applicable and backend-specific performance receipts.

## Mixed-access filesystem policy

`fs/zipraf_cap_fs.h` composes:

```text
POSIX-like owner/group/other base rights
+ explicit ACL allow/deny
+ object-scoped capability token with expiry
+ immutable / append-only / noexec object flags
```

Policy rules:

- explicit ACL `DENY` wins, including over a capability token;
- capability scope is one object ID and can expire;
- `IMMUTABLE` denies write/append/admin mutations;
- `APPEND_ONLY` denies generic overwrite while retaining explicitly granted append;
- `NOEXEC` denies execute;
- the policy core performs no privilege escalation and is **not** an operating-system filesystem by itself.

## BBS/operator console

A small ANSI front-end is included for installation smoke tests and registry inspection. It is intentionally subordinate to existing `tools/rafbbs/`, which remains the richer operator console.

```sh
make -C native/zipraf_hw_service_v1 test
make -C native/zipraf_hw_service_v1 freestanding
make -C native/zipraf_hw_service_v1 bbs
/tmp/zipraf_hw_bbs crypto
```

`install.sh` defaults to `$HOME/.local`; it does not require root, network access or permission escalation.

## ASM boundary

ASM is an optimization backend, never the semantic oracle. No architecture-specific path is promoted until:

```text
portable scalar golden
-> official/reference vectors
-> bit-for-bit equivalence
-> architecture capability proof
-> benchmark on exact artifact/device
-> regression/rollback receipt
```

If no measured gain exists, C/Rust remains canonical. This is deliberate: fewer ISA-specific bytes can be faster, safer and easier to audit than speculative hand-written ASM.

## Legal/IP boundary

The original integration code and documentation can carry authorship, copyright and license notices, but a header is not a magical legal shield. The module preserves exact provenance, source identity and third-party license boundaries. See `LEGAL_BERNE_IP_PROFILE.md` and the repository legal pack.

## Current evidence state

Local creation gate for this module may prove only the commands recorded in `receipts/`. It does not promote:

- physical ARMv7/AArch64/x86-64/RISC-V performance;
- BLAKE3/ChaCha/Ed25519 production implementations;
- constant-time behavior;
- independent security review;
- ZIPRAF universal compression superiority;
- legal enforceability in every jurisdiction.

Those remain separately gated.
