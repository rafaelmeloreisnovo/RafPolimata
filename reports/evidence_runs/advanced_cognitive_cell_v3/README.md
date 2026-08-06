# Advanced Cognitive Cell V3 — Local Evidence Run

**Artifact:** `RAFAELIA-ACC-V3-20260806`  
**Evidence state:** `VERIFIED_LIMITED_LOCAL`  
**Canonical producer:** `rafaelmeloreisnovo/GAIA_phi`  
**Producer PR:** `GAIA_phi #71`  
**Observed producer head:** `271c2ff4599737a61840d185096ecefd24bdfc78`  
**Claim boundary:** `claim_allowed=false`

## Responsibility boundary

RafPolimata stores the evidence record and reproduction boundary. It does not copy or become the canonical owner of the PyTorch implementation.

Canonical code paths:

```text
GAIA_phi/dados/cognitive_memory/advanced_cognitive_cell_v3.py
GAIA_phi/tests/test_advanced_cognitive_cell_v3.py
GAIA_phi/docs/ADVANCED_COGNITIVE_CELL_V3.md
GAIA_phi/docs/ADVANCED_COGNITIVE_CELL_V3_CLAIMS.json
```

## Reproduction command

After checking out the observed GAIA_phi branch/head:

```bash
python tests/test_advanced_cognitive_cell_v3.py
```

Observed local stdout:

```text
ADVANCED_COGNITIVE_CELL_V3_TESTS_PASS
```

## Gates covered

| Gate | State |
|---|---|
| output/state shapes | `PASS_LOCAL` |
| finite outputs and gradients | `PASS_LOCAL` |
| causal-prefix isolation | `PASS_LOCAL` |
| same input + same explicit state produces same output | `PASS_LOCAL` |
| incompatible state shape rejection | `PASS_LOCAL` |
| remote CI | `TOKEN_VAZIO` |
| training convergence | `TOKEN_VAZIO` |
| quality against baselines | `TOKEN_VAZIO` |
| p50/p95/p99 and peak memory | `TOKEN_VAZIO` |
| Android/Termux deployment | `TOKEN_VAZIO` |
| SOTA claim | `TOKEN_VAZIO` |

## Interpretation

The local run establishes that the current source is executable in the observed environment and satisfies the four bounded unit gates. It does not establish usefulness after training, production readiness, biological validity, long-context superiority or actual storage compression.

`Xi` is currently an evidence-conditioned write/retention gate. Because the recurrent tensor remains `[B, S, D]`, no compression-ratio claim is allowed.

## Falsifiers and next evidence

1. Any non-finite activation or parameter gradient invalidates the finitude gate.
2. Any changed prefix under a future-only input perturbation invalidates the causal gate.
3. Any output difference for identical explicit state/input invalidates the purity gate.
4. A baseline or ablation showing no reliable benefit prevents promotion of architectural-value claims.
5. A remote job without observable steps remains `TOKEN_VAZIO_RUNNER`, not `PASS`.

## Retrofeedback

- `F_ok`: executable local packet with deterministic bounded tests.
- `F_gap`: no training, baseline, remote runner or deployment evidence.
- `F_next`: controlled task benchmark and component ablations at the immutable GAIA_phi head.
