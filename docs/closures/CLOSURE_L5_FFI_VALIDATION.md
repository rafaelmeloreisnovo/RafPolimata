# L5 — FFI Validation Closure

**Date:** 2026-08-15  
**Status:** `EXECUTABLE_BOUNDED_SLICE_READY`  
**Current execution evidence on this PR head:** `TOKEN_VAZIO_CI_QUEUED`  
**12-language FFI coverage:** `TOKEN_VAZIO_OUT_OF_SCOPE`  
**claim_allowed:** `false` for broad polyglot interoperability

## What L5 means here

L5 is split into two claims that must not be conflated:

```text
L5-A: at least one real foreign-function boundary can load a native artifact,
      call typed exported functions, preserve ABI semantics and reject invalid bounds.

L5-B: the RafPolimata language matrix has equivalent FFI behavior across all
      intended languages/platforms/ABIs.
```

This branch implements a falsifiable proof for **L5-A only**.

## Executable artifact

```text
tools/validate_ffi.sh
```

The tool builds a real position-independent shared C object and loads it through CPython `ctypes`.

Observed conditions required for PASS:

1. native compiler available;
2. shared object compiles with `-Wall -Wextra -Werror -fPIC -shared`;
3. exported ABI symbols are visible when `nm`/`readelf` is available;
4. Python dynamically loads the `.so`;
5. `raf_ffi_abi_version()` returns `0x00010000`;
6. `raf_ffi_add_u32(20,22) == 42`;
7. unsigned 32-bit wrap is preserved;
8. 64-bit XOR semantics are preserved;
9. a byte-buffer transform crosses the FFI boundary correctly;
10. insufficient output capacity is rejected (`-1`).

Successful execution emits:

```text
docs/proofs/L5_FFI_C_PYTHON_<timestamp>.json
```

with state:

```text
PASS_C_PYTHON_FFI
```

and explicitly keeps:

```text
twelve_language_coverage = TOKEN_VAZIO_OUT_OF_SCOPE
claim_allowed = false
```

## CI binding

`.github/workflows/phase-cd-truthful-gates.yml` executes the FFI proof and validates the generated receipt. Until that workflow has actually run on the current head, the repository must not promote the current PR to observed L5 PASS.

## Next expansion

After L5-A has provider evidence, extend without weakening the contract:

```text
C ABI ↔ Python ctypes
C ABI ↔ Java/JNI
C ABI ↔ Kotlin/JNI
C ABI ↔ Rust extern "C"
C ABI ↔ Go cgo (hosted-only lane)
```

Each language/ABI pair should have its own receipt and platform constraints. The aggregate 12-language claim remains `TOKEN_VAZIO` until those lanes exist and run.
