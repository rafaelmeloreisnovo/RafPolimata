# ApkC raw-source anti-bypass hardening receipt — 2026-08-12

Status: VERIFIED_LIMITED_STATIC
claim_allowed: false
branch: `audit/apkc-raw-source-gate-20260812`
PR: #220
base: `3f2873b6c8cf56a4a8e116eab40523af03354e92`
head_at_receipt: `7a5cbcd0fd12c375ddc8bf857b1ec751f1dea908`

## F_ok

1. Prior source-cap hardening remains present in validator, language coverage, API/ABI matrix, canonical proof and Termux hermetic routes.
2. A concrete remaining bypass was identified in `.github/workflows/ci.yml`: step `APKc compile transcript — F1 chain of custody` still invoked `clang ... -fsyntax-only Apkc/apkc.c` directly.
3. The CI F1 step was changed to execute `make apkc-hardened-source`, compile only `build/generated/Apkc/apkc.source-cap-hardened.c`, and assert both the overflow guard and absence of the legacy vulnerable anchor.
4. Added `scripts/audit_apkc_raw_source_paths.py`, a fail-closed static gate over executable/control-plane files (`.sh`, `.bash`, `.py`, `.yml`, `.yaml`, `Makefile`). A raw `Apkc/apkc.c` reference is accepted only when paired with a recognized hardening entrypoint.
5. Added `tests/test_apkc_raw_source_gate.py` with adversarial fixtures covering direct compiler bypass, hardened source-cap route, runtime-hardening route, workflow YAML bypass, hardened workflow route and documentation non-execution.
6. Local isolated execution of the classifier fixture test returned `PASS raw-source gate classification` before repository write.

## F_gap

- Repository-wide execution of the new gate on GitHub remains `TOKEN_VAZIO_RUNNER`.
- GitHub Actions CI run `31600272538` for head `7a5cbcd0fd12c375ddc8bf857b1ec751f1dea908` concluded `failure`, but its only job had `steps: []`, `runner_id: 0`, and an empty runner name. This is classified as infrastructure/runner non-start, not code failure.
- `Apkc/apkc.c` itself still contains the legacy source-read anchor. Direct canonical-source hardening is therefore not yet closed.
- Physical ARM boundary execution, install, runtime logcat and end-to-end APK provenance remain `TOKEN_VAZIO`.
- No runtime/install/end-to-end claim is promoted.

## F_next

Priority correction after this branch:

1. obtain one runner that starts steps and execute `python3 tests/test_apkc_raw_source_gate.py` plus `python3 scripts/audit_apkc_raw_source_paths.py`;
2. only if repository-wide gate is PASS, merge PR #220;
3. make `patch_apkc_source_cap.py` idempotent for an already-hardened canonical source, with falsifiers for vulnerable / already-hardened / ambiguous states;
4. patch `Apkc/apkc.c` directly so the canonical source is fail-closed without a transform dependency;
5. execute boundary triad on ARM/Termux and bind raw/canonical source SHA -> compiler ELF SHA -> APK SHA -> sign/install/runtime receipts.

## Contract

`VISÃO != ARTEFATO != EXECUÇÃO != EVIDÊNCIA != CLAIM` remains enforced.
`TOKEN_VAZIO` is preserved where execution evidence is absent.
No workflow conclusion is interpreted as proof when no job step actually ran.
