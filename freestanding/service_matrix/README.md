# RAFAELIA S12 · Serviço L0 de composição de módulos e reconhecimento de cabeçalhos

Estado: **implementação local de uma capacidade limitada**, não APK/verificador ART/DEX. Inherit `../AGENTS.md` and this subtree's `AGENTS.md`. The kernel is new authorial C11 source, with no third-party code incorporated.

## ABI / limites
`void raf_fs_probe(const raf_u8 *input, raf_u32 length, raf_u32 matrix[8])`

Input/output are **caller-owned, correctly sized and non-overlapping**. An absent `matrix` cannot be reported; in that case the function returns without touching memory. An absent `input` writes INVALID_POINTER to the matrix. Header probing is one fixed frame and does not read beyond 4 bytes if the input length is shorter than 8; a length shorter than four yields NEED_BYTES. The consumer must not claim security or authenticity from a candidate label.

Cells 0..7: state, format, input length, minimum length, ambiguity (1), validated_format (always 0), ownership (always 0), ABI version (1). C code may use identifiers for auditing; the **contract maps matrix positions**, it does not hide meaning behind anonymous variable names.

Thirteen **candidate** prefix groups: `ELF, DEX, CDEX, OAT, VDEX, ART, ZIP, AR, WASM, PNG, GZIP, MZ, Mach-O`. Classification does NOT imply complete header verification, API compatibility, executable authenticity, APK signing, valid DEX/ART, or parsing payloads. OAT/ART/VDEX are explicitly platform/version sensitive. No I/O, syscalls, JNI, runtime dependency or external symbol is intended in the L0 object.

The seven friction lanes (Android JNI/Java; ART/JIT; SDK/NDK; heap/GC; Gradle/R8/D8/JDK; ELF loader and syscalls; flags/branch/tail/shadow/hex) are **separate future adapters and evidence gates**, not implemented in this one C probe.

## Execute the scoped gate
```sh
sh freestanding/service_matrix/tests/verify_s12.sh
```
The shell test uses Clang 17, GCC-style host binary fixture with sanitizers, cross-compiles freestanding ARMv7 and AArch64 relocatable objects, rejects undefined symbols with `nm -u`, verifies architecture with `readelf -h`. Python, shell, Clang, linker, sanitizer and CI are TOOLING_TEST, not native dependencies of L0.

Local observed: fixture PASS, ARMv7+AArch64 object compile PASS, undefined-symbol check PASS. GitHub Actions/exact-head and physical-device receipts are independent and must be observed. No merge or source deletion.

## Fail modes and rollback
- Null input -> INVALID_POINTER; frame too short -> NEED_BYTES; unknown magic -> UNRECOGNIZED.
- A matching prefix -> HEADER_CANDIDATE only (always validated_format=0).
- No owner/allocator/system binding and no static allocation in L0.
- ABI/API/arch drift -> fail closed and revert consumer pointer to predecessor; keep all commits/receipts.
- Human must explicitly inspect independent local and CI results before merge.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`. `TOKEN_VAZIO != 0`. `claim_allowed=false`.


## S12 R2 — module boarding / flags admission (actual implementation)
`raf_fs_admit.c/.h` is an additional authored **PURE_CORE admission stage** with a fixed 8-cell request/result and a `void` function. It admits contracts for the two implemented L0 module types (PREFIX_PROBE and FLAG_GATE) only when ABI=1 and external-capability mask=0. Its policy denies capability bits for libc, heap, syscalls, JNI, JVM, JIT, network, OS logging and dynamic linkage. PLATFORM_SYSCALL and ANDROID_BRIDGE module IDs are explicitly **NOT_IMPLEMENTED**, never silent stubs that return success. This stage does not execute a module, schedule it or issue syscalls, and result[5]=0 even on admission.
The CI's `verify_s12.sh` now includes an additional 8-case negative/positive admission fixture gate, host sanitizer and ARMv7/AArch64 undefined-symbol checks for BOTH C objects. Human promotion remains blocked; no in-device execution.
