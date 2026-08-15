# ApkC Stage 11 Integrity Truth — Provider Receipt — 2026-08-15

## Boundary

`claim_allowed=false`

This receipt closes only the Stage 11.4 truth/fail-closed boundary for the built-in integrity substitute. It does **not** claim SHA-256, MAC, signature, authenticity, Android physical runtime, or cross-device determinism.

## Provenance

- repository: `rafaelmeloreisnovo/RafPolimata`
- base main: `c7ccbb6f89b99ec86d76b2b7c755761b4744ca5b`
- tested code head: `e04814502befa6b09f55d4d2cf4f9058fc11c9e2`
- PR: `#274` draft
- workflow: `ApkC Stage11 Integrity Truth`
- provider run: `31867253011`
- provider job: `94970185378`
- conclusion: `success`

## Before → After

```text
BEFORE
- simple 64-bit multiplicative hash described as cryptographic / SHA256-like marker
- harden_verify_seal(unsealed_context, state) -> GATE_PASS
- harden_seal_state() implicitly increments replay counter

AFTER
- capability explicitly INTEGRITY_SUBSTITUTE_ONLY
- HARDEN_INTEGRITY_CRYPTOGRAPHIC=0
- harden_verify_seal(unsealed_context, state) -> GATE_REJECT
- bootstrap allowed only by composite checkpoint on pristine context
- replay counter is owned by the monotonic gate and equals caller-observed counter
- claim_allowed=false
```

## Exact provider falsifiers

```text
PASS noncrypto_capability
PASS unsealed_verify_rejects
PASS bootstrap_checkpoint_passes
PASS counter_exact_after_bootstrap
PASS sealed_verify_passes
PASS tamper_rejects
PASS nonpristine_zero_seal_rejects
PASS bootstrap_counter_replay_rejects
RESULT pass=8 fail=0 claim_allowed=false
```

Provider steps `Compile strict`, `Run falsifiers`, `Hash evidence`, and `Upload evidence` all completed `success` on a real Ubuntu hosted runner.

## Evidence hashes

```text
16207e8274e87a0df84fd69d9fc1e570018ec76e54f97ec97fc1c6192ec6c7f3  Apkc/sec_hardening_gates.h
e778d41a15a3686178ccc446cdc20c8349d3452d5d544debdda9824482623932  tests/test_stage11_integrity_truth.c
7f9dac3cd87dd58bbaa699aa2b1815d3689e0d1dcafa6e0ac6dd6b3e77f7df38  stage11-integrity-truth.txt
```

Artifact:

```text
id=9242379528
bytes=684
sha256=2fe695f0549e9de8ce47779e86e424fbd519eb363cb8e973c664a2e636fae3b0
```

## Negative evidence preserved

Runs `31867180832` and `31867216710` compiled and executed the 8 falsifiers successfully but failed in the receipt hashing step. The second negative run uploaded artifact `9242370266`; its transcript contains 8/8 PASS while its hash manifest is empty. This was classified as receipt-tooling failure, not code failure, and corrected inside the same PR rather than stacked into a new PR.

## Remaining gaps

```text
TOKEN_VAZIO_CRYPTOGRAPHIC_INTEGRITY_PRIMITIVE
TOKEN_VAZIO_AUTHENTICATED_SEAL_MAC_OR_SIGNATURE
TOKEN_VAZIO_STAGE11_ANDROID_PHYSICAL_EXECUTION
TOKEN_VAZIO_STAGE11_CROSS_DEVICE_DETERMINISM
```

No merge, release, approval, or default-branch write was performed by this hardening cycle.
