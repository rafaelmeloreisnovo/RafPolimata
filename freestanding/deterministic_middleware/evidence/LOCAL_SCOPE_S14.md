# S14 — local executable gate (scope-limited evidence)

Date: 2026-10-24; host compiler: Clang 17 (x86_64 host). The new **pure C11** `raf_mw_route` has one public function and 16×32-bit request/response, without standard library, heap, syscalls, JNI, callback, logging or dynamic allocation in its linked L0 object.

Executed locally on matching C header/source Git blob identities:
- `raf_mw.c`: Git blob `e46b1e24191f901edf91d6ce4c5003fa5d74438f`, SHA256 `0d4d1c1af657801aa532a3f06816b3a1b2135d9899c5c8433dc6ae4b33235b45`.
- `raf_mw.h`: Git blob `ad97c24a2c9c3ea291e26ce3d74322f89d84f9d5`, SHA256 `5ac9ebc3e6d023cb37c217678f95092e42796244d8907956b02f7d7f3f6cae22`.
- Sanitized host fixture: 14 scenario cases + 4 bounds/NULL guards PASS; no hidden release claim.
- Relocatable objects: x86-64, ARMv7-A, AArch64 compiled with `-ffreestanding -fno-builtin`; `nm -u` empty and `readelf -h` machine type matched each target. Cross objects not executed on those processors.
- Clang -Os local x86 object `.text` size 607 bytes (one configuration, NOT benchmark of energy/performance nor a comparison to legacy). Local disassembly contains conditional jumps, so `BRANCHLESS` **NOT VERIFIED**; no stackless/tail-free claim without systematic analysis.
- The two watchdogs are passive integer comparisons; there is no clock or active scheduler. Failover and rollback are PLAN/RECOMMENDATION states, NOT physical operations.
- Capability availability/host_permission/quality fields are supplied by an external controller and are not independent hardware, consent or quality attestations.

**False claims blocked:** no actual NEON SIMD instruction execution, GPU or NUMA scheduling; no ELF/DEX/ART parsing or Android native integration; no real phone install, signed APK, upstream license verification or required-check provider enforcement; `claim_allowed=false`.

F_next: exact-head provider run + negative cases, host adapter design with owner authorization, actual ARM32 runtime receipt, and independent rights/privacy/cultural review. Rollback: close unmerged branch, preserve predecessors.
