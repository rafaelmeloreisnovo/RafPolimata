# RAFAELIA S14 — authorial deterministic middleware

One original C11 procedure `void raf_mw_route(const unsigned *request, unsigned request_words, unsigned *result, unsigned result_words)`. A 16×32-bit caller-owned matrix produces a **plan token**, no GPU/CPU work invocation or host operation.

## ABI map
Inputs: 0 version; 1 primary lane (one-hot); 2 allowed mask; 3 separately evidenced mask; 4 faulted mask; 5 fallback mask; 6 binary format label (0–13, not validated); 7 profile (safe=0/throughput=1/energy=2); 8 fallback/failover/rollback policy bits; 9 idle ticks; 10 idle limit; 11 elapsed ticks; 12 total limit; 13 host permission; 14 quality; 15 quality floor.

Outputs: 0 status; 1 selected lane; 2 eligible CPU mask; 3 GPU/NUMA host-boundary mask; 4/5 two watchdog alarms; 6/7 fallback/failover flags; 8 rollback recommendation; 9/10 format/profile; 11 quality gate; 12 primary; 13 flags; 14 EXECUTED=0; 15 ABI=1.

Lanes: 1 scalar, 2 NEON, 4 SIMD; GPU=8 and NUMA=16 are **not** automatically executed. Preferred attested/no-fault lane wins; otherwise use explicit fallback mask. Throughput prioritizes NEON, SIMD, scalar; safe/energy prioritize scalar. No fallback when not authorized, watchdog threshold exceeded, input ABI malformed or quality below floor.

The two watchdogs only compare caller-supplied counters, without timers/threads/interrupts. Failover selects an alternative *plan* following a recorded fault. Rollback sets a suggestion only. The host must enforce authorization, transaction integrity, device capability evidence and any actual rollback.

## Relations
`USER_INTENT -> Mapa -> RafGitTools -> S12 header candidate/admission -> S14 L0 route plan -> explicit HOSTED_BOUNDARY -> audited CPU/Android/NUMA/GPU adapter -> execution receipt -> human approval`.

No ELF/DEX/ART loader, NDK/JNI/Java, GPU/NUMA driver, signed APK, OS syscall, native system function or remote execution is implemented here. Do not mislabel CPU ISA metadata as a working SIMD kernel. There is no shell/Gradle/JIT/malloc/libc runtime dependency in L0; build tools remain external.

## Test and rollback
`sh freestanding/deterministic_middleware/tests/verify.sh` uses host sanitizer tests, three cross-compiled relocatable objects, `nm -u` and `readelf`. A successful object gate is not physical Android or GPU/NUMA proof. Inspect disassembly for branches/stack/hidden compiler helpers before making those claims. Keep the branch unmerged; close or supersede reversibly if gates fail. Read `LICENSE-STATUS.md` and `AGENTS.md` before distribution. `claim_allowed=false`.
