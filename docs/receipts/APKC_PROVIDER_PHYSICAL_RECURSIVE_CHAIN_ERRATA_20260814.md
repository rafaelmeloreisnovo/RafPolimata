# ApkC Provider → Physical Recursive Chain — Provider Failure Errata — 2026-08-14

State: `NEGATIVE_EVIDENCE_PRESERVED / CORRECTION_MATERIALIZED / claim_allowed=false`

## First provider execution

Workflow: `ApkC Provider Physical Recursive Chain`

Run: `31840645187`

Job: `94896543050`

Observed real steps:

- checkout: PASS
- bash syntax: PASS
- falsifier step: FAIL
- evidence upload: PASS

Observed test summary:

`RESULT pass=4 fail=3 claim_allowed=false`

Passing controls:

- positive recursive binding;
- nested-tree substitution detected;
- negative evidence sealed with original exit preserved;
- claim gate preserved.

Failing controls:

- dirty output rejection;
- missing transfer gate expected exit;
- unknown mode expected exit.

Evidence artifact:

- artifact id: `9234098108`
- bytes: `369`
- provider-reported SHA-256: `fc6de2c936e2e08adbb4ffb51cf214521863aa85b5590ed9b89045479a6aba84`

## Root causes

1. Dirty-output detection used `find | grep -q` under `set -o pipefail`. Early grep termination can SIGPIPE `find`, making the conditional pipeline non-zero even when an entry exists.
2. New wrapper exit codes were `291..298`. POSIX process exit status is represented in 8 bits, so those values are truncated modulo 256 and cannot be an exact external contract.
3. The workflow expected `pass=6`, but the test actually contains seven controls including the explicit claim-gate check.

These are defects in the new wrapper/test contract only. No pre-existing C04 transfer, physical gate, sealed-stdin mechanism, or recursive sealer failure was observed.

## Corrective delta

- dirty-output check changed to a command-substitution emptiness test with no SIGPIPE-prone pipeline;
- wrapper-specific exits moved to `231..238`;
- tests aligned to the exact exit contract;
- provider workflow now requires `RESULT pass=7 fail=0 claim_allowed=false`.

## Evidence boundary

The correction is not promoted until a new provider execution of the corrected head completes with real steps and all seven controls PASS.

`TOKEN_VAZIO_PROVIDER_PHYSICAL_RECURSIVE_CHAIN_EXECUTION` remains open.

Physical Android/Termux execution remains `TOKEN_VAZIO`.
