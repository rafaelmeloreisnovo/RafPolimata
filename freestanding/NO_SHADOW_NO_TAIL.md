# No-shadow / no-tail rule

## Shadow

A second copy of canonical state is forbidden unless it has an explicit architectural purpose (for example, a hardware-mandated staging register or transactional snapshot). Any such copy must name ownership, lifetime and reconciliation law.

## Tail

A backend must not silently divert the final partial vector into a scalar compatibility loop. The residual is explicit:

```text
remaining -> { lane_count, lane_mask }
```

The architecture backend may consume that descriptor with predication/masking (AVX-512 K, SVE P, RVV VL/mask, etc.) or with a fixed-width caller-owned final stage. The generic core does not create a hidden tail routine.

## Pipeline transition

A branch is permitted when it transfers ownership to the next pipeline stage. Value selection inside a hot stage should prefer mask/select lowering when profitable and verifiable.
