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

### x86_64

Baseline object:

```text
-target x86_64-unknown-none
-m64
-mno-red-zone       # kernel/firmware profile only
```

Vector extensions must be selected explicitly (`-mavx2`, `-mavx512f`, AMX flags, etc.); the generic L0 must not silently raise the ISA floor.

### i686

```text
-target i686-unknown-none
-m32
-march=i686
-fomit-frame-pointer
```

Do not emit `mfence`/SSE2 instructions for baseline i686 unless the build raises the ISA floor explicitly.

### ARMv7-A / AArch32

```text
-target armv7a-unknown-none-eabi
-march=armv7-a
```

Optional SIMD profile:

```text
-mfpu=neon-vfpv4
-mfloat-abi=softfp
```

The generic scalar object does not require NEON.

### AArch64

```text
-target aarch64-unknown-none
-march=armv8-a
```

SVE/SME profiles are separate opt-in builds; no generic object may assume them.

### RISC-V RV32

```text
-target riscv32-unknown-elf
-march=rv32i
-mabi=ilp32
```

Raise to `rv32imac`, vector, bitmanip or other extensions only in an explicit profile.

### RISC-V RV64

```text
-target riscv64-unknown-elf
-march=rv64i
-mabi=lp64
```

## Symbol policy

The hot core should compile to no externally required helper symbol. `static inline`, compile-time macros and inline assembly are preferred. Raw opcode bytes/words are allowed only when the selected assembler cannot encode a required instruction; every raw encoding must carry a readable mnemonic and ISA encoding comment.

## No-tail policy

Do not generate a hidden scalar tail routine. A final partial vector is represented by an explicit lane count/mask so the architecture backend may use predication/masking where available.
