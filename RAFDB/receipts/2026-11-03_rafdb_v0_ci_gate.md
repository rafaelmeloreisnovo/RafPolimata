# RAFDB V0 — CI gate receipt — 2026-11-03

receipt_id: RAFDB-V0-CI-20261103-01
source_commit: 17346c9e0182a716a11d0b362b644b954f4592a1
parent_code_commit: 7f0288720ce5e8730063e53c84077fdafe1b4489
branch: feat/rafdb-zipraf-16xor-v1-20261103
pull_request: https://github.com/rafaelmeloreisnovo/RafPolimata/pull/449
producer_scope: RAFDB/banco/, RAFDB/gateway/, RAFDB/tests/, .github/workflows/rafdb-contract.yml

## Observed gates (from GitHub runs for source_commit)

- RAFDB bounded C11 contract: PASS — https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702902963
- Workflow Graph Audit: PASS — https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702902941
- CI: PASS — https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702902878
- RAFCODE shared owned source and evidence receipt: PASS — https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702902867
- Formal Science Orchestrator: SKIPPED
- Internal Custody Ledger: SKIPPED

## Historical negative evidence preserved

First PR check at 7f028872: RAFDB test FAIL line 50 (incorrect SQL literal length 33 vs 34); graph audit FAIL for newly introduced mutable checkout action. Both corrected on successor commits. Original evidence remains at:
- https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702250586
- https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41702250360

## Claims and boundaries

CLAIM_ALLOWED=false. PASS means the named CI gates passed on the named source commit, not that RAFDB has ZIPRAF persistence, MySQL wire compatibility, Oracle parity, RAC, 16x performance, physical multi-ABI execution, safety certification, or real production transaction durability. Those are TOKEN_VAZIO / NOT_RUN.

Rollback: close PR #449 or revert its commits. Existing files were not deleted/renamed. Promotion to main requires user authorization. SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM.
