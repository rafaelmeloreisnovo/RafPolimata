# CLOSURE_L16 — Hardware MMIO governance / evidence boundary

**State:** GOVERNED_OPEN (NOT a material closure). **Producer:** RafPolimata. **Scope:** PR #439 source/host-only MMIO fail-closed patch (GPIO M021–M023, SPI M029, I²C M030, PWM M031, AVR M016) and stacked PR #440. This record registers unknowns and their exit criteria; it neither certifies physical safety nor promotes claims.

**Gap ID:** HW-MMIO-L16. **Priority:** P0 physical write safety; P1 device evidence and reproducibility. **Owner role:** firmware/hardware maintainer. **Required independent roles:** board-electrical reviewer and CI reviewer. **Status:** TOKEN_VAZIO for physical target behavior, target-specific pin/clock ownership and watchdog rollback. **claim_allowed:** false. **release_allowed:** false.

## Provenance and observed evidence
- Source: `rafaelmeloreisnovo/RafPolimata`, PR #439, `RAF_hw_status.h` and `docs/hardware/FAIL_CLOSED_MMIO_GATE_V1.md`; producer branch, never the default branch.
- Host mock and compile-only evidence is bounded to recorded GitHub Actions runs/commit SHAs. A successful C object compilation or fake MMIO test is not an AVR flash, physical Raspberry Pi execution, or safe GPIO permission.
- Trigger for this governance hotfix: run 39568456727 at PR head `544bb04db0e8e82e69687e41c92746204471884c` failed the strict changed-line TOKEN_VAZIO validator on two unlinked lines (report hash `7d91138f071627fd48b81bf376b7e8feacde04e886ceedf6e2a2a2ac078fe89f`); all 20 validator unit tests passed. Linkage fixes must preserve strict rejection of unknown ungoverned gaps.

## Conditions required to close HW-MMIO-L16 materially
1. Record exact source HEAD SHA, changed file hashes, CI run IDs, job conclusions and logs; strict changed-line validator and host negative tests must pass **at the final HEAD**.
2. Board/SoC/address/register map and separate Clock Manager/PWM mapping independently reviewed; target-specific pin and shared-clock ownership documented by a human.
3. Timeout behavior, cleanup on failure, electrical limits, and output restoration verified on the exact board with bounded power-safe instrumentation; explicit human approval before activating a physical write flag.
4. Cross-compiler firmware link/ELF/HEX, real device execution and physical receipt independently verified. Distinguish host C, ARM32 object, AVR compiled object, flashed firmware and board behavior.
5. Provider-enforced review and required checks demonstrably reject an unsafe candidate; independent human review and non-stale provider governance receipt.
6. Produce an evidence-backed successor record and independent review. This file itself is **not** evidence of gates 1–5.

## Falsifiers / negative controls
- Missing/aliased PWM-vs-CM mappings, invalid/NULL pointers or wrong SoC must fail closed without writes.
- Stuck SPI/I²C ready/status bits must not cause unbounded waits.
- Unsupported architecture/no device must never report numeric success for an unknown state.
- Missing physical receipt, missing HEAD binding or failed provider check must keep `claim_allowed=false`.
- Any test that only runs host mock code must not be labeled physical-device PASS.

**Rollback:** revert only branch commits or close Draft PR; preserve original main, recorded failures, upstream licenses and receipt history. **R3:** F_ok=typed governance link; F_gap=physical/provider/independent evidence; F_next=exact-HEAD CI and instrumented board review.
