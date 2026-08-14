# ApkC — Recursive Receipt Tree Hardening — 2026-08-14

State: `MATERIALIZED_PENDING_PROVIDER_EXECUTION`

Claim gate: `claim_allowed=false`

## Observed gap

The canonical physical and provider-transfer gates create SHA-256 receipts with `find ... -maxdepth 1`. Nested installer evidence is stored below directories such as `install-chain/`, so the top-level receipt does not itself bind every nested evidence byte.

This is a custody coverage gap, not evidence that any existing nested file was altered.

## Delta

- `scripts/apkc_seal_receipt_tree.sh`: deterministic recursive SHA-256 manifest over regular files; rejects symlinks fail-closed; manifest and verification outputs are excluded from their own digest set.
- `scripts/apkc_termux_sealed_stdin_gate_receipted.sh`: compatibility wrapper around the existing physical gate. The existing gate bytes are not changed. Only a physical-gate exit 0 can proceed to recursive sealing.
- `tests/test_apkc_recursive_receipt_tree.sh`: nested binding, nested tamper, symlink, deterministic manifest and empty-tree controls.
- `tests/test_apkc_recursive_receipted_gate.sh`: nested installer binding, post-seal tamper, physical-gate exit propagation, symlink rejection and claim gate.

## Invariants

`legacy_gate != recursive_wrapper`; compatibility is preserved.

`gate_exit=0` is necessary but not sufficient: recursive receipt sealing and independent verification must also pass.

`symlink_present -> FAIL` to avoid binding a path whose target may escape or change outside the evidence tree.

`claim_allowed=false` remains mandatory.

## Evidence boundary

The source and falsifiers are provider-materialized on branch `hardening/apkc-recursive-receipt-tree-20260814`. No provider or physical execution of this exact branch is claimed by this receipt yet.

## F_ok

Gap identified from current main; deterministic recursive sealer, compositional wrapper and adversarial tests materialized without modifying the existing provider-proven gate.

## F_gap

- `TOKEN_VAZIO_RECURSIVE_RECEIPT_TREE_PROVIDER_EXECUTION`
- `TOKEN_VAZIO_RECURSIVE_RECEIPT_TREE_PHYSICAL_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`

## F_next

Execute both new test scripts on a provider runner. Only after they pass, use the receipted wrapper for the physical `probe-only` route and preserve the complete recursive manifest before any install attempt.
