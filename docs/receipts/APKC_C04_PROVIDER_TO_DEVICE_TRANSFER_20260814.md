# ApkC / RAFAELIA — C04 Provider → Device Transfer Gate — 2026-08-14

State: `LOCAL_REAL_ARTIFACT_PASS / PROVIDER_IDENTITY_BOUND / claim_allowed=false`

## Purpose

Bind the already provider-verified C04 artifact to the exact APK that will be handed to the physical Termux/Android preflight. This closes neither installation nor runtime; it removes an identity gap between provider evidence and a later device run.

## Authority

- repository: `rafaelmeloreisnovo/RafPolimata`
- provider workflow run: `31812435821`
- provider artifact: `9223681724`
- provider verified head: `8b46572c2fe3ad8610ca543ebdc2fd7b2b06ffea`
- structural merge: `3980c48a759dcd4ad469a7237df74e257e2a06e3`
- structural receipt commit: `e842820374f7eaa9c061536a1bdcb20581b74709`
- provider artifact ZIP SHA-256: `5b1666efbb9f7be69b6db900287e10aab4caea476ee95462c5797ca6d336aa94`
- expected member: `Apkc/proofs/out/hello.apk`
- exact APK SHA-256: `7957e4421921da85924cb278fce38db8d6ba747c99ecb9eb7e04edfbe42e9ff3`

## Materialized gate

`script/apkc_verify_provider_c04_transfer.sh` (repository path is `scripts/apkc_verify_provider_c04_transfer.sh`) enforces, fail-closed:

1. provider artifact exists and is non-empty;
2. artifact SHA-256 equals the frozen provider artifact digest before extraction;
3. expected APK member occurs exactly once;
4. extracted APK is non-empty;
5. extracted APK SHA-256 equals the frozen C04 APK digest;
6. only then may a physical preflight mode be invoked;
7. no pathname identity substitution is accepted as evidence;
8. receipt files preserve `claim_allowed=false`.

Git blob at materialization: `2b4402f5cb5f55285b1b5286a93dfb505fa34ce4`.
Local script content SHA-256 used for the real-artifact execution: `0f0566e43e84c8d8b328941f2823c0940775204f33102a0385589e39bf3e6f63`.

## Local execution against the real provider artifact

The artifact was downloaded from GitHub Actions and verified before testing.

Observed:

- artifact SHA-256: `5b1666efbb9f7be69b6db900287e10aab4caea476ee95462c5797ca6d336aa94` = expected;
- inner APK SHA-256: `7957e4421921da85924cb278fce38db8d6ba747c99ecb9eb7e04edfbe42e9ff3` = expected;
- `unzip -t` on the APK: no errors;
- dual ABI members observed: `lib/arm64-v8a/librafpolimata_c04.so` and `lib/armeabi-v7a/librafpolimata_c04.so`;
- positive extraction receipt verified with `sha256sum -c`;
- `runtime_result=TOKEN_VAZIO_NOT_ATTEMPTED` in extract-only mode.

Adversarial battery:

```text
PASS positive rc=0
PASS artifact_digest_mismatch rc=245
PASS missing_artifact rc=244
PASS unknown_mode rc=253
PASS physical_gate_missing rc=252
RESULT pass=5 total=5 claim_allowed=false
```

Transcript summary SHA-256: `9be6ff1240ea51be25a2b2151435c484f71b5fbf83801f5c31f8858aa57825ad`.

## Evidence boundary

This proves byte identity from the known provider artifact ZIP to the extracted APK candidate and fail-closed rejection of several transfer faults. It does **not** prove:

- current Termux ARM32 execution;
- memfd/seals on the actual device;
- `adb shell pm install -S N -` compatibility on the actual device;
- physical installation;
- launch;
- runtime semantics;
- cross-device determinism;
- TLS runtime certification.

Those remain `TOKEN_VAZIO` until physical evidence exists.

## F_ok

- provider artifact ZIP identity reverified;
- inner APK identity frozen and reproduced;
- exact C04 APK extracted from provider evidence;
- 5/5 transfer controls passed locally against the real artifact;
- receipt verification passed;
- structural scope and runtime scope remain separated.

## F_gap

- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`
- `TOKEN_VAZIO_TLS_RUNTIME_CERTIFICATION`

## F_next

Transfer the frozen provider artifact and this gate to the actual device. First execute `extract-only`, then `probe-only` with `APKC_ROOT` bound to the exact RafPolimata checkout. Only after a physical probe PASS execute `adb-shell-pm`. Preserve every negative exit and receipt; never fall back silently to an unbound pathname.
