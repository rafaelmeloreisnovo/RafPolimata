# ApkC physical Android proof-chain hardening receipt — 2026-08-12

status: `VERIFIED_STATIC`
claim_allowed: `false`
base_main: `3f2873b6c8cf56a4a8e116eab40523af03354e92`
branch: `audit/apkc-runtime-proof-hardened-20260812`
implementation_commit: `dbe90101912a4451168a2798093d7a595cb272c7`
file: `scripts/capture_android_proof_chain.sh`
content_blob: `701e4f3c165287c4e8fc78ef986cc2c01877a682`

## P0 raw-source bypass mitigated statically

Before this change the physical proof chain could compile raw `Apkc/apkc.c` directly. The path now refuses compilation until `scripts/patch_apkc_runtime_source.py` produces and verifies a runtime-hardened translation unit.

Precompile contract:

1. require raw source, Python3 and runtime transformer;
2. generate `OUT_DIR/apkc-runtime-hardened.c` plus runtime hardening manifest;
3. require `source exceeds 1MiB bounded input` marker;
4. require `APKC-RH-013` in the manifest;
5. require the legacy `if (n<=0) break;` anchor to be absent;
6. record raw and runtime-hardened SHA-256 values;
7. compile only the generated TU with `-I Apkc`.

The GitHub file was read back after write and these gates are present.

## Anti-stale / fail-closed improvements

The same change also removes stale outputs before compiler/APK/signing steps and makes PASS depend on command result plus material output:

- compiler PASS requires exit 0 + executable;
- APK generation PASS requires exit 0 + non-empty APK;
- unzip/readelf PASS requires exit 0;
- signed APK PASS requires signing success + non-empty output + `apksigner verify` exit 0;
- Android install PASS requires `adb install` or `pm install` exit 0;
- launch PASS requires the launch command exit 0;
- runtime remains `AUDIT` after logcat capture; launch success alone is not promoted to crash-free runtime.

The custody manifest advances to `rafpolimata.android_proof_chain.v2` and records:

`raw ApkC SHA → runtime-hardened ApkC SHA → input source SHA → ApkC ELF SHA → APK SHA → signed APK SHA`.

## Remaining signing-policy gap

A narrower fail-closed policy gap remains: when `DO_SIGN=1` is explicitly requested and signing/verification fails, the current variable fallback can still reference the raw APK for the later install section. This is not classified as a source-hardening bypass, but it is a `KNOWN_GAP/P1` because an explicit signing requirement should block installation rather than degrade to an unsigned/raw candidate.

closure_gate: introduce an explicit `SIGN_REQUIRED_FAILED`/empty install candidate state; when `DO_SIGN=1`, any sign or verify failure must make install/launch `TOKEN_VAZIO_BLOCKED_BY_SIGNING` and no install command may run.

## Evidence boundary

Not executed yet on this exact branch/device:

- runtime transformer on-device;
- ApkC compilation;
- APK generation;
- signature verification;
- physical installation;
- launch;
- semantic logcat/crash-free proof.

Therefore:

- `ANDROID_CAPTURE_RAW_SOURCE_BYPASS = MITIGATED_STATIC`
- `ANDROID_CAPTURE_ANTI_STALE = VERIFIED_STATIC`
- `ANDROID_SIGNING_REQUIRED_FAIL_CLOSED = KNOWN_GAP/P1`
- `ANDROID_PHYSICAL_CURRENT_CHAIN = TOKEN_VAZIO`
- `claim_allowed = false`

Historical `RafaelTeste` installation evidence remains historical and is not cryptographically bound to this branch.

F_next: after closing the explicit-signing fallback, execute this exact script on the Android/Termux target and archive status.tsv + manifest.json + hardening/build/APK/sign/install/launch/logcat receipts.
