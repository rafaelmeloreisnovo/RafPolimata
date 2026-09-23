# Receipt — T7 Application Bridge V2

- parent_main: `2a43cbbf2ef67e72f68a4cbe0bec9ec5614ea44f`
- kind: T7/APPLICATION-BRIDGE/PRE-REPLACEMENT
- governance_closure: `CLOSURE_L9`
- claim_allowed: false
- runtime_replacement_authorized: false

## Delta

`Pi_apply_V2` is an additive bridge from `T7WorkflowProjectionV1` to the
seven `T7State.s[]` coordinates.

Policy: `PRESENT_ONLY`.

- present bit set: apply the exact integer Q16 alpha=1/4 IIR used by legacy `t7_map_input`;
- present bit unset: preserve the previous coordinate exactly;
- numeric zero with the present bit set: apply zero as a real input;
- invalid projection version: no state mutation;
- legacy `t7_map_input`: unchanged and still available.

## Required invariants

1. all seven coordinates present + semantic q equal legacy raw coordinates => exact state parity with legacy `t7_map_input`;
2. missing chi preserves pre-application chi;
3. chi present with q=0 changes chi according to the IIR, proving zero-present != absent;
4. bridge emits applied/preserved masks for auditability.

## Boundary

This bridge is not a runtime replacement and does not define scientific
source→axis calibration. It only makes the already-versioned semantic
projection executable without silent zero-imputation.

## Gate

- Python bridge contract validator;
- hosted C bridge tests;
- freestanding x86-64/ARMv7/AArch64 compile matrix;
- existing T7 fixture and projection gates;
- repository exact-head CI/governance/custody/graph gates.

## R3

F_ok: additive executable bridge specified with legacy parity and absence semantics.  
F_gap: exact-head provider evidence; deliberate runtime selection/replacement policy; physical ARM execution; stronger dynamical proof.  
F_next: exact-head gates; if PASS, integrate bridge without replacing legacy runtime.
