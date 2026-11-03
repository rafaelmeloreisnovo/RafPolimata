# RAFAELIA Ω — Guard Supervisor L0 V1
Status: IMPLEMENTED_SOURCE / LOCAL_TESTS_PASS_SCOPED / DEVICE_AND_PROVIDER_NOT_RUN.
Authority: RafPolimata producer; additive successor to `spec/FREESTANDING_FAILSAFE_FAILOVER_WATCHDOG_ROLLBACK_V1.md` and PR #445.
Authorial implementation for Rafael Melo Reis. Repository-wide redistribution license: TOKEN_VAZIO; never infer other source rights.

## Alpha → Omega: formal failure hypothesis and contract

H0 (falsifier): an incomplete, stale, contradictory, duplicated, unauthorized or unsynchronized witness can select an alternate or previous artifact.
H1 (bounded target): with the stated caller preconditions, all such inputs return INVALID, TOKEN_VAZIO or SAFE_STOP without modifying the decision, and only two fresh concordant fault reports authorize a candidate failover when the secondary route has two fresh concordant healthy reports, a distinct route/artefact, required declared capabilities and an explicit authorization reference.

The policy is a deterministic function of caller-provided structs, not a physical double-watchdog, proof of source authenticity, autonomous intelligence or academic proof of independence. Distinct opaque observer IDs are **necessary but not sufficient** for independent hardware failure domains. Real observer trust is an external security gate.

## Low-level boundaries

| Layer | Permitted | Forbidden claim |
|---|---|---|
| L0 `raf_fs_guard_select` | inspect typed watch observations and select intent | running watchdogs, switching binaries, applying rollback, measuring time |
| Existing ELF32/ELF64 view | bounded structural inspection | loader, linker or execution |
| Existing DEX view | bounded header/map inspection | ART execution, dexopt or class loading |
| ART | hosted/Android-only adapter and verified physical receipt | ART inside L0, Java-less Android runtime proof |
| NUMA plan | capacity/distance advisory selection | physical page migration or memory placement |
| Supabase `raf_private` | external authenticated provenance, SHA and audit receipts | implied SDK access, bypassing RLS or duplicating private corpus |

Capability mask bits are caller declarations only. No code path probes real ELF/DEX/ART/NUMA automatically. Never admit a route from flags alone; the hosted adapter independently validates source SHA, actor authorization, workload, tool identity, signature, privilege, evidence, requested capability and rollback preimage. Only a distinct per-target executor may carry out an explicitly authorized action and produce a new receipt.

## State machine, negative gate and reversibility

1. Require caller-supplied monotonic now, positive maximum witness age, source/artefact identity, explicit requested capability mask.
2. Check two independently identified observations with positive evidence refs and separate observer refs; future/stale/unknown => TOKEN_VAZIO; disagreement => SAFE_STOP.
3. Both primary ALIVE + requested declared capability => PRIMARY candidate. Fault from one witness cannot cause automatic failover.
4. Both primary FAULT + `allow_failover=1` + explicit authorization + distinct secondary source/artifact + two fresh ALIVE secondary observers + capability => FAILOVER candidate.
5. When no permitted route remains, only `allow_rollback=1` with a distinct, caller-verified known-good artifact returns ROLLBACK_READY; this does **not** restore bytes. Otherwise SAFE_STOP or TOKEN_VAZIO.
6. No automatic retries, watchdog scheduling, mutable global variables, heap, syscall, host class, dynamic loader or network calls in L0.

Source ≠ Artifact ≠ Execution ≠ Evidence ≠ Claim. A returned PRIMARY/FAILOVER or ROLLBACK_READY is **intent classification**, not PASS, deployment or scientific validation. TOKEN_VAZIO ≠ numeric zero.

## Falsification executed, reproducible gates

Run `sh freestanding/tests/verify_guard_supervisor_v1.sh`:
- 51 numbered host assertions at Clang O0/O2/O3, plus 256 repeated combinations of dual-watchdog statuses and unchanged-output rejection checks.
- Six target objects compiled with Clang -ffreestanding -nostdinc -fno-builtin: x86_64, i686, ARMv7, AArch64, RV32, RV64. Each must have zero unresolved imports.
- Local additional GCC O2 and Clang ASan+UBSan tests; ARMv7/AArch64 inspected for direct call instructions.
- Predecessor V1 Git blob SHA1 refs (before the cross-role collision hotfix): header 26a502f8c6a91fe83cbbdd88658c059c0b4d37f9; test aca1714428472c09627499e33ca3d5dcd4533e30; probe f06edbaa15eea088eee196c5dbacbc1103baf7b0; shell 480a7aab6938fe8fd757614396d838efd1c9b810. Current exact-head source hashes must be read back after mutation.
- Hotfix: all observer/evidence references must be distinct within each watchdog pair and across the two candidate routes. Four negative checks cover collision rejection with unchanged output; this is only identifier-level hygiene, not proof of independent physical fault domains.

These establish host/test and compiler-object properties only. Supabase API E2E, physical watchdog independence, ART, NUMA placement, exact-head CI, mobile execution, formal verification and external falsification: TOKEN_VAZIO until executed.

## Supabase boundary and rollback

Read-only project baseline 2026-11-03: Rafcodephi PG17, `raf_private` driver nodes=4, projections=12, typed edges=32, run receipts=2; anon/authenticated schema USAGE denied. This change imports no source blobs, adds no SQL functions, makes no migration or ACL change. The eventual adapter must be a separate reviewed change with replay/idempotence, RLS and API negative tests, rollback/backup, and authorized per-driver scope.

Branch change is additive against main merge 9506526f3546cbe08b71c5307a5adf58ee2008bb. Rollback before merge: close review PR. After merge: only reviewed reverting commit or successor with parent/supersedes, never erase evidence. No automatic merge or promotion.
