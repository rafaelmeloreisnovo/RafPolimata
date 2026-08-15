# ApkC — Canonical Language Dispatch — Cycle 1 CI Evidence — 2026-08-14

Status: `G-S1_CODE_PLUS_CI_EVIDENCE / claim_allowed=false`

This receipt appends evidence to `APKC_CANONICAL_LANG_DISPATCH_CYCLE1_20260814.md`; it does not rewrite the predecessor state.

## Provenance

```text
repository = rafaelmeloreisnovo/RafPolimata
branch = fix/apkc-canonical-lang-dispatch-20260814
head_sha = 1d94100c072b381176f6d4650979bb12ef2ef265
base_main = c6658c2486bcff60213d12e19ce9d5a5e4a81e08
pr = 271
```

## Antiderivative: first red run preserved

```text
workflow = ApkC Canonical Language Dispatch
run_id = 31859492546
conclusion = failure
failed_step = Compile strict host gate
```

Root cause observed in logs:

- host test included libc `stdio.h`, while the imported ApkC syscall layer also defined `NULL`;
- `lang_profile.h` unnecessarily pulled `sys.h` into pure language-identity validation;
- x86_64 host compilation consequently entered the non-AArch64 syscall branch, exposing ARM32 pointer-width/inline-ASM warnings under `-Werror`.

This was classified as a harness/coupling falsifier, not as proof against canonical dispatch.

The fix did **not** relax `-Werror`.  Instead, `lang_profile.h` was decoupled from `sys.h` and now contains only profile data/lookup logic, using `__SIZE_TYPE__` where a size type is required.

## Green rerun

```text
workflow = ApkC Canonical Language Dispatch
run_id = 31859553562
job_id = 94950332846
head_sha = 1d94100c072b381176f6d4650979bb12ef2ef265
conclusion = success
```

Observed successful steps:

```text
Checkout exact PR head                PASS
Compile strict host gate              PASS
Run canonical identity/fail-closed    PASS
Freestanding syntax audit             PASS
Preserve gate receipt                 PASS
```

Focused runtime assertions:

```text
pass = 28
fail = 0
```

The gate includes positive and negative cases for canonical `LP_*` routing, unsupported profile rejection, out-of-range rejection, empty-input rejection, frontend-state recording, VM-buffer identity, and deliberate non-promotion of semantic proof.

## Artifact

```text
artifact_name = apkc-canonical-lang-dispatch-receipt
artifact_id = 9240099885
artifact_size = 566 bytes
artifact_zip_sha256 = e965c2136d1aff15f3a581c806bc7cca751e80d437d8d5220a8bfba451122a1e
```

## Transversal gates at the same head

The pull-request workflow set for head `1d94100c...` completed without a failure observed in the branch run set.  `Formal Science Orchestrator` run `31859553557` completed `success`.

This does not imply semantic/runtime closure beyond the scoped Cycle 1 gate.

## Promotion boundary

```text
G-S1 = CODE_PLUS_CI_EVIDENCE
G-S2_ACTIVE_ROUTING = CODE_PLUS_CI_EVIDENCE
G-S2_FULL_SEMANTICS = OPEN
G-S3 = OPEN
G-S4 = OPEN
G-S5 = OPEN
G-A1 = OPEN
G-A2 = OPEN
G-A3 = OPEN
PHYSICAL_ANDROID = TOKEN_VAZIO
claim_allowed = false
```

## F_ok

Canonical language identity and active language-specific routing now have repository code plus focused CI evidence.

## F_gap

The currently selected statement/expression frontends still do not establish semantic equivalence.  In particular, return expressions, precedence action proof, control flow, calls, comparison operators, VM execution, native relocation and physical Android remain independent gates.

## F_next

Cycle 2 may now start at the first semantic falsifier only: compile a bounded arithmetic `return` statement to the VM, execute the exact generated VM instruction stream, and require observed results `42` and `14` for `return 42` and `return 2 + 3 * 4` respectively.  Do not generalize that evidence to full C/Python/Rust semantics.
