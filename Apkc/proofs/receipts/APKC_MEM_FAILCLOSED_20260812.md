# ApkC memory allocator fail-closed receipt — 2026-08-12

State: `VERIFIED_LIMITED`  
Claim gate: `claim_allowed=false` for device/runtime conclusions.

## Scope

Hardening of `Apkc/mem.h` static BSS bump allocators. The prior implementation advanced `_apk_pos` / `_tmp_pos` without checking `APK_CAP` / `TMP_CAP`, so an oversized allocation could return a pointer that enabled writes beyond the fixed buffers.

## Provenance

- Repository: `rafaelmeloreisnovo/RafPolimata`
- Base commit: `a68aa9093e35f9ed2e332501425b2e0f5a33d99b`
- Branch: `audit/apkc-mem-failclosed-20260812`
- Pre-change `Apkc/mem.h` blob: `0b941906c29ff2e6fa07ea7ee97de691b2b88b3f`
- Hardening commit: `2a5bb5313e0e8bc9d1b39184313eec2bd9212c07`
- Post-change `Apkc/mem.h` blob: `48b6e2472367c8b8cc2f61fde03724713e966bb5`

## Material change

`apk_alloc()` and `tmp_alloc()` now:

1. reject requests larger than the remaining fixed pool;
2. verify the 8-byte-rounded allocation also fits;
3. return `NULL` instead of advancing beyond the pool;
4. latch `_apk_oom` / `_tmp_oom` until reset;
5. expose `apk_oom()` / `tmp_oom()` for downstream gates;
6. reset the OOM latch together with the corresponding cursor.

Normal in-range allocation semantics and 8-byte alignment are preserved.

## Local falsifier / regression probe

A host harness replaying the exact changed allocator body was compiled with:

```sh
clang -std=c11 -Wall -Wextra -Werror test.c -I. -o test
./test
```

Observed result:

```text
PASS allocator fail-closed bounds
```

Probe coverage:

- exact-capacity allocation succeeds;
- next byte is rejected;
- cursor does not advance after rejection;
- OOM latch remains fail-closed until reset;
- reset clears cursor + latch;
- `APK_CAP-1` rounds safely to the 8-byte boundary;
- `(size_t)-1` is rejected before `n+7` can wrap;
- TMP pool has the same boundary behavior.

Harness source SHA-256: `e45c2b3d3532a67e400868abe713625afca40d08511c8e35dbae6fa5605ee7a7`  
Harness binary SHA-256: `98e59fbfba9c830ee97a64138e7c476d34ae442d404d2ec0ee488e2e314a67ca`

## Call-site census

Repository code search for `apk_alloc(` and `tmp_alloc(` returned only `Apkc/mem.h` itself at the base commit. No active caller was found. This means the change hardens a dormant/public utility path and should not alter the currently exercised ApkC build path.

This also changes the next priority: active-path hardening should target input/build/output paths rather than waiting for allocator consumers that do not currently exist.

## CI observation after draft PR creation

Draft PR: `#216` (`audit/apkc-mem-failclosed-20260812`).

Runs observed for head `edac479425257409e9f91a5bd8ff067aaa2c93ea`:

- `Formal Science Orchestrator` run `31586082725`: workflow conclusion `failure`; job `94080117180` returned zero steps.
- `CI` run `31586082737`: workflow conclusion `failure`; job `94080117575` returned zero steps; log fetch returned unavailable/404.
- `ApkC First Part Closure` run `31586082795`: workflow reported `failure`, while job `94080118361` was still reported `queued` with zero steps.

Per the fail-closed evidence contract these are **not attributed to the code change**. They are classified `TOKEN_VAZIO_RUNNER` / infrastructure-inconclusive until job steps/logs exist. A generic workflow-level `failure` with `steps=0` is not a code-test failure.

## Newly exposed active-path gap

Inspection of `Apkc/apkc.c` shows `_src_local[0x100000]` is filled only while `src_len < sizeof(_src_local)-1`; after the loop the compiler does not probe for an additional byte. Therefore an input at or above the buffer limit can be silently truncated and compiled from a prefix. This is an active fail-closed gap and is higher priority than dormant allocator call sites.

Status: `KNOWN_GAP / claim_allowed=false` until patched and falsified with an oversized-input negative test.

## Limitations / TOKEN_VAZIO

- `TOKEN_VAZIO`: the host harness replays the exact allocator body but does not include the repository `sys.h`, because that header intentionally emits ARM inline assembly and this receipt environment is x86_64.
- `TOKEN_VAZIO`: ARM32 physical runtime not executed in this receipt.
- `TOKEN_VAZIO`: ARM64 physical runtime not executed in this receipt.
- `TOKEN_VAZIO_RUNNER`: current PR workflows did not provide executable job steps/logs sufficient to classify the code.
- `TOKEN_VAZIO`: active `_src_local` truncation gap is identified but not yet patched in this receipt.

## Closure gate

This allocator gap is locally contained, but full branch promotion requires:

1. ARM-target compilation of the branch passes with real job steps/logs;
2. relevant existing ApkC validation/CI stays non-regressive;
3. the active source-input truncation path is made fail-closed and receives an oversized-input negative test.

## R3

- `F_ok`: fixed-pool overflow no longer advances allocator state; deterministic host boundary probe PASS; repository census found no active allocator consumers, reducing regression risk.
- `F_gap`: CI runner evidence is inconclusive; ARM physical execution remains open; active source-buffer truncation can still silently compile a prefix.
- `F_next`: patch `apkc_main` source ingestion to detect any byte beyond `_src_local` capacity and return a hard build error before `build_apk()`.
