# ApkC anti-retraction post-merge receipt — 2026-08-12

status: `VERIFIED_LIMITED`
claim_allowed: `false`
repo: `rafaelmeloreisnovo/RafPolimata`
source_branch: `audit/apkc-mem-failclosed-20260812`

## Custody boundary

PR #216 was merged into `main` at 2026-08-12T11:49:32Z.

- merged PR: `#216`
- merged head snapshot: `c5d939c67e5016c02dfc5428dff2637c1d1b3e2a`
- main merge commit: `88a7559e2dc3966d8d0a8faaa9de60e38d4c273d`
- merge commit verification: GitHub `verified=true`

The original audit branch continued receiving commits after the merged snapshot. Those later commits are **not represented by PR #216** and are therefore tracked separately in draft PR #218.

At creation of PR #218:

- head: `7b03099e1a809620a93556f29201f34c9fd23389`
- compare against then-current `main`: `ahead_by=5`, `behind_by=24`, `status=diverged`
- PR #218: open, draft, not merged

No statement that these post-merge changes are present in `main` is allowed until an explicit later merge/commit proves it.

## Evidence preserved from the merged line

The merged #216 line contains the allocator/source-cap hardening and the canonical/validator/Termux protections up to head `c5d939c...`.

Prior local evidence remains scoped to its original environment:

- source-cap transformer falsifier: PASS
- transformer SHA-256: `238736d72ee7288b31b2994c78c62838779603371b9f914d4ad829eadbc0d0dc`
- falsifier SHA-256: `176689a7793c38c4b47d910ab783c355735d401571cee10883960f447edac982`
- allocator boundary harness: PASS
- allocator harness source SHA-256: `e45c2b3d3532a67e400868abe713625afca40d08511c8e35dbae6fa5605ee7a7`
- allocator harness binary SHA-256: `98e59fbfba9c830ee97a64138e7c476d34ae442d404d2ec0ee488e2e314a67ca`

## Post-merge anti-retraction delta

The following changes exist on the continuation branch / PR #218 and remain subject to reconciliation with current main:

1. `scripts/apkc_lang_coverage.sh`
   - refuses raw `Apkc/apkc.c` compilation;
   - requires source-cap transformer/falsifier;
   - builds only a run-scoped hardened TU;
   - keeps unrunnable-host language gates as `TOKEN_VAZIO`.

2. `scripts/apkc_api_abi_matrix.sh`
   - same mandatory hardened-source contract for the API/ABI matrix;
   - does not promote runtime matrix cells without an executable ApkC.

3. `Makefile`
   - canonical `syntax` depends on `apkc-hardened-source`;
   - generated TU is falsified before syntax check;
   - vulnerable anchor surviving transformation is a hard failure.

4. `tools/raf_source_to_binary_proof.sh`
   - mandatory source-cap transformer/falsifier before compiler invocation;
   - AArch64 and ARM32 build/identity/reproducibility use the same hardened TU;
   - raw and hardened source SHA-256 are recorded;
   - canonical PASS is not promoted on partial failure.

## Transformer contract / uncertainty reduction

Static inspection proves that `scripts/patch_apkc_source_cap.py` and `scripts/patch_apkc_runtime_source.py` both replace the same vulnerable source-read anchor containing `if (n<=0) break;`.

The runtime transformer already includes `APKC-RH-013`, which changes that read path to:

- separate read error from EOF;
- probe beyond the bounded source buffer;
- reject oversized input;
- reject source close failure.

Therefore the current safe contract is **alternative hardeners by execution path**, not serial composition:

- structural/proof/validator paths → `source-cap-hardened` TU;
- runtime/QEMU/device proof paths → `runtime-hardened` TU, whose `APKC-RH-013` supplies the bounded-source gate.

Running source-cap first and then the current runtime transformer, or vice versa, is not a supported composition because the first exact-anchor replacement removes the second transformer's expected anchor. This is `KNOWN_CONTRACT_CONFLICT`, not a reason to silently bypass either hardener.

Directly editing canonical `Apkc/apkc.c` is postponed until every exact-anchor consumer is migrated or made compatible. This is intentional anti-retraction behavior.

## Remaining executable P0 bypasses

Static inspection still identifies:

### P0-A — QEMU ARM64 proof
`scripts/arm64_apk_qemu_proof.sh` cross-compiles raw `Apkc/apkc.c` before running QEMU and producing/signing/verifying an APK.

closure_gate: route its compile input through `patch_apkc_runtime_source.py`, verify runtime hardening manifest/markers, then rerun QEMU evidence with raw→runtime-hardened→ELF→APK hashes.

### P0-B — physical Android proof capture
`scripts/capture_android_proof_chain.sh` has an `ensure_apkc` path that compiles raw `Apkc/apkc.c` before APK generation/install/runtime capture.

closure_gate: route `ensure_apkc` through a hardened generated TU, archive raw/hardened/ELF hashes, then bind the resulting APK/sign/install/launch/logcat evidence.

### Classified non-bypass
`scripts/java_dex_pipeline_probe.sh` does not compile `Apkc/apkc.c`; it probes Java/Javac/D8/fork boundaries and preserves unavailable execution as `TOKEN_VAZIO`. It is not classified as a source-cap bypass.

## CI / runtime epistemic state

Earlier GitHub Actions failures on this line included jobs with `steps=[]`, `runner_id=0`, and empty runner names. They remain `TOKEN_VAZIO_RUNNER`; workflow conclusion alone is not evidence of code regression.

The historical Android screenshot proving an installed `RafaelTeste` app is evidence of physical installation of that historical artifact. It is **not** cryptographically bound to the current branch's source/ELF/APK and therefore does not close the present end-to-end gate.

Current status matrix:

- `ALLOCATOR_FAIL_CLOSED_LOCAL`: VERIFIED
- `SOURCE_CAP_TRANSFORMER_LOCAL`: VERIFIED
- `MERGED_PR_216_PROVENANCE`: VERIFIED
- `POSTMERGE_PR_218_DELTA`: VERIFIED_STATIC
- `TRANSFORMER_SERIAL_COMPOSITION`: KNOWN_CONTRACT_CONFLICT
- `QEMU_RAW_SOURCE_BYPASS`: KNOWN_GAP / P0
- `ANDROID_CAPTURE_RAW_SOURCE_BYPASS`: KNOWN_GAP / P0
- `CURRENT_BRANCH_CANONICAL_PROOF_EXECUTION`: TOKEN_VAZIO_RUNNER
- `CURRENT_BRANCH_ARM_BOUNDARY_TRIAD`: TOKEN_VAZIO
- `CURRENT_SOURCE_TO_INSTALL_RUNTIME_CHAIN`: TOKEN_VAZIO
- `claim_allowed`: false

## Anti-regression provisions

A gap is closed only when its `closure_gate` has a command/result/hash/runtime receipt. The following are forbidden as closure evidence by themselves:

- documentation or PR description;
- workflow `success/failure` without started steps/logs;
- an historical screenshot without artifact hash binding;
- source presence without build execution;
- generated APK without structural/signature/runtime chain where that claim requires those layers.

Any reconciliation of PR #218 with current `main` must preserve all already-merged #216 protections and must not remove tests, receipts, hardening gates or `claim_allowed=false` boundaries merely to resolve conflicts.

## F_ok / F_gap / F_next

`F_ok`: #216 merge provenance verified; post-merge delta isolated into draft #218; four additional proof/build routes are statically routed away from raw source; transformer overlap is explicitly classified instead of hidden.

`F_gap`: branch is behind current main; QEMU and Android capture remain raw-source P0 bypasses; runner execution, ARM boundary triad and exact current source→ELF→APK→install→runtime chain remain unproven.

`F_next`: reconcile the #218 delta onto current `main` without regression, then modify the QEMU proof to generate/compile the runtime-hardened TU and require `APKC-RH-013` evidence before any QEMU/APK PASS.
