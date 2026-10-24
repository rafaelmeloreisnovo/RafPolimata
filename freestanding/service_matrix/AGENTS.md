# AGENTS.md — RAFAELIA L0 / service_matrix (S12)

Scope: `freestanding/service_matrix/**`. Inherit and NEVER weaken `freestanding/AGENTS.md`, root `AGENTS.md`, `docs/AGENTES.md` and canonical Mapa routing authority.

## Purpose & execution boundary
This specialized authored core **recognizes candidate byte-prefix families only**. It is not a full ELF/DEX/ART/APK verifier, not a loader, not a platform service, not a signature or security attestation. No external code is copied. All claims default `claim_allowed=false`.

## Mandatory gates
1. `PURE_CORE`: C11 fixed-width state, caller-owned matrix, no hosted includes, libc, malloc, heap, GC, JNI, Java, JVM, ART/JIT, syscalls, network, files, OS handles, native-platform functions or external helper calls.
2. `PLATFORM_GATE`: ELF entry, syscalls, OS ABI and system logging belong **outside** this subtree. Never smuggle OS calls into L0. `JNI`/`Android/SDK/NDK` calls belong to distinct hosted adapters; necessity must be justified, permissioned and verified.
3. `TOOLING_TEST`: compiler, assembler, linker, nm, readelf, shell, Python, Gradle/R8/D8 can exist ONLY outside the linked L0 core. Build dependency != runtime dependency.
4. For each compiler profile execute `-ffreestanding -fno-builtin -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables`, compile to relocatable object, reject any undefined symbol. Compiler flags alone do not prove isolation; Clang may generate memcpy/memmove/memset under freestanding.
5. Prefer branchless select/fixed frames/no residual tail or shadow, but **not at expense of memory bounds**. Input guard is required. No raw hex opcodes without assembler inability, ISA/encoding/clobber evidence, and equivalence test.
6. Only caller-owned uint32-equivalent matrix[8] crosses ABI. Slot IDs must remain documented; intentionally anonymous state defeats auditability.
7. Test commit, flags, compiler version, readelf machine, nm -u, fixtures/negative cases and hashes. ARM objects are compile-only; do not claim Android device evidence.
8. Fail closed (UNSUPPORTED/NEED_BYTES/INVALID_POINTER). Never report APK verified, ART validated, full format validation or execution from a prefix. No auto-merge or promotion, human review and rollback required. `TOKEN_VAZIO != 0`.

## Ownership
RafPolimata owns L0. RafGitTools owns control-plane checks. Termux-app owns Android runtime. Mapa owns federation. alphaXiv citations are prior-art references, not authorial code, peer review or publishing. `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`.

Run `sh freestanding/service_matrix/tests/verify_s12.sh`; result is PASS only in executed scope.