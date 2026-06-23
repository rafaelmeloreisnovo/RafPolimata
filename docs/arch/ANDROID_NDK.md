# Android NDK / ARM ABI Architecture Reference

This document covers the Android NDK ABI conventions, JNI bridge patterns,
CMake configuration, and syscall usage as implemented in the RafPolimata project.
All code constants and patterns are taken directly from RAF_041–RAF_047 and
`Apkc/lang_profile.h`.

---

## 1. ABI Overview

The Android NDK supports multiple ABIs. The project explicitly targets two ARM
variants and defines their invariants in RAF_043 and RAF_044.

| ABI name     | Architecture | Data model | Pointer width | long width |
|--------------|-------------|------------|---------------|------------|
| arm64-v8a    | AArch64     | LP64       | 8 bytes       | 8 bytes    |
| armeabi-v7a  | ARM Thumb-2 | ILP32      | 4 bytes       | 4 bytes    |
| x86_64       | x86-64      | LP64       | 8 bytes       | 8 bytes    |

LP64: long and pointer are 64-bit, int is 32-bit.
ILP32: int, long, and pointer are all 32-bit.

### 1.1 Compile-Time Size Invariants

From RAF_043 (arm64-v8a):
```c
_Static_assert(sizeof(uint64_t) == 8u, "uint64_t must be 8 bytes (ARM64 LP64 ABI)");
_Static_assert(sizeof(uint32_t) == 4u, "uint32_t must be 4 bytes");
_Static_assert(sizeof(uint16_t) == 2u, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint8_t)  == 1u, "uint8_t must be 1 byte");
```

From RAF_044 (armeabi-v7a, ILP32):
```c
_Static_assert(sizeof(uint32_t) == 4u, "uint32_t must be 4 bytes (ARM32 ILP32 ABI)");
_Static_assert(sizeof(uint16_t) == 2u, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint8_t)  == 1u, "uint8_t must be 1 byte");
_Static_assert(sizeof(uint64_t) == 8u, "uint64_t must be 8 bytes even on ARM32");
```

Note: `uint64_t` is 8 bytes on both ABIs. The difference is `sizeof(long)` and
`sizeof(void *)`, which are 4 on armeabi-v7a and 8 on arm64-v8a.

### 1.2 Runtime ABI Detection

RAF_045 provides a runtime enum:
```c
typedef enum {
    ABI_UNKNOWN = 0,
    ABI_ARM64   = 1,   // __aarch64__
    ABI_ARM32   = 2,   // __arm__
    ABI_X86_64  = 3,   // __x86_64__
    ABI_X86     = 4    // __i386__
} rafaelia_abi_t;
```

Detection is done at compile time via preprocessor macros; the runtime function
returns the pre-selected enum value. In freestanding/embedded code, the ABI is
always known at compile time and this enum eliminates branch-on-unknown logic.

---

## 2. JNI Bridge

Source file: RAF_041.

### 2.1 JNI Type Definitions

When compiling on the Android target with the NDK, `<jni.h>` provides the
canonical types. For host builds without the NDK, RAF_041 defines local aliases:

```c
#if defined(__ANDROID__)
#include <jni.h>
#else
typedef void *   JNIEnv;
typedef void *   jobject;
typedef int32_t  jint;
typedef int64_t  jlong;
#endif
```

The aliases enable the same C source to be compiled and tested on the host
without requiring an NDK installation.

### 2.2 JNI Function Naming Convention

JNI functions must follow the naming convention:
```
Java_<package_underscored>_<ClassName>_<methodName>
```

Example from RAF_041 — exposing `nativeGetValue()` in class `com.rafael.RafNative`:
```c
jlong rafaelia_m041_jni_impl(JNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    return (jlong)42;
}
```

The actual exported symbol for production use would be:
```
Java_com_rafael_RafNative_nativeGetValue
```

The Java/Kotlin caller loads the library with `System.loadLibrary("rafnative")`,
and the JVM resolves the function automatically by name at first call.

### 2.3 JNI_OnLoad Convention

For libraries that need initialization before any JNI method is called, the
canonical entry point is:
```c
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK)
        return JNI_ERR;
    // register natives, cache class references, etc.
    return JNI_VERSION_1_6;
}
```

This is not implemented in RAF_041 (minimal bridge), but it is the standard
pattern for production JNI libraries.

### 2.4 JNI Ring Buffer (RAF_047)

RAF_047 exposes a 64-element ring buffer using `jlong` (64-bit integer) as the
element type. It uses typed aliases so the same source works on Android and host:

```c
#if defined(__ANDROID__)
typedef jlong raf_jlong;
typedef jint  raf_jint;
#else
typedef long  raf_jlong;
typedef int   raf_jint;
#endif

#define M047_CAPACITY 64

typedef struct {
    raf_jlong buf[M047_CAPACITY];
    raf_jint  head;
    raf_jint  tail;
    raf_jint  count;
} rafaelia_ring_m047_t;
```

The ring buffer is statically allocated (no heap) and uses index-modulo access
(`head % CAPACITY`) to avoid branch-heavy wrap logic. This design allows the
buffer to be read by the JVM via direct byte buffer or JNI array copy without
additional allocation.

---

## 3. CMakeLists.txt for Per-ABI Builds

Source file: RAF_042.

The project's canonical `CMakeLists.txt` template for Android NDK builds applies
different compile flags for each ABI:

```cmake
cmake_minimum_required(VERSION 3.22)
project(RafNative C)

add_library(rafnative SHARED rafnative.c)

if(ANDROID_ABI STREQUAL "arm64-v8a")
  target_compile_options(rafnative PRIVATE
    -march=armv8.2-a+crypto+crc
    -O3 -ffast-math)
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
  target_compile_options(rafnative PRIVATE
    -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp
    -O2)
elseif(ANDROID_ABI STREQUAL "x86_64")
  target_compile_options(rafnative PRIVATE
    -march=x86-64 -msse4.2 -O2)
endif()

target_link_libraries(rafnative log)
```

This template is embedded as a string constant in RAF_042 and verified at
runtime by scanning for the presence of both `arm64` and `armeabi` substrings.

### 3.1 ABI-Specific Compile Flags

| ABI          | -march flag                  | Optimization | Extensions |
|--------------|------------------------------|-------------|------------|
| arm64-v8a    | armv8.2-a+crypto+crc        | -O3 -ffast-math | Crypto (AES, SHA, PMULL), CRC32 |
| armeabi-v7a  | armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp | -O2 | NEON VFPv4 |
| x86_64       | x86-64 -msse4.2              | -O2         | SSE4.2 |

The `softfp` float ABI for armeabi-v7a means that floating-point arguments are
passed in integer registers but computed in VFP registers. This is required for
compatibility with the Android system libraries that use the softfp convention.

### 3.2 Integration with build.gradle

The `CMakeLists.txt` is referenced from `build.gradle` (Groovy) or
`build.gradle.kts` (Kotlin):
```groovy
android {
    defaultConfig {
        externalNativeBuild {
            cmake {
                abiFilters "arm64-v8a", "armeabi-v7a"
            }
        }
    }
    externalNativeBuild {
        cmake {
            path "CMakeLists.txt"
        }
    }
}
```

---

## 4. arm64-v8a Build Verification (RAF_043)

RAF_043 verifies arm64-v8a invariants at both compile time (_Static_assert) and
runtime (pointer width check + NEON register probe):

```c
#if defined(__aarch64__)
    if (sizeof(void *) != 8u) return -1;
    __asm__ __volatile__("movi v0.16b, #0" : : : "v0");  // NEON availability probe
    return 0;
#endif
```

The `movi v0.16b, #0` instruction zeros a NEON vector register. If the CPU does
not support NEON (which is mandatory on arm64-v8a by specification), this
instruction will fault. In practice, all AArch64 cores include NEON.

---

## 5. armeabi-v7a Build Verification (RAF_044)

RAF_044 verifies armeabi-v7a invariants:
```c
#if defined(__arm__)
    if (sizeof(void *) != 4u) return -1;
    volatile uint32_t pc_val = 0u;
    __asm__ __volatile__("mov %0, pc" : "=r"(pc_val));
    (void)pc_val;
    return 0;
#endif
```

The `mov %0, pc` instruction reads the program counter, confirming Thumb-2 ISA
availability. For armeabi-v7a, the NDK defaults to Thumb-2 encoding for compact
code. The ABI does not support ARM32 VFP with hardware float ABI (`hardfp`) because
Android uses `softfp` system libraries.

---

## 6. Direct Syscall (RAF_046)

Source file: RAF_046.

For latency-critical operations, the project demonstrates calling Linux syscalls
directly without going through the Bionic libc wrapper:

```c
#include <sys/syscall.h>
#include <unistd.h>

long tid = syscall(SYS_gettid);
```

`SYS_gettid` returns the kernel thread ID without the overhead of the libc
`gettid()` wrapper or `pthread_self()`. On Android, Bionic's `syscall()` is a
thin inline wrapper that places the syscall number in the appropriate register
(X8 on AArch64, R7 on ARM32) and executes `svc #0`.

### 6.1 When Direct Syscalls Are Appropriate

Direct syscalls are appropriate when:
- The libc wrapper adds measurable overhead (e.g., VDSO not available).
- The target syscall is not wrapped by Bionic (rare).
- The code runs in a freestanding environment without Bionic.

The project's `Apkc/sys.h` provides freestanding wrappers for read/write/open/
fork/execve/waitpid using `svc` (AArch64) or `swi` (ARM32) directly, without
any libc dependency.

### 6.2 Syscall Number Portability

Syscall numbers differ between AArch64 and ARM32 (and x86). The `SYS_*` macros
from `<sys/syscall.h>` are ABI-specific and must not be hardcoded as integers
in portable code. In `lang_script.h`, `__NR_execve = 221` is hardcoded because
the bootstrap generates ARM64 code unconditionally.

---

## 7. Stack Layout on Android NDK

### 7.1 arm64-v8a Stack (LP64)

The AAPCS64 calling convention requires:
- SP aligned to 16 bytes at all call sites.
- Frame pointer (X29) points to the saved FP/LR slot.
- X0-X7: argument registers; X0 = return value.
- X19-X28: callee-saved registers.

### 7.2 armeabi-v7a Stack (ILP32, Thumb-2)

The AAPCS calling convention:
- SP aligned to 8 bytes; 8-byte alignment required for `double` and `int64_t`.
- R0-R3: argument registers; R0 = return value.
- R4-R11: callee-saved registers.
- R7: frame pointer (in thumb conventions; R11 in ARM mode).
- R14 (LR): link register; R15 (PC): program counter.

### 7.3 libmain.so Entry Point

The `Apkc` compiler generates `libmain.so` (ELF64 or ELF32 .so) placed in the
APK under `lib/arm64-v8a/libmain.so` or `lib/armeabi-v7a/libmain.so`. This is a
standard Android shared library entry. The Android runtime loads it with
`System.loadLibrary("main")` or via the NativeActivity mechanism.

The generated `.so` exports two symbols: `_start` (unused at load time) and
`JNI_OnLoad` or interpreter-bootstrap code depending on the language profile.

---

## 8. Freestanding Constraints in the NDK Context

### 8.1 Apkc vs. Bionic

The `Apkc/` subsystem is fully freestanding: no `malloc`, no libc includes, and
syscalls via `Apkc/sys.h` inline `svc` wrappers. This is distinct from the
typical NDK build, which links against Bionic (Android's libc).

| Property | Apkc freestanding | Normal NDK |
|----------|-------------------|------------|
| malloc/free | Not used | Available (Bionic) |
| printf/snprintf | Not used | Available |
| syscalls | Direct svc (sys.h) | Via Bionic wrappers |
| exception handling | Not used | Available (libunwind) |
| thread-local storage | Not used | Available (TLS via TPIDR_EL0) |

### 8.2 Script Languages on Android

Python, Shell, Perl, Node.js, and PHP reach the device via the execve bootstrap
in `lang_script.h`. This is only viable if the interpreter is installed on the
device (e.g., Termux provides `/usr/bin/python3`, `/bin/sh`, etc.). On a stock
Android device without Termux, only the native compiler paths (C, C++, Rust)
and JVM paths (Kotlin, Java) are viable.

### 8.3 DEX Output Path (Kotlin, Java)

Kotlin and Java source files go through a two-stage pipeline:
1. `kotlinc` or `javac` produces a `.jar` or class directory.
2. `d8` converts to `classes.dex`.
3. The `.dex` file is stored as `classes.dex` in the APK root.

This path is controlled by `use_d8 = 1` in `lang_profile.h`:

| Language | LP ID  | Compiler | use_d8 | Output |
|----------|--------|----------|--------|--------|
| Kotlin   | LP_KT  | kotlinc  | 1      | classes.dex |
| Java     | LP_JAVA | javac   | 1      | classes.dex |
| Dart     | (not in table) | — | — | — |

---

## 9. Language Dispatch Table Entries (Android-Relevant)

From `Apkc/lang_profile.h`, the entries that produce Android-compatible output:

| LP_ID    | Name  | Pipeline | arm64_only | Output format |
|----------|-------|----------|------------|---------------|
| LP_C     | c     | clang --target aarch64-linux-android -shared | 1 | lib/arm64-v8a/libmain.so |
| LP_CPP   | cpp   | clang++ --target aarch64-linux-android -shared | 1 | lib/arm64-v8a/libmain.so |
| LP_RS    | rs    | rustc --target aarch64-linux-android --crate-type cdylib | 1 | lib/arm64-v8a/libmain.so |
| LP_KT    | kt    | kotlinc + d8 | 0 | classes.dex |
| LP_JAVA  | java  | javac + d8   | 0 | classes.dex |
| LP_PY    | py    | execve bootstrap (python3 -c) | 1 | lib/arm64-v8a/libmain.so |
| LP_JSX   | jsx   | npx babel + node bootstrap | 1 | lib/arm64-v8a/libmain.so |

The `arm64_only = 1` flag means the `.so` bootstrap is generated only for arm64-v8a.
Kotlin and Java output DEX which runs on the ART virtual machine on both 32-bit and
64-bit devices.

---

## 10. Key Source Files

| File | Domain | Description |
|------|--------|-------------|
| RAF_041_jni_bridge_minimo.c | JNI | Minimal JNI function, jlong return, host aliases |
| RAF_042_cmake_separado_por_abi.c | Build | Per-ABI CMakeLists.txt template as string constant |
| RAF_043_build_arm64_v8a.c | Build | LP64 invariants, NEON probe, pointer width check |
| RAF_044_build_armeabi_v7a.c | Build | ILP32 invariants, Thumb-2 PC read probe |
| RAF_045_deteccao_de_abi_em_runtime.c | Runtime | ABI detection enum from preprocessor macros |
| RAF_046_syscall_direta_quando_fizer_sentido.c | Syscall | SYS_gettid direct call via syscall() |
| RAF_047_ring_buffer_nativo_exposto_ao_kotlin_java.c | Buffer | jlong ring buffer, capacity 64, no heap |
| Apkc/lang_profile.h | Dispatch | Language table with arm64_only, use_d8, jsx_node flags |
| Apkc/lang_script.h | Bootstrap | execve bootstrap; __NR_execve = 221 hardcoded |
| Apkc/sys.h | Syscalls | Freestanding svc/swi wrappers (no Bionic) |
| Apkc/fmt_elf.h | Output | ELF64/ELF32 .so builder for libmain.so generation |
