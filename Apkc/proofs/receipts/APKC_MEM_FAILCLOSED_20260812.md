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

## Limitations / TOKEN_VAZIO

- `TOKEN_VAZIO`: the host harness replays the exact allocator body but does not include the repository `sys.h`, because that header intentionally emits ARM inline assembly and this receipt environment is x86_64.
- `TOKEN_VAZIO`: ARM32 physical runtime not executed in this receipt.
- `TOKEN_VAZIO`: ARM64 physical runtime not executed in this receipt.
- `TOKEN_VAZIO`: downstream call sites have not yet been audited to ensure every `NULL` return is converted into an explicit build error rather than a later fault.

## Closure gate

This gap is not fully closed until:

1. ARM-target compilation of the branch passes;
2. relevant existing ApkC validation/CI stays non-regressive;
3. all `apk_alloc` / `tmp_alloc` call sites either check `NULL` or a central fail-closed gate proves the OOM latch before use.

## R3

- `F_ok`: fixed-pool overflow no longer advances the allocator; deterministic host boundary probe PASS.
- `F_gap`: ARM execution and downstream NULL-consumer audit remain open.
- `F_next`: enumerate allocator call sites and harden the first unchecked consumer, then run the repository ApkC validation workflow.
