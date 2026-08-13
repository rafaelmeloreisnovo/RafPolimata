# ApkC Path Canonicality Gate Receipt — 2026-08-13

Status: VERIFIED_LIMITED_LOCAL_EXACT_BYTES
claim_allowed=false
runtime_semantic_pass=TOKEN_VAZIO
physical_termux_arm32=TOKEN_VAZIO

## Scope

This receipt records execution of the exact UTF-8 bytes fetched from the branch after commit `168b46e78cbc6d18af80d8ce5a74da7060137875` for:

- `scripts/apkc_path_canonicality_gate.sh`
- `tests/test_apkc_path_canonicality_gate.sh`

Exact-byte identity was checked by `git hash-object` against the GitHub content blob SHAs returned for those files.

## Git blob identity

- gate blob: `d5f6aabbb65dee5bd0969ea06c38d91785e81e6e`
- test blob: `5afa0c5a7d0397a38c8883cca6d00be608a29e6b`

Observed local `git hash-object` values matched both blob SHAs exactly.

## SHA-256

- gate: `5231c5d23ba793cb72160f72ffd81b517c926bdd0e4e3caea83a21609e508702`
- test: `95afbf7c596b3002084f41bd96d23d9fac2c7a21569f744b8e741b9807bdbf64`
- transcript: `a78758ec73beab8ebe2215c5fedf6c7568cc3fd58125c3127d008cc62de91ec2`

## Execution

`bash -n` passed for gate and test.

Observed transcript:

```text
PASS canonical_relative
PASS dot_segment_rejected
PASS backslash_alias_rejected
PASS traversal_rejected
PASS legacy_absolute_canonical
PASS legacy_absolute_backslash_rejected
RESULT pass=6 fail=0 claim_allowed=false
exit_code=0
```

## Evidence boundary

This proves the exact fetched bytes of the new additive path-canonicality gate and its adversarial test behaved as recorded in the available local environment. It is not evidence of Android/Termux/ARM32 execution, signing, install, launch, runtime semantics, physical relocation, or cross-device determinism.

The direct producer/verifier internal backslash fix is still not landed because the attempted replacement of existing files was blocked before reaching GitHub. The additive pre-gate is therefore a mitigation, not yet the final internal consolidation.

## F_ok

- backslash canonicality gap identified from producer/verifier source inspection;
- additive fail-closed pre-gate landed;
- six-case adversarial test landed;
- exact fetched bytes matched GitHub blob identities;
- exact-byte local execution: 6/6 PASS, exit 0;
- claim_allowed=false preserved.

## F_gap

- internal producer rejection of backslash: TOKEN_VAZIO_NOT_LANDED;
- internal independent-verifier rejection of backslash: TOKEN_VAZIO_NOT_LANDED;
- bridge integration of pre-gate before producer/verifier: TOKEN_VAZIO_NOT_LANDED;
- Termux/ARM32 physical execution: TOKEN_VAZIO;
- runtime semantic PASS: TOKEN_VAZIO;
- cross-device determinism: TOKEN_VAZIO.

## F_next

Priority 1: integrate this pre-gate into the hardened Android bridge before receipt production and independent verification, fail-closed on any nonzero result.

Priority 2: when update access permits, duplicate the same backslash rejection internally in producer and verifier, then keep the pre-gate as defense-in-depth or retire it only with an explicit compatibility/migration proof.

Priority 3: execute the exact branch on Termux/ARM32 and preserve commit SHA, Git blob identities, SHA-256, transcript, exit code, and resulting receipt.
