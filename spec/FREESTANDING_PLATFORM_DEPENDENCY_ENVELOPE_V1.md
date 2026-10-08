# Freestanding Platform Dependency Envelope V1

Status: `REFERENCE`
Lifecycle: `ACTIVE_CANDIDATE`
Area: `freestanding | low-level | cross-platform | Android | Linux | macOS | Windows | iPhone`
Owner logic: RafPolimata low-level architecture
Observed base: `main@432d884c76f0229ce96c0bb19ca5004d885a76b6`
Receipt: `receipts/2026-10-08_FREESTANDING_PLATFORM_ENVELOPE_V1.md`
Schema: `schemas/freestanding-platform-envelope.v1.schema.json`

## Intent

Define a bounded envelope for platform-dependent and architecture-dependent development so RafPolimata can route Android, Linux, macOS, Windows and iPhone work without mixing:

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
```

This document is a scaffold and routing contract. It does not claim that every platform currently builds, boots, installs or runs.

## Scope

The envelope covers files or future modules written in:

| Language | Role in envelope | Freestanding boundary |
|---|---|---|
| C | canonical low-level reference and ABI-facing core | allowed for freestanding core when no libc/heap/syscall is imported |
| ASM | exact architecture leaf routines and trap-free kernels | allowed only behind explicit architecture contract |
| Java | Android/JVM host shell, diagnostics and packaging glue | hosted, not freestanding proof |
| shell | orchestration, checks, artifact movement | hosted tooling only |
| Python | generators, validators, receipts and deterministic audits | hosted tooling only |
| Rust | optional safe-hosted tools or `no_std` experiments | `no_std` required before freestanding claim |
| C++ | optional hosted adapters or explicitly freestanding subset | no exceptions/RTTI/stdlib before freestanding claim |
| Kotlin/Multiplatform | Android/iOS/desktop product shell | hosted/app layer; no low-level proof by itself |

## Platform matrix

| Platform | Execution class | Primary architecture axes | Current claim state | Minimum next gate |
|---|---|---|---|---|
| Android | app + native/ELF + optional JVM shell | `armeabi-v7a`, `arm64-v8a`, x86_64 emulator | `TOKEN_VAZIO` for current device runtime unless receipt ties exact APK/ELF/install/log | source -> artifact -> APK -> signature -> install -> launch/dlopen -> receipt |
| Linux | hosted CLI/tools + optional raw syscall | x86_64, aarch64, armv7, riscv64 | source/doc scaffold only in this envelope | compile selected target and record toolchain/exit/output hash |
| macOS | hosted CLI/tools | arm64, x86_64 | source/doc scaffold only in this envelope | compile host tool or mark unsupported with reason |
| Windows | hosted CLI/tools | x86_64, arm64 | source/doc scaffold only in this envelope | compile via chosen toolchain and record ABI assumptions |
| iPhone/iOS | app shell + restricted native boundary | arm64 | `TOKEN_VAZIO` unless Apple toolchain/sign/install evidence exists | define entitlement/signing boundary before implementation claim |

## Dependency classes

| Class | Meaning | Examples | Promotion rule |
|---|---|---|---|
| `L0_FREESTANDING` | no libc, no allocator, no syscall, no runtime | pure C/ASM primitives, caller-owned buffers | may be claimed only after static contract gate |
| `L1_ABI_ADAPTER` | architecture ABI boundary with explicit calling convention | ARMv7/AArch64/x86_64 assembly leaves | requires golden vectors or disassembly/audit receipt |
| `L2_OS_BOUNDARY` | OS trap, syscall, loader or process contract | Linux syscall, Windows API, Android linker | cannot be imported back into L0 |
| `L3_HOST_TOOL` | build/test/generator/audit tool | shell, Python, hosted C/Rust/C++ | evidence of tool execution is not runtime proof |
| `L4_APP_SHELL` | product UI/package wrapper | Android Java/Kotlin, iOS app shell | app success does not validate low-level core without bridge receipt |
| `L5_DISTRIBUTION` | package/sign/release surface | APK, IPA, tar/zip, installer | requires identity, hash and reproducibility notes |

## Architecture axes

Every platform-dependent change should declare:

| Axis | Required value |
|---|---|
| `cpu_arch` | `armv7`, `aarch64`, `x86`, `x86_64`, `riscv64`, `wasm32`, or `TOKEN_VAZIO` |
| `abi` | concrete ABI name, e.g. `armeabi-v7a`, `arm64-v8a`, `sysv-amd64`, `win64`, or `TOKEN_VAZIO` |
| `endianness` | `little`, `big`, `bi`, or `TOKEN_VAZIO` |
| `word_bits` | 32, 64, or `TOKEN_VAZIO` |
| `runtime_class` | one of the dependency classes above |
| `tail_policy` | `none`, `explicit_residual_lane`, `hosted_tail`, or `TOKEN_VAZIO` |
| `shadow_policy` | `none`, `declared_shadow_state`, `host_runtime_shadow`, or `TOKEN_VAZIO` |

## Tail and shadow rule

For freestanding claims, hidden cleanup tails and implicit shadow runtime state are not accepted.

Allowed patterns:

| Pattern | Status | Rule |
|---|---|---|
| fixed block exact path | allowed | input length/domain is part of the contract |
| explicit residual lane | allowed | residual behavior is named and tested |
| hosted wrapper tail | hosted only | cannot be used as L0 proof |
| implicit runtime shadow | blocked | must become declared state or remain `TOKEN_VAZIO` |


## Android ARM32/ARM64 instances

The first concrete Android instances are recorded as data, not as runtime claims:

| Manifest | ABI | CPU | Word bits | State | Limit |
|---|---|---|---:|---|---|
| `data/platform/arm32-android-freestanding-envelope.v1.json` | `armeabi-v7a` | `armv7` | 32 | `NOT_RUN` | no current ELF/APK/device proof |
| `data/platform/arm64-android-freestanding-envelope.v1.json` | `arm64-v8a` | `aarch64` | 64 | `NOT_RUN` | no current ELF/APK/device proof |

Both preserve:

```text
tail_policy   = explicit_residual_lane
shadow_policy = none
claim_allowed = false
```

Promotion requires a current-chain receipt:

```text
source -> ARM artifact -> APK -> embedded identity -> signature -> install -> launch/dlopen -> log/exit receipt
```

## Change checklist

Before a platform-dependent change is promoted beyond reference:

1. Declare dependency class and platform row.
2. Declare source authority and artifact target.
3. Preserve `TOKEN_VAZIO` for missing device/toolchain evidence.
4. Add or point to the smallest relevant gate.
5. Record receipt with exact command or reason `NOT_RUN`.

## R3

`F_ok`: route and vocabulary are defined for cross-platform low-level work.
`F_gap`: no current build, install, device or CI proof is created by this document.
`F_next`: instantiate one narrow target first, preferably Android ARMv7/AArch64 or Linux x86_64, then attach exact receipts.
