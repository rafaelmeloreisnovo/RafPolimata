# ApkC — Capability & Action Proof Audit — 2026-08-14

Status: `AUDIT_CANDIDATE / claim_allowed=false`

## Provenance

- Repository: `rafaelmeloreisnovo/RafPolimata`
- Audited base commit: `2f790cbf47d3770ce95938cf90839f9fce942ed6`
- Audit branch: `audit/apkc-capability-action-proof-20260814`
- Candidate workflow file: `apkc-capability-action-proof.v1.2026-08-14.yml`
- Candidate workflow SHA-256: `7a1df2fcfa679b95cf617e93db03ac1671633ca6d39c335240f2fca81379178e`
- Candidate workflow size: `22674 bytes`
- Candidate workflow lines: `620`
- Canonical raw copy preserved append-only in the RAFAELIA file library under `RAFAELIA/RafPolimata/ApkC/Capability-Action-Proof/`.

No execution result is claimed by this document. The workflow is a falsifiable candidate and is expected to expose current semantic gaps until they are fixed.

## Audit conclusion

The repository already contains a strong structural foundation for a source-to-artifact proof:

1. `tools/raf_source_to_binary_proof.sh` hardens `Apkc/apkc.c`, compiles AArch64 and ARM32 artifacts, identifies ELF class/machine, repeats builds and requires byte-identical reproducibility before promotion.
2. `.github/workflows/apkc-first-part.yml` can run the AArch64 compiler under QEMU, build an APK, validate DEX/ELF/ABI and emit a bounded structural receipt.
3. `scripts/apkc_verified_build.py` uses private candidate generation and only promotes a structurally valid APK.

The missing closure is semantic/action proof, not another layer of architecture.

## Material gaps observed at base commit

### G-S1 — language identity mismatch

`Apkc/apkc_branchless_handler.h` documents language IDs in a different order from `Apkc/lang_profile.h`. The active dispatcher passes the LangProfile table index to `apkc_branchless_compile()`. This must become one canonical language identity contract before language-sensitive routing is trusted.

### G-S2 — `compile_universal()` does not yet prove language-specific compilation

At the audited base, `Apkc/compiler_language_direct.h` contains `(void)lang` in `compile_universal()` and uses simple source pattern detection such as `+` / `return`, with a fallback that can emit code even when source semantics were not parsed. Therefore:

`COMPILE_PASS != SEMANTIC_PASS`.

### G-S3 — statement parsing is partly structural-only

`Apkc/lang_stmt.h` explicitly describes `if`, `while` and `for` paths as structural parsing until control labels are wired. Several declaration/assignment paths consume tokens without emitting equivalent executable semantics.

### G-S4 — expression operators still contain placeholders/simplifications

Comparison/equality and some unary/logical paths in `Apkc/lang_expr.h` do not yet demonstrate full language semantics. A proof must compare actual program results, not merely instruction presence.

### G-S5 — VM validation is skipped in active branchless handler

`Apkc/apkc_branchless_handler.h` currently states that execution validation is skipped in Phase 3a. Therefore `steps_executed==0` is a valid falsifier of any claim that source semantics were already executed by that path.

### G-A1 — native code layout/relocation is not closed

`Apkc/apkc_machine_to_arm64.h` can expand one VM `MOVI` into two AArch64 instructions, while jump/call immediates are encoded directly from `insn->imm >> 2`. A valid backend needs an explicit mapping:

`VM instruction index -> native byte offset -> relocation`.

Logical VM PC is not generally equal to native PC/4.

### G-A2 — CALL/RET contract is inconsistent

The VM and AArch64 paths do not yet share one explicit link-register/stack contract. The current AArch64 CALL path emits `B` while RET uses `X30`. This must be reconciled before nested or ordinary calls can be claimed correct.

### G-A3 — ELF symbol offsets must use the same layout map

The branchless APK path exports `ANativeActivity_onCreate` at byte offset 0 and `android_main` at byte offset 4. Because a logical instruction can expand to multiple native instructions, symbol offsets cannot be derived from a fixed logical-instruction stride. Branch relocation, CALL targets and exported symbol offsets must use the same layout table.

## Capability/action proof contract

The candidate pipeline is defined as a sequence of fail-closed gates:

- `G0`: reproducible compiler source -> AArch64 + ARM32 artifacts.
- `G1`: hardened-source strict freestanding compile.
- `G2`: raw-source anti-bypass fail-closed.
- `G3`: semantic concepts executed through the active branchless path.
- `G4`: generated AArch64 bytes executed under `qemu-aarch64`, with observed result equal to expected result.
- `G5`: the `.text` packaged inside the generated APK is byte-identical to the native bytes independently executed in G4.
- `G6`: real Android APK signing with `apksigner`, positive verification and negative tamper rejection.

Initial semantic falsifiers are deliberately small:

| Concept | Expected result |
|---|---:|
| literal `return 42` | 42 |
| precedence `2 + 3 * 4` | 14 |
| conditional `3 < 5 ? 7 : 9` | 7 |
| loop sum `1..5` | 15 |
| five-argument call `1+2+3+4+5` | 15 |

The proof is stronger when these minimal programs fail early than when large fixtures merely produce files.

## Required invariants

- `IDEA != IMPLEMENTATION != EXECUTION != EVIDENCE != CLAIM`
- `COMPILE_PASS != SEMANTIC_PASS`
- `VM_PASS != NATIVE_ACTION_PASS`
- `NATIVE_ACTION_PASS != APK_RUNTIME_PASS`
- `APK_STRUCTURAL_PASS != APK_SIGNED_PASS`
- `QEMU_ACTION != PHYSICAL_ANDROID_ACTION`
- `TOKEN_VAZIO != PASS`
- unknown or unavailable physical-device evidence remains `TOKEN_VAZIO`
- `claim_allowed=false` until the scoped gates produce evidence and the claim scope is explicitly bounded

## F_gap

1. canonicalize language IDs and dispatch precedence;
2. wire language-specific parser/compiler semantics;
3. make statements and expressions emit executable-equivalent IR;
4. enable VM execution as an oracle in the active branchless handler;
5. separate VM SP/LR semantics explicitly;
6. implement deterministic native layout and relocation;
7. fix CALL/RET and symbol offsets from that layout;
8. execute the generated native bytes and compare observed outputs;
9. bind executed bytes to APK `.text`;
10. sign and verify with real Android signing tooling;
11. keep physical Android install/launch as `TOKEN_VAZIO` until device evidence exists.

## F_next

Integrate the candidate workflow on this audit branch without weakening it to obtain green CI. Current red gates are expected evidence. Fix the earliest falsified invariant first, rerun, and preserve each prior receipt append-only. Do not merge solely because a structural build passes.
