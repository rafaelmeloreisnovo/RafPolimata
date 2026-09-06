# RAFAELIA L0 — validation receipt

The initial source was cross-compiled as freestanding objects with Clang 17 for:

```text
x86_64-linux-gnu
i686-linux-gnu
armv7a-linux-gnueabihf
aarch64-linux-gnu
riscv32-linux-gnu
riscv64-linux-gnu
```

Common validation flags:

```text
-ffreestanding
-fno-builtin
-fno-stack-protector
-fno-unwind-tables
-fno-asynchronous-unwind-tables
-fno-ident
-fomit-frame-pointer
-Os
```

Validated source units:

1. `freestanding/include/raf_fs_core_noloop.h` through a minimal caller-owned stage.
2. `syscall/linux/raf_linux_syscall.h` through a three-argument raw syscall probe.

Result for all six targets: object compilation succeeded in the local validation environment.

This receipt proves source/toolchain acceptance only. It does not prove runtime execution on physical targets. Runtime and device states remain `TOKEN_VAZIO` until receipts exist.
