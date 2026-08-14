# ApkC / RAFAELIA — Provider C04 Structural Closure — 2026-08-14

State: `PASS_STRUCTURAL / PROVIDER_EXECUTED / claim_allowed=false`

## Authority

- Repository: `rafaelmeloreisnovo/RafPolimata`
- PR: `#244`
- Verified PR head: `8b46572c2fe3ad8610ca543ebdc2fd7b2b06ffea`
- Merge commit: `3980c48a759dcd4ad469a7237df74e257e2a06e3`
- Provider workflow: `ApkC First Part Closure`
- Workflow run: `31812435821`
- Job: `94806032610`
- Hosted runner: Ubuntu 24.04.4 / runner 2.336.0

## Provider-executed gates

All final-enforce outcomes were `success` on the exact verified head:

- tooling
- syntax/contracts
- Python tests
- custody hardening
- source-to-binary proof
- dual-ABI APK generation
- runtime hardening
- first-part gate
- browser/TLS contracts
- structural receipt

Observed details:

- Python suite: `42/42 PASS`
- custody hardening: `8/8 + 6/6 + 7/7 + 4/4 + 9/9 = 34/34 PASS`
- `android_compatibility=TOKEN_VAZIO`
- source-to-binary proof: `PASS`
- strict dual-ABI generation used `Apkc/hello.dualabi.structural.s.txt`
- `--allow-undef` was not used
- verified APK generation: `PASS`
- runtime source hardening: `13` deterministic changes; SHA-256 `13844d70544e1734910ebf387d9a604ae186483ffbe8498eceeeef5f6d142654`
- runtime hardening proof: AArch64 SHA-256 `46e05b9a5f1a20aa33f6f082df00e600ad820b406ed9f082896cac0718288339`; ARM32 SHA-256 `0daaf28686af6318227e42f3b537e2b28ce515ef77a3672d871b3ee55bf6ca07`
- first-part gate: `8/8`, contradictions=`0`, state=`PASS`
- C04 receipt: `PASS_STRUCTURAL`

## Artifact custody

Provider artifact:

- ID: `9223681724`
- bytes: `567582`
- ZIP SHA-256: `5b1666efbb9f7be69b6db900287e10aab4caea476ee95462c5797ca6d336aa94`

The artifact includes the generated APK, source-proof outputs, runtime-hardening evidence, first-part gate, TLS audit, format validation, custody transcript and C04 structural closure receipt.

## Scope boundary

The C04 receipt explicitly enforces:

- `structural_claim_allowed=true`
- global `claim_allowed=false`
- `runtime_boundary.apk_installed=TOKEN_VAZIO`
- `runtime_boundary.package_launched=TOKEN_VAZIO`

Therefore:

`dual-ABI structural closure != physical Android install != package launch != runtime semantic proof`.

TLS static policy evidence also does not equal certified TLS runtime behavior.

## F_ok

- ancestry-conflicted predecessor was superseded by a clean reconstruction on current main;
- TLS static audit reconciled with negative controls;
- ABI-neutral structural fixture generated both payloads under strict mode;
- DEX `map_off` field corrected to offset `0x34` and adversarially guarded;
- exact head executed by provider;
- First Part, primary CI and Formal Science Orchestrator all succeeded;
- C04 scoped structural closure landed in `main`.

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

Prioritize the existing fail-closed physical preflight on the actual Termux/ARM32 device using the exact APK/SHA pair. First run probe-only. Only after a physical probe PASS, execute the sealed-stdin `adb shell pm install -S N -` path, preserving stdout/stderr, exit code, hashes, device pseudonym and receipt. Do not fall back silently to pathname transport.
