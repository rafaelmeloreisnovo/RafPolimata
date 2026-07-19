# RLL Session Avalanche Orchestrator

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional — normalize→permute→prune→score→route→evidence, scalar como fallback obrigatório) e §8 (entradas canônicas por subsistema — raf_compile.h como backend de execução). Este documento define o plano de execução finito do RafPolimata para sessões de avalanche RLL.

RafPolimata owns the finite execution plan, not the physics.

```text
RLL         -> equations, phase gates and baseline
RafGitTools -> single-session capsule and provenance
RafPolimata -> permutations, pruning, scaling and backend routing
Mapa        -> pointer-only topology
papers      -> optional editorial projection
```

The computational avalanche is:

```text
normalize -> permute -> prune -> score -> route -> evidence
```

It is not the Townsend avalanche model and does not validate it.

Scalar is the mandatory fallback. SIMD/GPU/DSP/NPU require device evidence.
Every generated job preserves:

```text
automatic_cross_repo_write=false
automatic_merge=false
claim_allowed=false
```
