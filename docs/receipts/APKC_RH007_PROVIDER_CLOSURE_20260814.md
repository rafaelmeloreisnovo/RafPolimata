# ApkC RH-007 Provider Closure — 2026-08-14

State: `VERIFIED_LIMITED_PROVIDER / claim_allowed=false`

## Scope

This receipt records only the `APKC-RH-007` runtime-source hardening anchor correction on PR #239. It does not promote source-cap, browser/TLS, Android, installation, launch, or runtime claims.

## Change

Historical RH-007 matched the full D8 success/failure block. The current source still contains the D8 success condition and characteristic comment, but the larger historical byte anchor had drifted. The correction narrows RH-007 to the unique condition+comment anchor while preserving `replace_once` and its exact-one-occurrence fail-closed rule.

The transformation remains:

`if (dexout)` → `if (dexout && _apkc_is_dex(_fork_out,dexout))`

The existing body, `else`, and refusal path are not replaced by RH-007.

A new adversarial unit test mutates the minimal RH-007 anchor and requires `ValueError: APKC-RH-007`.

## Provider execution

Head: `de9d676059d4af7a4eae3555c2829b0c5773fdd0`

Workflow run: `31796297945`

Job: `94754046179`

Observed on hosted Ubuntu 24.04 runner:

- syntax/contracts setup: success;
- direct unit suite no longer reports either prior RH-007 `ValueError`;
- `scripts/patch_apkc_runtime_source.py` emitted `PASS changes=13`;
- runtime-hardening proof invoked the transformer twice and both invocations emitted identical output SHA-256 `650c559d20888d580fc4168785ecd04b5add084d8f884a9daecf0e820fe2b1b0`;
- custody shell families remained `8/8 + 6/6 + 7/7 + 4/4 + 9/9 = 34/34 PASS` with `claim_allowed=false`;
- Android compatibility remained `TOKEN_VAZIO`.

Provider artifact:

- artifact ID: `9217514876`
- bytes: `145522`
- SHA-256: `3df4da333a0ca914bec7fde7e8b87d1503e5e095816558a9c4550b812efc727f`
- expires: `2026-09-13T11:28:51Z`

## Remaining independent blockers

The workflow still finished `failure`. Provider logs show these roots remain open and must not be collapsed into RH-007:

1. `PROOF-FAIL-CLOSED`: first-part gate still expects historical `source-to-binary-proof.v2` while the current proof family declares v3;
2. source-to-binary proof: `source-cap hardening failed`;
3. browser/TLS: `https_transport_adapter_static_evidence=false`.

`GENERATE_APK`, `FIRST_PART`, and structural C04 failure remain downstream consequences while source proof / code-gate truth is incomplete.

## TOKEN_VAZIO delta

- `TOKEN_VAZIO_RUNTIME_PATCH_CURRENT_SOURCE_RECONCILIATION` → `RESOLVED_RH007_PROVIDER_EXECUTED`
- `TOKEN_VAZIO_PROOF_SCHEMA_V3_GATE_RECONCILIATION` → remains open
- `TOKEN_VAZIO_SOURCE_CAP_CURRENT_SOURCE_RECONCILIATION` → remains open
- `TOKEN_VAZIO_BROWSER_TLS_STATIC_EVIDENCE_RECONCILIATION` → remains open
- physical Android/Termux execution → remains `TOKEN_VAZIO`

## R3

`F_ok`: RH-007 exact-one anchor repaired; deterministic transform observed twice with identical SHA-256; adversarial anchor drift retained; provider evidence archived.

`F_gap`: proof-v3 gate, source-cap transform, browser/TLS static evidence, physical Android/Termux.

`F_next`: reconcile `PROOF-FAIL-CLOSED` v2→v3 by validating the current proof contract rather than accepting a legacy marker; then inspect the exact source-cap hardening transcript/anchor before modifying source-cap logic.
