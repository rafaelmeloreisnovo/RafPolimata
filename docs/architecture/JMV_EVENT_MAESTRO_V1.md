# JMV Ω — Maestro de eventos (freestanding, v1)

Owner: `rafaelmeloreisnovo/RafPolimata`, `freestanding/include/raf_fs_jmv.h`.
Lifecycle: `IMPLEMENTED_UNTESTED` until an exact-HEAD gate; `claim_allowed=false` globally.
Gap governance: `CLOSURE_L11` binds source/authority/rights uncertainties; `CLOSURE_L12` leaves physical runtime/device evidence open. Binding is not promotion.
Route: `START HERE -> CURRENT_STATE -> freestanding/README.md -> this file -> verify_jmv.sh -> exact-HEAD CI -> receipt`.

## The mechanical-piano metaphor

A perforated score is a bounded tape with four caller-owned arrays: `(tick, voice, opcode, value)`. One `raf_fs_jmv_step` invocation selects the least not-yet-consumed event ordered by `(tick, voice, source_index)`, applies it to exactly one of eight voices, and emits a saturated mix. Source storage need not be chronological: explicit arbitration creates a deterministic sequence.

This is **not** parallel execution, temporal physics, Gregorian chant synthesis, physical audio generation, a Java virtual machine, Java bytecode interpreter or JVM compatibility.

## Pure L0 contract

- No classes, Java/JNI, native OS functions, libc, hosted headers, heap, GC, malloc, clock, syscalls, linker, runtime calls, external helpers or fallback in the JMV kernel.
- Caller owns buffers and any I/O, time base, audio, scheduling across CPU threads or adapters. That caller can be hosted, but its dependencies do not enter L0.
- `0..32` events, eight voices (`0..7`); `NOTE` sets pitch integer `0..127`; `DELTA` saturating signed addition; `GAIN` saturating multiplication by `[-16,16]`; `REST` zeroes with operand zero.
- A `raf_u32` consumed mask owns the event history; bit `i` denotes source index `i`. No implicit state is retained.
- `BAD_INPUT` and `BAD_TAPE` reject invalid input **before state or output mutation**; `DONE` means no remaining events; `STEP` means exactly one event committed.
- `TOKEN_VAZIO` is a distinct evidence mark, never success. Input not observed is not zero.
- Exact tie behavior: tick, then voice, then source order. Permuting same-voice/same-tick noncommuting operations can change semantics: no unsupported permutation-invariance claim.
- Explicit fixed-bound scans validate the tape and arbitrate the event; they are not hidden SIMD residual-tail handlers.
- Target-runtime dependencies: zero. Clang/GCC, POSIX shell, `nm` and GitHub runners belong **only** to the build/test provider boundary.

## Evidence and falsifiers

Run `sh freestanding/tests/verify_jmv.sh`: synthetic host assertions plus six target `-nostdinc` freestanding object builds, each rejected on unresolved symbols. Test success does not grant JVM, hardware runtime or physical device claims.

Open `TOKEN_VAZIO`: root rights/redistribution provenance, exact-head CI, review, real ARMv7/AArch64 physical runtime and independent reconstruction. No imported code, no prior implementation replaced.

Rollback: close the draft PR or revert its additive JMV files; no change to canonical `raf_fs_core.h` or existing gate thresholds.

R3 = <new authorial kernel+test+CI contract, external/physical/license proof open, exact-HEAD CI/readback and review next>.
