# ApkC — Scoped Semantic Return Oracle — Cycle 2 — 2026-08-14

Status: `SCOPED_SEMANTIC_ACTION_EVIDENCE / claim_allowed=false`

This receipt is append-only. It follows:

1. `APKC_CAPABILITY_ACTION_PROOF_AUDIT_20260814.md`;
2. `APKC_CANONICAL_LANG_DISPATCH_CYCLE1_20260814.md`;
3. `APKC_CANONICAL_LANG_DISPATCH_CYCLE1_CI_20260814.md`.

## Provenance

```text
repository = rafaelmeloreisnovo/RafPolimata
branch = fix/apkc-canonical-lang-dispatch-20260814
pr = 271
base_main = c6658c2486bcff60213d12e19ce9d5a5e4a81e08
validated_head = a313789f96456566df982283bb4626880d9c5b9b
claim_allowed = false
```

## Invariant selected

The predecessor audit required the first semantic/action falsifiers:

```text
return 42        -> 42
2 + 3 * 4        -> 14
```

Cycle 2 deliberately closes only a bounded common statement-fragment grammar:

```text
return-stmt := "return" expr [";"] EOF
expr        := term { ("+" | "-") term }
term        := factor { ("*" | "/" | "%") factor }
factor      := uint32 | "(" expr ")" | "-" factor
```

This grammar is **not** promoted to full C, Rust, Python, Go, Java, JavaScript or Swift semantics.

## Implementation delta

Added:

```text
Apkc/apkc_semantic_return_subset.h
tests/test_apkc_semantic_return_oracle.c
.github/workflows/apkc-semantic-return-oracle.yml
```

Updated:

```text
Apkc/apkc_language_dispatch.h
Apkc/apkc_branchless_handler.h
.github/workflows/apkc-canonical-lang-dispatch.yml
```

### Exact route

```text
canonical LP_* validation
-> bounded return-arithmetic parser when matched
-> MachineLinear instructions
-> exact copy into ExecutionContext.code
-> scoped VM execution
-> observed r0
-> independent expected-value assertion in CI
-> same MachineLinear stream accepted by ARM64 encoder
```

If a source begins with `return` but violates the bounded grammar, the dispatcher fails closed instead of falling through to a permissive structural frontend.

Non-matching source still follows the existing language-specific structural frontend and is explicitly left semantically unproved.

## VM return convention

The bounded subset emits ordinary `OP_RET`, which already maps to native `RET X30` in the ARM64 encoder.

For the synthetic VM caller used only by this scoped oracle:

```text
r14 = code_len
```

before execution. Therefore VM `RET` exits to the synthetic caller boundary. This does **not** close the general CALL/RET ABI gap `G-A2`.

## Focused semantic CI evidence

```text
workflow = ApkC Scoped Semantic Return Oracle
run_id = 31859879049
job_id = 94951233044
head_sha = a313789f96456566df982283bb4626880d9c5b9b
conclusion = success
```

Observed gate:

```text
compile strict gate                PASS
execute expected-value falsifiers PASS
freestanding syntax audit         PASS
preserve receipt                  PASS
```

The prior run at the same logic also observed `26/26 PASS`; the latest head rerun remained green after compatibility hardening.

Required semantic observations:

```text
return 42            -> vm_result 42  PASS
return 2 + 3 * 4     -> vm_result 14  PASS
return (2 + 3) * 4;  -> vm_result 20  PASS
invalid `return 2 +` -> fail closed    PASS
normal structural path -> VM not run, semantic proof unpromoted PASS
```

Artifact:

```text
name = apkc-semantic-return-oracle-receipt
artifact_id = 9240195832
size = 346 bytes
sha256 = cb77e40f4ccca8062cca427faa7c4166935124faadfe0c1db56b10924adc14d6
```

## Anti-regression gate

The canonical workflow was expanded to run the historical `test_apkc_branchless_integration.c` suite in addition to the new canonical identity gate.

```text
workflow = ApkC Canonical Language Dispatch
run_id = 31859879090
head_sha = a313789f96456566df982283bb4626880d9c5b9b
conclusion = success
canonical gate = 28/28 PASS
historical branchless integration = 53/53 PASS
```

Its artifact is:

```text
artifact_id = 9240196830
size = 973 bytes
sha256 = 4d336b6a109dbb8b60e0adaa42bdd51fb8c0d400b9c0a42142c40227af5c478d
```

The historical status-string contract requiring status 2 to begin with `o` was preserved as `overflow_or_encode_error`, avoiding a needless compatibility regression.

## Same-head transversal evidence

At `a313789f...` the following were observed:

```text
ApkC Scoped Semantic Return Oracle  success
ApkC Canonical Language Dispatch    success
Formal Science Orchestrator         success
Document Governance                 success
CI                                  success
ApkC First Part Closure             all substantive steps PASS; final cleanup observed in progress at inspection time
```

The last line is not promoted to final workflow success until GitHub reports the run completed.

## Closure matrix

```text
G-S1 canonical language identity        = CODE_PLUS_CI_EVIDENCE
G-S2 active routing                     = CODE_PLUS_CI_EVIDENCE
G-S3 return arithmetic fragment         = SCOPED_CODE_VM_CI_EVIDENCE
G-S3 declarations/assignments/functions = OPEN
G-S4 arithmetic precedence subset       = SCOPED_CODE_VM_CI_EVIDENCE
G-S4 comparisons/equality/calls         = OPEN
G-S5 VM action oracle                   = SCOPED_ACTIVE
G-A1 native layout/relocation            = OPEN
G-A2 general CALL/RET ABI                = OPEN
G-A3 ELF symbol/native layout binding    = OPEN
physical Android                         = TOKEN_VAZIO
full seven-language semantics            = TOKEN_VAZIO
claim_allowed                            = false
```

## F_ok

The first two capability/action falsifiers now have source -> machine -> VM action -> expected-value comparison -> ARM64-encode evidence, while the pre-existing 53-test branchless suite remains green.

## F_gap

Variables, assignments, function bodies, comparisons, equality, ternary semantics, loops, general calls/returns, relocations, native execution equivalence and physical Android remain independent gaps.

## F_next

Next semantic vertex: close comparison/equality as a bounded subset with positive and negative expected-value falsifiers, then use that result to build a minimal conditional branch. Do not jump directly to loops or full-language claims before comparison truth is executable.
