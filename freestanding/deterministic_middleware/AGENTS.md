# AGENTS.md — RAFAELIA PURE_CORE deterministic middleware S14

This directory is governed by the closest `freestanding/AGENTS.md`, root `AGENTS.md`, `docs/AGENTES.md` and the user's current instructions. The stronger evidence, safety and reversibility rule wins.

This module has **one exported C11 function** `raf_mw_route`, fixed caller-owned 16-word input/output, no hosted headers, libc, heap, GC, JNI, Java, ART, syscalls, timers, GPU or NUMA runtime calls, dynamic dispatch, external native helpers, logging or network. External compiler, Python, shell, GitHub CI, readelf and nm are **TOOLING_TEST**, not L0 runtime dependencies.

All statuses are **routing-plan tokens**, not execution. `OUT_EXECUTED=0` is mandatory. Two watchdogs only compare caller-provided counters; they are not running timers. Failover/fallback require explicit host-given capability evidence and policy flags; the mask itself does not constitute independent verification. Rollback is only a recommendation. Host permission is not proof of human authorization.

Only scalar/NEON/SIMD may be returned as candidate CPU lanes; GPU/NUMA are hosted capabilities and must not be silently executed. ELF/DEX/ART are metadata hints, not parsers. Do not label branchless/stackless/tailless or energy efficient without actual same-head disassembly and benchmarks.

Before reuse check `LICENSE-STATUS.md` and parent/upstream license; this draft does not grant rights or override any existing license. Maintain exact original source and authorial provenance, no deletion, no auto-merge, explicit rollback and human review.

Run `sh freestanding/deterministic_middleware/tests/verify.sh`, verify host sanitizer tests, ARMv7+AArch64 objects, machine codes and `nm -u`. A CI PASS is scoped to that workflow and SHA, not phone/Android/GPU proof.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`; `TOKEN_VAZIO != 0`; `claim_allowed=false`.
