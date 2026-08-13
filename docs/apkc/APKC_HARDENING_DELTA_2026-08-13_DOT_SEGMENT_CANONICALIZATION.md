# ApkC/RAFAELIA — Hardening Delta — Dot-Segment Canonicalization — 2026-08-13

## Scope
Close pathname identity ambiguity where filesystem-equivalent paths such as `07_signed/app.apk` and `07_signed/./app.apk` could represent the same bytes with different textual identities.

## Provenance
- Branch: `hardening/apkc-android-final-verifier-20260813`
- Producer patch commit: `9fd4132f79ba440d62e4bc37b28ce1dd75308000`
- Verifier patch commit: `8e6e0193dceaeeaba4400d7aa345783444afe868`
- Adversarial test commit/head before this record: `ce91ea34bc9a08a00189b97aa075898781f06dd1`

## F_ok
- `dot_segment_gap = VERIFIED_SOURCE_INSPECTION`
- `producer_patch = LANDED`
- `verifier_patch = LANDED`
- `adversarial_rehash_test_source = LANDED`
- `claim_allowed = false`

Producer and verifier now reject relative pathname forms containing any canonical dot-segment alias: `.`, `./*`, `*/./*`, `*/.` in addition to existing parent traversal rejection.

The adversarial source covers both:
1. producer rejection of `07_signed/./app.apk` even when it resolves to an existing APK;
2. verifier rejection after selector, final state, manifest path and affected hashes are recomputed around `07_signed/./app.apk`.

## CI observation
For head `ce91ea34bc9a08a00189b97aa075898781f06dd1`, GitHub created the ApkC workflow but the observed job completed with `steps=[]`, `runner_id=0`, empty runner name and no observable code step. Classification: `RUNNER_STARTUP_FAIL`, not `CODE_FAIL`.

## F_gap
- `HEAD_EXACT_TEST = TOKEN_VAZIO`
- `CI_CODE_EXECUTION = TOKEN_VAZIO_RUNNER_EXECUTION`
- `TERMUX_ARM32_CURRENT_RUN = TOKEN_VAZIO`
- `REAL_SIGNING_CERTIFICATE = TOKEN_VAZIO`
- `PHYSICAL_INSTALL = TOKEN_VAZIO`
- `PHYSICAL_LAUNCH = TOKEN_VAZIO`
- `PHYSICAL_RECEIPT_RELOCATION = TOKEN_VAZIO`
- `RUNTIME_SEMANTIC_PASS = TOKEN_VAZIO`
- `CROSS_DEVICE_DETERMINISM = TOKEN_VAZIO`

## F_next
1. Execute `bash tests/test_apkc_android_final_verifier.sh` from an exact checkout of the current head and preserve commit SHA, SHA-256 of producer/verifier/test, transcript SHA-256 and exit code.
2. Execute producer -> independent verifier on physical Termux/ARM32.
3. Copy the complete `OUT_DIR` to another pathname/storage and repeat verification.
4. Apply controlled tamper and require rejection.
5. Only then promote `PHYSICAL_RECEIPT_RELOCATION`; keep runtime semantics as an orthogonal gate.

## Invariant
Implementation != execution != evidence != claim. Missing proof remains `TOKEN_VAZIO`; negative evidence is preserved; no runtime or physical claim is promoted by source changes alone.
