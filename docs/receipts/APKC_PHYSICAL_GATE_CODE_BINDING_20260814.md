# ApkC Physical Gate Code Binding — 2026-08-14

State: `MATERIALIZED / LOCAL_CONTROLLED_PASS / PROVIDER_EXECUTION_PENDING`

Claim gate: `claim_allowed=false`

## Purpose

Bind future physical ApkC evidence not only to the byte-exact C04 provider artifact and recursive evidence tree, but also to the exact bytes of the gate programs that produced the measurement.

Current main after merged PR #250 is `7acf3857f6c4cc1d65d1b415e92fe215cc4d8bfe`.

The existing chain already binds provider artifact -> exact APK -> physical/preflight evidence -> recursive receipt tree. The remaining custody ambiguity is that the physical receipt does not independently freeze the bytes of every transitive gate/helper used during that measurement.

## New additive entrypoint

`scripts/apkc_provider_to_physical_codebound.sh`

Git blob at materialization: `1ef6d145a0291c10b39a289f1932437799c74f61`.

It does not rewrite any previously provider-proven gate. Before invoking the merged provider-to-physical chain it creates `gate-code.sha256` over exactly nine required files:

1. `scripts/apkc_provider_to_physical_receipted.sh`
2. `scripts/apkc_verify_provider_c04_transfer.sh`
3. `scripts/apkc_termux_sealed_stdin_gate_receipted.sh`
4. `scripts/apkc_termux_sealed_stdin_gate.sh`
5. `scripts/apkc_install_sealed_stdin.sh`
6. `scripts/apkc_install_sealed_stdin.c`
7. `scripts/apkc_finalize_stdin_install_receipt.sh`
8. `scripts/apkc_probe_memfd_runtime.c`
9. `scripts/apkc_seal_receipt_tree.sh`

Each dependency must be a regular non-symlink file. The manifest is verified before execution. The pre-existing chain executes into a child `execution/` directory. After PASS or FAIL status is written, the complete root containing code manifest, verification result and child execution evidence is recursively sealed as `codebound-tree.sha256`.

A non-zero child exit is preserved after the negative evidence tree is sealed.

## Local controlled falsification

The materialized logic was exercised against isolated fixture roots and mock child chains:

`RESULT pass=7 fail=0 claim_allowed=false`

Controls:

1. positive code-bound chain;
2. gate-code tamper detected by `gate-code.sha256`;
3. nested execution-evidence tamper detected by parent tree;
4. missing gate dependency rejected fail-closed;
5. negative child exit and evidence both preserved;
6. dirty output directory rejected;
7. `claim_allowed=true` absent.

Test Git blob: `770b5d172cce7620152729f8baca0daf16df7a65`.

This is controlled local evidence, not physical Android evidence and not yet exact provider execution of the branch.

## Invariant

`same APK + same physical outputs` is not considered equivalent evidence unless the gate-code manifest is also bound or the difference is explicitly represented.

`PROVIDER_PASS != PHYSICAL_PASS`.

`CODEBOUND_PASS != ANDROID_RUNTIME_PASS`.

## TOKEN_VAZIO delta

Materialized, not yet provider-promoted:

- `TOKEN_VAZIO_PHYSICAL_GATE_CODE_IDENTITY_BINDING` -> `MATERIALIZED_CODEBOUND_CUSTODY_GATE`

Still open:

- `TOKEN_VAZIO_PHYSICAL_GATE_CODEBINDING_PROVIDER_EXECUTION`
- `TOKEN_VAZIO_TERMUX_ARM32_CURRENT_RUN`
- `TOKEN_VAZIO_ANDROID_MEMFD_CREATE_AND_SEALS_CURRENT_DEVICE`
- `TOKEN_VAZIO_ADB_SHELL_PM_STDIN_PHYSICAL_COMPATIBILITY`
- `TOKEN_VAZIO_PHYSICAL_INSTALL`
- `TOKEN_VAZIO_PHYSICAL_LAUNCH`
- `TOKEN_VAZIO_RUNTIME_SEMANTIC_PASS`
- `TOKEN_VAZIO_CROSS_DEVICE_DETERMINISM`

## F_ok

- PR #250 recursive provider-to-physical custody is merged in main.
- Gate-code identity gap was observed from current main receipts rather than inferred from historical memory.
- Additive wrapper preserves lower-level gate bytes and exits.
- Nine transitive gate/helper files are SHA-256 bound before execution.
- Code manifest plus execution tree are sealed together.
- Controlled adversarial battery: 7/7 PASS.
- `claim_allowed=false`.

## F_gap

Provider execution of this exact branch is pending. No physical Termux/Android capability, installation, launch or runtime behavior is claimed.

## F_next

Execute the exact head in the dedicated provider workflow. If 7/7 controls pass with real steps, reduce only the provider code-binding gap. The next physical operation remains the byte-exact C04 `probe-only` path, now preferably through the code-bound wrapper so the first device receipt freezes both APK identity and measurement-code identity.
