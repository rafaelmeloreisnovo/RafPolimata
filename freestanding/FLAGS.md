# RAFAELIA L0 — compile/link flags

These flags describe zero-runtime objects. Freestanding gates use OS-neutral target triples; Linux triples belong only to `syscall/`.

## Common C/Clang profile

```text
-ffreestanding
-fno-builtin
-fno-stack-protector
-fno-unwind-tables
-fno-asynchronous-unwind-tables
-fno-ident
-fomit-frame-pointer
-fdata-sections
-ffunction-sections
-Os
```

Optional when the final environment guarantees it:

```text
-fno-pic
-fno-pie
```

## Standalone link-only profile

```text
-nostdlib
-nostartfiles
-nodefaultlibs
-Wl,--gc-sections
-Wl,--build-id=none
```

The linker script, entry symbol, memory map and external calling convention are environment-owned and remain outside generic L0.

## x86 profiles

### x86_64 scalar / SSE2

```text
-target x86_64-unknown-none
-msse2
-mno-red-zone
```

### i686 scalar / explicit SSE2

```text
-target i686-unknown-none
-march=i686
-msse2
-mregparm=3   # codegen probe only: isolates L0 from external cdecl stack argument passing
```

The production hot core is forced inline; `-mregparm=3` is not declared as a universal external ABI.

### x86_64 AVX2

```text
-target x86_64-unknown-none
-mavx2
-mno-red-zone
-mno-vzeroupper
```

### x86_64 AVX-512F + K-mask

```text
-target x86_64-unknown-none
-mavx512f
-mno-red-zone
-mno-vzeroupper
```

The gate requires `ZMM` plus native `K1` masked `vmovdqu32` codegen. `-mno-vzeroupper` avoids inserting a transition instruction into the probe; an external hosted ABI adapter may choose a different transition policy.

### x86_64 AMX-TILE direct-register profile

```text
-target x86_64-unknown-none
-mamx-tile
-mno-red-zone
```

The current L0 primitive gates direct `TMM0` ownership (`tilezero`). Tile-state enablement/configuration is a caller/environment precondition, not an L0 syscall/runtime service.

## ARM profiles

### ARMv7-A / AArch32 scalar

```text
-target armv7a-none-eabi
-march=armv7-a
```

### ARMv7-A NEON

```text
-target armv7a-none-eabi
-march=armv7-a
-mfpu=neon-vfpv4
-mfloat-abi=softfp
```

### AArch64 / ARM64 Advanced SIMD

```text
-target aarch64-none-elf
-march=armv8-a
```

`ARM64` and `AArch64` are the same 64-bit Arm execution state in this project.

### AArch64 SVE

```text
-target aarch64-none-elf
-march=armv8.2-a+sve
```

One hardware-predicated stage reports `consumed_lanes` to caller-owned state; no scalar cleanup loop is added.

### AArch64 SME direct ZA profile

```text
-target aarch64-none-elf
-march=armv9-a+sme
```

The current primitive gates direct `ZA` ownership (`zero {za}`). SME/ZA enablement is caller/environment state; generic L0 does not hide an enable/disable sequence.

## RISC-V profiles

### RV32 scalar

```text
-target riscv32-unknown-elf
-march=rv32i
-mabi=ilp32
```

### RV32 V

```text
-target riscv32-unknown-elf
-march=rv32gcv
-mabi=ilp32d
```

### RV64 scalar

```text
-target riscv64-unknown-elf
-march=rv64i
-mabi=lp64
```

### RV64 V

```text
-target riscv64-unknown-elf
-march=rv64gcv
-mabi=lp64d
```

RVV executes one `VL`-bounded block and stores the actual consumed lane count in caller-owned state; no internal continuation loop exists.

## Metadata-only next architectures

Current zero-runtime metadata probes compile with:

```text
-target powerpc64le-unknown-none
-target loongarch64-unknown-none
-target s390x-unknown-none
```

These prove only register/ABI topology headers for POWER64, LoongArch64 and IBM z/s390x. They do not imply VSX/MMA, LSX/LASX or z-vector executor support.

## Symbol policy

The hot core must not require external helper symbols. Prefer compile-time constants, macros, forced `static inline`, then inline assembly. Raw opcode bytes/words are allowed only when the selected assembler cannot express a required instruction; every raw encoding must document readable mnemonic, operands, ISA encoding and clobbers.

## No-tail / no-shadow policy

Do not generate a hidden scalar tail routine. Fixed-vector residuals use explicit masks or native predication; scalable-vector residuals use hardware predicate/VL and return consumed lanes to the next pipeline stage. No compatibility shadow buffer may appear silently.
