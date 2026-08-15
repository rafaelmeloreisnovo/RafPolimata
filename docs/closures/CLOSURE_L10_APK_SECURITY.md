# L10 — APK Security / Signature Verification Closure

**Date:** 2026-08-15  
**Status:** `CURRENT_ARTIFACT_TOKEN_VAZIO`  
**Historical evidence:** `AVAILABLE`  
**claim_allowed:** `false` for current-commit APK signing until re-executed

## Scope

This closure is intentionally narrow: **APK signature verification and signing provenance**. It does not claim a complete mobile application security audit.

## Historical evidence preserved

`Apkc/proofs/out/apksigner-verify.txt` records a prior proof dated 2026-06-14 with:

- signed APK SHA-256 `063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28`;
- v1 JAR signing = true;
- APK Signature Scheme v2 = true;
- APK Signature Scheme v3 = true;
- v3.1 = false;
- v4 = false;
- one signer;
- certificate SHA-256 `32fb9b6af9112b60a1cd95f1543ab4a6fc02e4ba8ce72c0f9d730ae8734e7992`;
- RSA 2048 key.

That evidence is valid as historical provenance. It is **not** silently inherited by a newly generated APK or a different commit.

## Correct current gate

`tools/verify_all_gaps.sh L10` now requires:

```sh
RAF_APK_UNDER_TEST=/path/to/current.apk bash tools/verify_all_gaps.sh L10
```

The gate must locate `apksigner` and execute:

```text
apksigner verify --verbose --print-certs <APK>
```

Result semantics:

```text
PASS        = apksigner accepted the exact supplied APK
FAIL        = apksigner rejected the exact supplied APK
TOKEN_VAZIO = apksigner or current APK artifact is unavailable
```

`zipalign` is not accepted as signature evidence. `jarsigner` availability by itself is not accepted as verification of Android v2/v3 schemes.

## Integration with physical chain

`scripts/capture_android_proof_chain.sh` already supports signing/verifying during the canonical Android proof run. For a current Phase D run, the preferred custody path is:

```text
runtime-hardened ApkC
→ generated APK
→ apksigner sign/verify when keystore is supplied
→ install
→ launch
→ logcat
→ manifest/status receipt
```

If `APKSIGNER_KEYSTORE` is not supplied, signature state remains audit/TOKEN_VAZIO as appropriate; the script must not fabricate signing custody.

## Security work not covered by this closure

The following remain separate future gates:

- production-key custody/HSM policy;
- certificate rotation and compromise response;
- manifest exported-component review;
- permissions minimization;
- debuggable/testOnly flags;
- Network Security Config;
- native hardening (RELRO/NX/PIE/canaries where applicable);
- dependency/SBOM vulnerability review;
- Play Integrity or equivalent deployment controls.

These are not implied by a successful `apksigner verify`.

## Closure verdict

```text
HISTORICAL_APKSIGNER_V1_V2_V3 = EVIDENCE_AVAILABLE_2026_06_14
CURRENT_COMMIT_SIGNED_APK      = TOKEN_VAZIO_ARTIFACT
CURRENT_APKSIGNER_VERIFICATION = TOKEN_VAZIO_EXECUTION
FULL_APK_SECURITY_AUDIT        = TOKEN_VAZIO_OUT_OF_SCOPE
```

The next honest promotion requires a freshly generated APK tied to the current commit and an observed `apksigner verify` receipt.
