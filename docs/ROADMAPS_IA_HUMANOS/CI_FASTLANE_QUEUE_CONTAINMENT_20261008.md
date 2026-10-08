# CI fastlane and queue containment — 2026-10-08

Copyright (c) 2026 Rafael Melo Reis. Retain original project license and provenance.

CI fastlane. Feature branch push+pull_request duplicated CI, Internal Custody Ledger and Formal Science. Change: automatic push on main only; draft PR runs quick source/coherence/TOKEN_VAZIO checks; ready_for_review and main run original full gates; stable PR concurrency cancels obsolete jobs. Quick is NOT full, Android physical NOT_RUN. Rights preserved; rollback by reverting PR. SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM.

## Steps for a human or AI operator

1. Keep work in a Draft PR; inspect quick source-contract results without promoting them to binary PASS.
2. Mark Ready for review to trigger all retained full PR/ABI tests that apply.
3. Review source and rights; merge only with required checks and provider branch-protection readback.
4. Invoke expensive producer/beta/publish workflows deliberately after source is stable; capture exact source SHA, run ID and artifacts.
5. Never call source-only, QEMU-only or CI-only evidence physical Android validation. Record TOKEN_VAZIO, NOT_RUN, FAIL or PASS precisely.

R3 = <F_ok: source gate/refactor on PR, F_gap: CI executed exact head, provider P0, hardware receipts, F_next: CI quick -> ready full -> explicit delivery -> device readback>.
