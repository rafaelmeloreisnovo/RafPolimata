# Ω LADDER — remaining transitions

| Gate | State after V4.2 | Promotion condition |
|---|---|---|
| Ω0 Source identity | PASS | V4 ZIP SHA-256 exact |
| Ω1 V4.1 hardening | PASS | parent patch SHA + host regression gates |
| Ω2 ARMv7 assemble | PASS 9/9 | clang ARMv7-A, warnings fatal |
| Ω3 ARMv7 static link | PASS 9/9 | LLD, no undefined, ELF32 ARM EABI5 |
| Ω4 NDK dual ABI | TOKEN_VAZIO | `librafaelia_core.so` for armeabi-v7a + arm64-v8a |
| Ω5 ARM32 physical run | TOKEN_VAZIO | nine static binaries exit 0 on Termux ARM32 |
| Ω6 JNI device load/call | TOKEN_VAZIO | library loaded and native calls return expected receipt |
| Ω7 real GPU compute | TOKEN_VAZIO | OpenCL XOR kernel executes and result verifies |
| ↑Ω terminal | BLOCKED BY Ω4–Ω7 | `CLAIM_ALLOWED=true`, `STATUS=OMEGA_PHYSICAL_PASS`, `PASS OMEGA_FINAL` |

The final script never substitutes a cross-build for a physical execution.
