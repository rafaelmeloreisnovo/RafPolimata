# ZIPRAF Ω Hardware Capability Fabric V1

**Governance binding:** `CLOSURE_L11` — explicit unknown-state markers remain gaps; this binding does not promote them.

**State:** `IMPLEMENTED_SOURCE / LOCAL_GATES_RECORDED`  
**Languages:** ASM + C11 + Rust `no_std`  
**Role:** hardware-capability routing, permission enforcement and adapter registry; **not** the ZIPRAF codec itself.

This is the canonical RafPolimata directory for a compact execution layer between ZIPRAF workloads and hardware-specific implementations without changing primitive mathematics or ZIPRAF serialization.

## Authority split

```text
RafPolimata/native/zipraf_hw_fabric_v1 = capability + permission + dispatch math
rafaelmeloreisnovo/ZIPRAF_CORE         = binary/ABI authority
rafaelmeloreisnovo/ZIPRAF_OMEGA_FULL   = codec/round-trip authority
native/raf_hash_fabric_v1              = related hash/Merkle control precedent
```

Pipeline:

```text
ZIPRAF request
 -> permission gate
 -> primitive descriptor
 -> capability geometry
 -> deterministic dispatch plan
 -> validated typed adapter
 -> evidence receipt
```

V1 implements the control plane and deliberately leaves actual cryptographic adapters evidence-gated.

## Primitive registry

SHA-256, SHA-512, SHA3-256, BLAKE2s, BLAKE2b, BLAKE3, ChaCha20, Poly1305, XChaCha20-Poly1305, Ed25519, X25519, CRC32C and MD5.

MD5 is permanently `LEGACY_INSECURE`; CRC32C is a checksum. Neither is promoted as a modern security primitive.

## Hardware mathematics

For primitive word width `w`, vector width `v`, and maximum independent parallelism `p`:

```text
lanes = max(1, min(floor(v / w), p))
blocks = max(1, ceil(bytes / block_or_chunk))
C_q16 = ceil(blocks * 1.0_Q16 / lanes)
```

V1 uses neutral coefficients. `C_q16` is workload geometry only, never ns/op, cycles/byte, MB/s or a speed claim. Measured coefficients require architecture-bound receipts.

## Mixed FS/capability permissions

Low bits preserve `READ|WRITE|EXEC`; extended rights add `VERIFY|SIGN|DERIVE|EXPORT|ADMIN|PRIVATE|NETWORK|METADATA|BENCH`.

```text
effective = allow & ~deny
```

Deny wins. `ADMIN` is not a universal cryptographic bypass.

## BBS bootstrap

```sh
sh install/bbs_install.sh
```

The bootstrap renders a colored terminal/BBS banner, probes toolchains, runs the C selftest, compiles Rust when `rustc` exists, and syntax-checks ARMv7/AArch64/x86_64 ASM when the assembler exists. It performs no package-manager or privileged operation.

## Evidence boundary

`SOURCE != EXECUTION != EVIDENCE != CLAIM`.

Local pre-commit gates established C routing/permission selftest PASS under cc and clang, ASM syntax PASS for ARMv7/AArch64/x86_64, and JSON parse PASS. Rust compilation remains `TOKEN_VAZIO` in that executor because `rustc` was unavailable. Crypto KATs, physical performance and side-channel claims remain open.
