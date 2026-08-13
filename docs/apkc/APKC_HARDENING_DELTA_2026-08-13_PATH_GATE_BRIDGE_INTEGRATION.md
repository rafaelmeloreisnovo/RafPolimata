# ApkC/RAFAELIA hardening delta — PathGate bridge integration — 2026-08-13

## Invariants

- `claim_allowed=false` unless stronger evidence is independently demonstrated.
- Implementation != execution != physical Android evidence != runtime semantic claim.
- `TOKEN_VAZIO` remains valid for unmeasured states.
- Fail-closed: a non-canonical install target must not reach `adb install` / `pm install`.
- Preserve compatibility: explicit `DO_SIGN=0` remains allowed without a signing claim.
- Preserve provenance and append-only custody records.

## F_ok

- Integrated `scripts/apkc_path_canonicality_gate.sh` into `scripts/capture_android_proof_chain_hardened.sh` after signed-target selection and before any install command.
- Missing PathGate is fail-closed (`exit 100`).
- PathGate rejection withholds the install target by rewriting `install-target.txt` to `TOKEN_VAZIO`, records `path_canonicality_gate FAIL`, and exits non-zero (`exit 101`).
- PathGate success is recorded in `status.tsv` before install can proceed.
- Added adversarial bridge fixture using an `OUT_DIR` containing a textual `./` alias. This case demonstrates why expected-target equality alone is insufficient: the expected path can itself be non-canonical.
- The adversarial fixture requires `TOKEN_VAZIO`, non-zero exit, a FAIL status, and absence of install evidence.

## Evidence boundary

- Source integration: LANDED.
- Adversarial test source: LANDED.
- GitHub Actions for head `6ecfb3e62cc0ab5a1a05bcdc66d76e443b396ec6`: workflow created, but ApkC job had `steps=[]`, `runner_id=0`, empty runner name; therefore `RUNNER_STARTUP_FAIL`, not `CODE_FAIL`.
- Head-exact test execution: `TOKEN_VAZIO`.
- Current Termux/ARM32 physical execution: `TOKEN_VAZIO`.
- Real signing certificate: `TOKEN_VAZIO`.
- Physical install/launch: `TOKEN_VAZIO`.
- Runtime semantic PASS: `TOKEN_VAZIO`.
- Cross-device determinism: `TOKEN_VAZIO`.

## F_gap

1. No observable CI step executed the new bridge test.
2. No current physical Termux/ARM32 run of the integrated chain.
3. The PathGate remains defense-in-depth; producer/verifier internal canonicality must continue to be checked independently.
4. Runtime semantic health remains intentionally unclaimed.

## F_next

Priority: obtain byte-exact execution evidence for the landed bridge integration before adding new claims.

Minimum next gate:

1. checkout exact head;
2. hash bridge, PathGate, sign gate and test bytes;
3. run `bash tests/test_apkc_hardened_capture_bridge.sh`;
4. preserve transcript SHA-256 + exit code;
5. on Android/Termux, execute sign -> PathGate -> install -> launch -> logcat -> final receipt;
6. independently verify the final receipt;
7. keep `runtime_semantic_pass=TOKEN_VAZIO` until a separate semantic runtime gate exists.

`R_3 = <F_ok: PathGate required pre-install, F_gap: exact/physical execution, F_next: exact-byte bridge test then ARM32 chain>`
