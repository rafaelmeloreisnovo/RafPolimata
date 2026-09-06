# RAFAELIA L0 — compile/link flags

These flags describe the zero-runtime object profile. They do not impose an OS ABI.

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

Do not force those two flags into reusable objects that may later be linked into PIE/shared images.

## Link-only profile for a standalone image

```text
-nostdlib
-nostartfiles
-nodefaultlibs
-Wl,--gc-sections
-Wl,--build-id=none
```

A linker script and entry symbol are environment-owned and therefore intentionally outside the OS-agnostic core.

## Target profiles

### x86_64 scalar

```text
-target x86_64-unknown-none
-m64
-mno-red-zone
```

### x86_64 AVX2

Profile gate currently uses:

```text
-target x86_64-linux-gnu
-mavx2
-mno-red-zone
-mno-vzeroupper
```

`-mno-vzeroupper` is deliberate for the no-extra-transition-instruction probe. A hosted/external ABI adapter may choose differently outside L0.

### x86_64 AVX-512F

```text
-target x86_64-linux-gnu
-mavx512f
-mno-red-zone
-mno-vzeroupper
```

AVX-512 K-mask residual memory and AMX remain separate explicit profiles; never raise the generic x86_64 floor silently.

### i686

```text
-target i686-unknown-none
-m32
-march=i686
-fomit-frame-pointer
```

Do not emit `mfence`/SSE2 instructions for baseline i686 unless the build raises the ISA floor explicitly.

### ARMv7-A / AArch32 scalar

```text
-target armv7a-unknown-none-eabi
-march=armv7-a
```

### ARMv7-A NEON

Profile gate currently uses:

```text
-target armv7a-linux-gnueabihf
-march=armv7-a
-mfpu=neon-vfpv4
-mfloat-abi=softfp
```

The vector object is still freestanding; the Linux-flavoured target triple is used only to obtain a readily available target configuration in CI, not to introduce libc/syscalls.

### AArch64 / ARM64 Advanced SIMD

```text
-target aarch64-linux-gnu
-march=armv8-a
```

Advanced SIMD is the fixed 128-bit profile.

### AArch64 SVE

```text
-target aarch64-linux-gnu
-march=armv8.2-a+sve
```

The SVE stage uses predicate/VL semantics and reports consumed lanes through caller-owned state; no scalar cleanup loop is added.

### RISC-V RV32 scalar

```text
-target riscv32-unknown-elf
-march=rv32i
-mabi=ilp32
```

### RISC-V RV32 V

```text
-target riscv32-linux-gnu
-march=rv32gcv
-mabi=ilp32d
```

### RISC-V RV64 scalar

```text
-target riscv64-unknown-elf
-march=rv64i
-mabi=lp64
```

### RISC-V RV64 V

```text
-target riscv64-linux-gnu
-march=rv64gcv
-mabi=lp64d
```

The RVV stage executes one `VL`-bounded block and stores the actual consumed lane count in caller-owned state. It does not loop internally.

## Symbol policy

The hot core should compile to no externally required helper symbol. `static inline`, compile-time macros and inline assembly are preferred. Raw opcode bytes/words are allowed only when the selected assembler cannot encode a required instruction; every raw encoding must carry a readable mnemonic and ISA encoding comment.

## No-tail policy

Do not generate a hidden scalar tail routine. Fixed-vector residuals use explicit masks/full-block preconditions; scalable-vector residuals use hardware predicate/VL state and return consumed lanes to the next pipeline stage.
