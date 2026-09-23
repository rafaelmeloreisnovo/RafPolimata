# Receipt — ZIPRAF Hardware / Crypto / FS Canonical V1

- timestamp_local: 2026-09-23T06:38:00-03:00
- source_intent: RafPolimata + Google Drive ZIPRAF canonical hardware/crypto/fs layer
- compiler: clang version 17.0.0
- claim_allowed: false

## Observed gates

- C hosted selftest: PASS (5/5)
- C freestanding compile with -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror: PASS
- freestanding C object undefined symbols: 0 observed by nm -u
- x86-64 ASM assemble: PASS
- ARMv7 ASM cross-assemble: PASS
- AArch64 ASM cross-assemble: PASS
- crypto registry JSON parse: PASS
- Rust cargo test: NOT_RUN (cargo unavailable in this execution environment)
- physical ARMv7/AArch64 runtime: TOKEN_VAZIO
- external security audit: TOKEN_VAZIO

## Boundaries

IMPLEMENTED_UNTESTED != PASS
MATH_PASS != CRYPTO_KAT_PASS != PROVIDER_PASS != DEVICE_PASS
MD5/SHA1 = compatibility_only

## Next gate

Run repository CI on exact branch head, then execute device-bound receipts without promoting performance from cross-assembly alone.
