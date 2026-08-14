# ApkC/RAFAELIA — TLS Static + Post-Merge Provider Reduction — 2026-08-14

State: `VERIFIED_LIMITED_PROVIDER`

Claim gate: `claim_allowed=false`

## Scope

This receipt records provider execution for PR #242 after the post-#240 main state. It distinguishes static HTTPS transport evidence from TLS runtime certification and records the remaining ApkC blocker without weakening strict mode.

## Exact provider execution

- PR: `#242`
- head: `0c1125931caa6b3246e7f9740b1bfb12b62afa38`
- workflow: `ApkC First Part Closure`
- run: `31807993811`
- job: `94791460904`
- hosted runner: Ubuntu 24.04
- artifact: `9221954176`
- artifact bytes: `564022`
- artifact SHA-256: `dae6a2e4c7f6d782d3dc2fac7289cf2c90398b6a821598564ff5f8165adc18f9`

## F_ok

Provider-observed PASS:

- syntax/contracts source checks: PASS;
- Python suite: `35/35 PASS`;
- custody hardening: `8/8 + 6/6 + 7/7 + 4/4 + 9/9 = 34/34 PASS`;
- source-to-binary proof: PASS for AArch64 and ARM32;
- runtime source hardening: PASS, 13 transforms, byte-deterministic across two transforms;
- runtime hardening proof: PASS on AArch64 and ARM32;
- first-part code gate: `8/8`, contradictions=`0`, state=`PASS`;
- browser/TLS contracts step: PASS;
- static HTTPS adapter CA policy audit: corrected to inspect executable non-comment shell text and reject active `-k` or `--insecure`;
- v3 proof genealogy restored explicitly: `raf.apkc.source-to-binary-proof.v3 supersedes source-to-binary-proof.v2`.

The HTTPS adapter remains classified as static transport evidence only. Runtime TLS certification is not promoted:

- `runtime_state=TOKEN_VAZIO`;
- `certified_tls=TOKEN_VAZIO`;
- `claim_allowed=false`.

## F_gap

The overall workflow remains FAIL because `GENERATE_APK_OUTCOME=failure`; the structural receipt then fails downstream because the expected `hello.apk` is absent/not validly produced.

The provider log and preserved verified-build evidence identify the generation failure as strict ARM32 assembly coverage: `Apkc/hello.s.txt` produces 39 unsupported ARM32 instructions classified as `UNDEF`; strict mode refuses APK emission.

This gap is preserved as:

`TOKEN_VAZIO_ARM32_STRICT_ASM_COVERAGE_HELLO_FIXTURE`

The following remain outside the current proof boundary:

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## Anti-regression boundary

Do not close the generation gap using `--allow-undef`. The current fail-closed behavior is useful evidence: unsupported instructions must either gain deterministic ARM32 encoding/lowering with falsifiers or the canonical dual-ABI fixture must be changed under an explicit contract proving why the changed instruction set is the intended coverage target.

## F_next

1. enumerate the 39 unsupported ARM32 mnemonics from the exact provider transcript;
2. classify each as parser normalization gap, missing encoding, architecture-incompatible instruction, or fixture mismatch;
3. implement the smallest deterministic encoding/lowering set with positive and adversarial tests;
4. rerun the exact `-both --strict` generation path;
5. only after `hello.apk` is produced and validated rerun C04 structural closure;
6. keep all Android physical/runtime states `TOKEN_VAZIO` until device evidence exists.
