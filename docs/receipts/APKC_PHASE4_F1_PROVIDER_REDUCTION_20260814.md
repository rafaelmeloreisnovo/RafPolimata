# ApkC Phase 4 — F1 Provider Reduction — 2026-08-14

State: `MATERIALIZED_FIX / PROVIDER_REEXECUTION_TOKEN_VAZIO`

Claim gate: `claim_allowed=false`

## Bound source

- PR under review: `#257`
- PR #257 head: `02817f1d2a2419a2df8db2b9653bb63a44ddbf77`
- failing provider CI run: `31851902830`
- failing job: `94929104954`
- stacked hardening PR: `#258`
- hardening head before this receipt: `7740eb7a376b68bbf289d42dfd661b1801f22632`

## Provider evidence

The provider runner executed the existing F1 hardened-source compile. Source-cap transformation and exact source-cap verification passed, then `clang -std=c11 -O2 -Wall -Wextra -Werror -target aarch64-linux-gnu -nostdlib -nostdinc -ffreestanding` failed with 11 diagnostics.

Observed classes:

1. `lang_expr.h`: three unused identifier placeholders;
2. `lang_expr.h`: `0xffffffff` passed to a `u8` register operand in `codegen_emit_xor`, producing a constant-conversion error and exposing an operand-kind bug;
3. `lang_stmt.h`: one unused type placeholder and six labels allocated but never emitted/consumed.

No `-Wno-error`, diagnostic suppression, or weakening of strict compilation is introduced.

## Materialized correction

`Apkc/lang_expr.h`:

- unused identifier placeholders removed;
- bitwise-not now materializes `0xffffffffu` with `MOVI` into a machine register before register-register XOR.

`Apkc/lang_stmt.h`:

- dead type placeholder removed;
- labels that were allocated but never connected to emitted control-flow instructions removed;
- comments state that these parsers remain structural until label emission is actually wired.

Diff against PR #257 head at materialization: two files, `+10/-19` before this receipt.

## Independent semantic blocker — not resolved by F1

`Apkc/lang_functions.h` is not proof of ARM64/AAPCS64 calling-convention conformance:

- only first three arguments are mapped to `r0-r2`;
- stack arguments are stored in metadata but the implementation says they "would be pushed here in a real implementation";
- current prologue emits `MOVI r14, local_size` followed by `SUB r14,r14,r14`, which zeros r14 rather than establishing a proven stack-pointer decrement.

Therefore:

`TOKEN_VAZIO_FUNCTION_ABI_CONFORMANCE` remains open.

No claim of AAPCS64, stack-argument execution, physical Android runtime, or device behavior is authorized.

## Additional provider finding

The raw-source audit emitted `state=FAIL`, `failures=7` in the same CI run, but the workflow step remained successful because the audit command does not currently enforce its reported failure state. This is tracked separately as:

`TOKEN_VAZIO_RAW_SOURCE_AUDIT_FAIL_CLOSED_ENFORCEMENT`.

It must not be hidden by the F1 compiler repair.

## F_ok

- provider runner real and exact failure observed;
- source-cap stage still PASS before strict compile;
- minimal F1 source fix materialized without warning suppression;
- broad ABI claim explicitly demoted to TOKEN_VAZIO;
- PR #257 received an append-only evidence comment.

## F_gap

- `TOKEN_VAZIO_PR258_PROVIDER_EXECUTION`
- `TOKEN_VAZIO_FUNCTION_ABI_CONFORMANCE`
- `TOKEN_VAZIO_FUNCTION_STACK_ARGUMENT_EMISSION`
- `TOKEN_VAZIO_FUNCTION_PROLOGUE_EPILOGUE_SEMANTICS`
- `TOKEN_VAZIO_RAW_SOURCE_AUDIT_FAIL_CLOSED_ENFORCEMENT`
- physical Android/Termux execution remains outside this evidence boundary.

## F_next

1. execute PR #258 exact head on provider and require F1 strict compile PASS;
2. add executable falsifiers for function prologue/epilogue and 4th+ argument transport before changing ABI claims;
3. repair raw-source audit enforcement separately so `state=FAIL` cannot produce a green CI step;
4. preserve `claim_allowed=false` until each independent gate has evidence.
