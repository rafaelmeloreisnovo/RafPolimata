# Android Proof-Bundle Validator

Estado: `ACTIVE / EVIDENCE-GATED`

This validator consumes the existing `rafpolimata.android_proof_chain.v1` manifest plus same-run transcript files. It does **not** trust the manifest's `promotion` fields by themselves.

## Computed gates

```text
G1 source_to_binary
  immutable git commit + apkc SHA-256 + compile transcript

G2 binary_to_apk
  G1 + APK SHA-256 + generation transcript + ZIP listing containing AndroidManifest.xml and classes.dex

G3 arm64_real
  G2 + ARM64 status PASS + readelf transcript proving ELF64/AArch64

G4 android_runtime
  G3 + install PASS + launch PASS + logcat PASS + no fatal/dlopen failure flags

G5 single_run_reproducibility
  G4 + non-placeholder run timestamp
```

## Overall states

- `TOKEN_VAZIO`: G2 is not yet closed; absence is not invented as code failure.
- `PASS_LIMITED`: source→APK evidence is sufficient, but physical Android chain is incomplete.
- `PASS`: all five computed gates close in the same evidence bundle.
- `FAIL`: malformed evidence, explicit runtime failure, or a claimed PASS unsupported by same-run evidence.

Only overall `PASS` sets `claim_allowed=true` for this specific proof chain.

## Usage

```sh
python3 scripts/validate_android_proof_bundle.py \
  --manifest proofs/run-arm64-full-chain/out/manifest.json \
  --out-dir proofs/run-arm64-full-chain/out \
  --write-report proofs/run-arm64-full-chain/out/validation-report.json
```

## No-device boundary

CI/x86 may test the validator and can reach `PASS_LIMITED` on a real host evidence bundle. It must not synthesize installation, launch, logcat or ARM64 runtime evidence. Those remain `TOKEN_VAZIO` until supplied by the actual run.
