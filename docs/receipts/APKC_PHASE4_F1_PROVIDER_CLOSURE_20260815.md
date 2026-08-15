# ApkC Phase 4 — F1 Provider Closure — 2026-08-15

State: `PROVIDER_F1_STRICT_COMPILE_PASS / INDEPENDENT_GAPS_OPEN`

Claim gate: `claim_allowed=false`

## Exact provider execution

- PR: `#258`
- tested head: `58326dc10587897f33ac30f4606c29965ab08413`
- CI run: `31852537048` — `success`
- CI job: `94930864139` — `success`
- ApkC First Part Closure: `31852537153` — `success`
- Formal Science Orchestrator: `31852537218` — `success`
- Document Governance: `31852536979` — `success`

The provider checked out the PR merge ref whose second parent is the exact hardening head above.

## F1 closure evidence

The exact F1 chain executed:

1. source-cap transformer test — PASS;
2. source-cap patch — PASS;
3. exact source-cap verifier — PASS;
4. `clang -std=c11 -O2 -Wall -Wextra -Werror -target aarch64-linux-gnu -nostdlib -nostdinc -ffreestanding -I Apkc -fsyntax-only ...` — PASS;
5. compile transcript: `0 lines`.

Observed hardened-source SHA-256:

`5f65f0ace8e09730b02e8ceac688ce1ad55644aef2fe0b86e80e1fc6300fac84`

Therefore:

`TOKEN_VAZIO_PR258_PROVIDER_EXECUTION -> RESOLVED_PROVIDER_EXECUTION`

`TOKEN_VAZIO_PHASE4_F1_STRICT_COMPILE -> RESOLVED_PROVIDER_WERROR_PASS`

No warning suppression was introduced.

## Provider artifacts

- `apkc-proofs-and-hotfix-31852537048`
  - artifact ID: `9237952660`
  - bytes: `76346`
  - SHA-256: `05fc3cc8b0fd735f1b273fc9dd3d744563b15d5c455405fcd021c086010b1eae`
- `apkc-proof-runs`
  - artifact ID: `9237952804`
  - bytes: `6560`
  - SHA-256: `01f0b51b9f756740022a7182211dcfd607f56c81d1aaa71d3402f57750784dae`

## Independent fail-closed gap confirmed by the same successful CI

The `APKc raw-source anti-bypass gate` step remained green even though the audit printed:

- schema: `raf.apkc.raw-source-gate.v1`
- state: `FAIL`
- failures: `7`
- claim_allowed: `false`

The auditor itself already returns exit `1` when failures exist. The workflow invokes it through:

`python3 scripts/audit_apkc_raw_source_paths.py | tee Apkc/proofs/out/raw-source-gate.json`

without `pipefail`, so `tee` masks the auditor's non-zero status.

This is tracked as:

`TOKEN_VAZIO_RAW_SOURCE_AUDIT_FAIL_CLOSED_ENFORCEMENT`.

The seven reported bypass rows must be reviewed separately; turning on `pipefail` alone would correctly make CI red but would not resolve those seven underlying rows.

## ABI boundary remains open

F1 syntax/strict compilation is not function ABI conformance. Stage 4.10 still has unimplemented stack argument emission and unproven prologue/epilogue semantics.

Remain open:

- `TOKEN_VAZIO_FUNCTION_ABI_CONFORMANCE`
- `TOKEN_VAZIO_FUNCTION_STACK_ARGUMENT_EMISSION`
- `TOKEN_VAZIO_FUNCTION_PROLOGUE_EPILOGUE_SEMANTICS`

No physical Android or Termux execution is inferred from provider Linux execution.

## F_ok

Provider strict F1 compiler gate is green with exact source-cap verification and zero diagnostics; four observed workflows are green; proof artifacts are SHA-bound; no warning suppression; claim gate remains false.

## F_gap

Raw-source audit is not fail-closed at workflow pipeline level and reports seven underlying bypass rows. Function ABI behavior remains unproven. Android/Termux physical execution remains `TOKEN_VAZIO`.

## F_next

1. classify the seven raw-source rows into true bypass versus audit-policy false positive;
2. repair each true bypass or narrow the audit policy with explicit falsifiers;
3. only then add `set -o pipefail` (or equivalent explicit status capture) so audit `state=FAIL` fails CI;
4. independently add executable function ABI tests before repairing/promoting stack arguments and prologue/epilogue.
