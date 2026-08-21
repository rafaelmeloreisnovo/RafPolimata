# Federated Doctor Pass Report

**Report ID**: federated-doctor-20260821T034515Z
**Generated**: 2026-08-21T03:45:15.765854Z
**Schema**: raf.federated-doctor-report.v1
**Report Hash**: 038f9a68f186e95138a2e6f8b531308f2b4bc5ed669b61e0fdefd5a8a7dde4e7

## Registry Reference

- **Path**: /home/user/mapa/data/control-plane/module_registry.v1.json
- **Hash**: 66a4599724edd7538691a6937d32eac0588f657fd0499462ad9fc6e028a80baa

## Module Inventory

| Metric | Count |
|--------|-------|
| Total observed | 7 |
| Verified | 3 |
| Unverified | 4 |
| Errors | 0 |
| TOKEN_VAZIO preserved | 4 |

## Module Evidence

### MOD-MAPA-CONTROL

- **Repository**: rafaelmeloreisnovo/Mapa
- **Branch**: main
- **Ref**: eb9cb679d42f...
- **State**: VERIFIED_LIMITED
- **Validation**: VERIFICADO_LIMITADO
- **Capabilities**: inventory, ontology...
- **Gaps**: 2 gaps

### MOD-RAF-CONTROL

- **Repository**: rafaelmeloreisnovo/RafGitTools
- **Branch**: feat/termux-health-loopback-v1
- **Ref**: 24c3bf6f0bb9...
- **State**: PARTIAL_DRAFT
- **Validation**: TOKEN_VAZIO
- **Capabilities**: git.status, git.diff...
- **Gaps**: 2 gaps

### MOD-TERMUX-RUNTIME

- **Repository**: rafaelmeloreisnovo/termux-app-rafacodephi
- **Branch**: feat/readonly-health-server-v1
- **Ref**: d62153f377c9...
- **State**: PARTIAL_DRAFT
- **Validation**: TOKEN_VAZIO
- **Capabilities**: health.readonly
- **Gaps**: 2 gaps

### MOD-RAFPOLIMATA-EVIDENCE

- **Repository**: rafaelmeloreisnovo/RafPolimata
- **Branch**: main
- **Ref**: 5a0551eb7384...
- **State**: VERIFIED_LIMITED
- **Validation**: VERIFICADO_LIMITADO
- **Capabilities**: bounded_conversation_index, ecosystem_build_doctor...
- **Gaps**: 1 gaps

### MOD-VECTRAS-RUNTIME

- **Repository**: rafaelmeloreisnovo/Vectras-VM-Android
- **Branch**: main
- **Ref**: 4c461538e1e6...
- **State**: VERIFIED_LIMITED
- **Validation**: VERIFICADO_LIMITADO
- **Capabilities**: vm_runtime, cmake_language_contract...
- **Gaps**: 1 gaps

### MOD-LLAMA-INTERPRET

- **Repository**: rafaelmeloreisnovo/llamaRafaelia
- **Branch**: master
- **Ref**: 4800636abb6b...
- **State**: PARTIAL
- **Validation**: TOKEN_VAZIO
- **Capabilities**: context_retrieval, rmrcti_structural_features
- **Gaps**: 1 gaps

### MOD-TERMUX-INTEGRATION

- **Repository**: rafaelmeloreisnovo/termux-app-rafacodephi
- **Branch**: claude/proximo-passo-natural-c15mg3
- **Ref**: 5e83eef9aea8...
- **State**: PARTIAL_DRAFT
- **Validation**: TOKEN_VAZIO
- **Capabilities**: Android runtime execution environment, native code integration (JNI/NDK)...
- **Gaps**: 5 gaps


## Aggregated Findings

- **module_inventory_complete**: VERIFICADO_LIMITADO
- **gap_preservation**: FATO

## F_ok (Validated)

- Mapa module registry loaded and parsed successfully
- All 6 registered modules observed
- Evidence collection completed for each module
- State transitions validated (VERIFIED_LIMITED, PARTIAL_DRAFT, etc)
- TOKEN_VAZIO gaps preserved and not fabricated
- Cross-repository topology visible and traced

## F_gap (Open)

- Device evidence for termux-app not yet collected
- Validation of independence between repositories pending
- Live synchronization state not verified
- Implementations for partial trajectories remain TOKEN_VAZIO
- Physical runtime evidence on Android device absent

## F_next (Action Items)

- Execute next_gates for BIBLIOTECONOMIA trajectory
- Map ONTOLOGY_CATALOG records to repository paths
- Implement deterministic bootstrap fixture for STATISTICS
- Create blinded benchmark for SCIENTIFIC_INFERENCE
- Collect device evidence from termux-app runtime
