# Benchmark RAFAELIA — no-libc userspace + Termux boundary

**State:** active engineering path.  
**Governance:** runtime/device gaps → `CLOSURE_L12`.  
**Scope:** ARM64/x86-64/ARM32 Linux/Android userspace without libc/CRT ownership in the benchmark executable.

## Execution classification

| Layer | Kernel/OS ABI | libc/heap | Meaning |
|---|---|---|---|
| `freestanding/` L0 | none | none | OS-neutral CPU/register/memory core |
| `syscall/` + `Benchmark/raf_sys.h` | Linux/Android raw syscalls | no libc; no dynamic heap in this harness | no-libc userspace adapter |
| Termux / `termux-app-rafacodephi` | Android/Linux runtime | runtime-owned | execution/evidence lane; does not redefine L0 |
| physical bare-metal firmware | no OS/kernel/syscall | target-specific | requires startup/linker/memory-map/board/device receipt |

Therefore `-nostdlib` or a static ELF with direct syscalls is **not** sufficient to call an artifact physical bare-metal.

## ARM32 / Termux correction

- `raf_sys.h` uses `clock_gettime(CLOCK_MONOTONIC)` through the raw Linux syscall ABI because PMU access is commonly unavailable in Android userland.
- ARM32 timer samples are already nanoseconds; `raf_bench.h` avoids a generic 64-bit division that could synthesize an external `__aeabi_*` helper.
- Timer syscall cost is part of the ARM32 measurement envelope and is no longer reported as zero.
- `build2.sh` has one native ARM32 profile only: `armv7-a + softfp + NEON`. The previous second ARM32 build overwrote the first artifact with different flags.
- The optional `arm-linux-gnueabihf-gcc` cross artifact remains separate as `raf_enterprise_a32_hf`.

## x86-64 timer unit

`rdtsc+lfence` is emitted as raw TSC ticks while `raf_tsc_freq()` is zero. The reporter therefore prints `ticks`, not false `ns`. Calibrated nanoseconds require a separately measured TSC frequency.

## Build on Termux ARM32

```bash
cd Benchmark
bash build2.sh
./raf_enterprise_a32
```

Expected classification of that execution:

```text
SOURCE        = Benchmark C/headers
ARTIFACT      = no-libc ELF
EXECUTION     = Linux/Android userspace via raw syscalls
EVIDENCE      = current run output + ELF/symbol/hash receipt
BARE_METAL    = TOKEN_VAZIO (CLOSURE_L12) until a board/startup/device path is actually executed
```

## Low-level design

- static/caller-owned storage; no `malloc/free` in the benchmark core;
- no GC runtime;
- direct syscall adapter for I/O/exit and ARM32 timing;
- Q16/integer kernels and architecture-specific primitives;
- compile/link flags remove hosted defaults but do not erase the kernel boundary.

Run the executable boundary guard with:

```bash
sh scripts/verify_execution_boundaries.sh
```

For the strict OS-neutral L0, use the independent `freestanding/tests/*` gates.
