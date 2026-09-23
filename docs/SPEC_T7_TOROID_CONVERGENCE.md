# SPEC — T^7 Toroid / Workflow Projection V1

**State:** `FORMAL_OBJECT_DEFINED / IMPLEMENTATION_MAPPED / CONVERGENCE_PROOF_PENDING`  
**Parent:** ZIPRAF-HW canonical V1 at `4dbd476791e7a6f26e95c5ff6ed9b4b810dc33a9`  
**claim_allowed:** false for 42-attractor convergence/stability claims.  
**Governance binding:** `CLOSURE_L9` — T^7 unknown/proof markers remain open unless their own evidence gate closes; this binding does not promote a claim.

## 1. Mathematical object

The canonical state space is the seven-dimensional torus

[
T^7=(\mathbb R/\mathbb Z)^7=(S^1)^7.
]

A state is

[
s=(u,v,\psi,\chi,\rho,\delta,\sigma),\qquad s_i\in[0,1)
]

with each coordinate interpreted modulo 1.

The fixed-point implementation uses the bijective grid representation

[
q_i\in\{0,\ldots,65535\},\qquad s_i=q_i/65536.
]

Arithmetic that represents toroidal position MUST wrap modulo 65536.

## 2. Canonical coordinate dictionary

| i | symbol | workflow meaning | evidence class |
|---|---|---|---|
| 0 | u | uncertainty / entropy signal | observed or TOKEN_VAZIO |
| 1 | v | coherence / consistency signal | derived from explicit metric |
| 2 | ψ | authorized intention / objective | human-declared |
| 3 | χ | observation / measured input | source-bound |
| 4 | ρ | noise / risk / disturbance | measured or typed uncertainty |
| 5 | δ | material state delta | receipt-bound |
| 6 | σ | memory / accumulated state | append-only lineage |

This dictionary is a workflow projection. It does not assert that human concepts are literally physical torus coordinates.

## 3. Flat torus distance

For x,y in T^7 define per-axis circular separation

[
d_i(x,y)=\min(|x_i-y_i|,1-|x_i-y_i|)
]

and canonical flat distance

[
d_{T^7}(x,y)=\sqrt{\sum_{i=0}^{6} d_i(x,y)^2}.
]

This metric is mathematical infrastructure for comparison. It is distinct from `phi_ethica`, Hamming distance, or any application-specific score.

## 4. Delta evolution

A generic versioned transition is

[
s_{t+1}=\operatorname{wrap}(s_t+\Delta_t).
]

For the current RafPolimata engine, the implemented transition is more specific:

1. `t7_map_input` performs Q16 IIR-like updates with α=1/4;
2. `t7_step` updates H/C through `q16_iir`;
3. `phi=(1-H)C`;
4. `rho` and `delta` receive spiral decay;
5. `psi` receives a phi-dependent increment;
6. `sigma` receives logarithmic IIR accumulation;
7. an auxiliary attractor index evolves in Z/42Z.

The auxiliary 42-state index is **not** the definition of T^7. T^7 is the continuous/periodic seven-coordinate state space; the 42 index is an application-level finite projection.

## 5. Existing implementation mapping

Observed sources:

- `Benchmark/raf_types.h`: `T7_DIM=7`, `T7_MOD=65536`;
- `Benchmark/raf_toroid.h`: `T7State`, `t7_init`, `t7_map_input`, `t7_step`, `t7_coherence`;
- `rafaelia/verbovivo.c`: hosted T^7 pipeline;
- `Apkc/coherence.h`: integer `phi_fst` and 42-slot projection.

### Semantic drift found

The comments declare:

```text
u=entropy, v=coherence, psi=intention, chi=observation,
rho=noise, delta=transmutation, sigma=memory
```

but current `t7_map_input` sources:

```text
u     <- data_hash low 16
v     <- data_hash high 16
psi   <- entropy low 16
chi   <- entropy high 16
rho   <- hw_state low 16
delta <- hw_state shifted 8
sigma <- data_hash XOR hw_state
```

Therefore the current numeric implementation and the semantic dictionary are not yet an identity mapping.

**Gap:** `GAP-T7-SEMANTIC-PROJECTION`.

No silent rewrite is permitted. A future semantic adapter must be versioned and regression-tested against the legacy mapping.

## 6. Workflow projection

For a workflow event e, a projection

[
\Pi_{wf}(e)=(u,v,\psi,\chi,\rho,\delta,\sigma)\in T^7
]

is valid only when each populated coordinate records:

- source/ref;
- normalization rule;
- timestamp;
- uncertainty or TOKEN_VAZIO state;
- version of the projection.

Missing coordinates remain TOKEN_VAZIO at the semantic layer. They MUST NOT be coerced to numerical zero merely to complete a vector.

If a numerical executor requires all seven coordinates, the imputation policy must be explicit, versioned, reversible, and excluded from evidence claims unless separately validated.

## 7. 42-attractor boundary

Current code contains a finite index `attractor in [0,41]` and update/collapse rules. This establishes an implemented 42-slot state machine.

It does **not** establish:

- existence of exactly 42 attractors of the T^7 dynamical system;
- global convergence to them;
- asymptotic stability;
- KAM-theorem applicability to this discrete update;
- a Lyapunov proof.

Those remain `TOKEN_VAZIO_FORMAL_PROOF`.

Required closure artifacts:

1. explicit dynamical map F:T^7->T^7 independent of code comments;
2. fixed/periodic point characterization;
3. basin definition;
4. candidate Lyapunov function or alternative stability argument;
5. perturbation bounds;
6. numerical experiment separated from proof;
7. independent reproduction.

## 8. phi_ethica boundary

The current implementation uses

[
\phi=(1-H)C.
]

This is an application-defined coherence score. It is not Euler's φ, the golden ratio, thermodynamic entropy, or a proof metric.

`Apkc/coherence.h::phi_fst` is a related freestanding integer metric, but its C term is a project-specific normalized construction. Equivalence between `phi_fst` and every hosted `phi_ethica` path is not assumed without a test.

## 9. Evolution invariant

```text
T7_MATH_OBJECT
!= WORKFLOW_PROJECTION
!= LEGACY_NUMERIC_MAPPING
!= 42_SLOT_STATE_MACHINE
!= CONVERGENCE_PROOF
!= PHYSICAL_RUNTIME_EVIDENCE
```

The evolution to T^7 is accepted at the architecture/formalization layer when the seven-axis schema and validator pass. Stronger dynamical claims remain blocked.

## 10. R3

F_ok: T^7 formal object, Q16 representation, seven-axis workflow projection and legacy mapping are now explicitly separated.  
F_gap: semantic projection drift; 42-attractor convergence/stability proof; physical validation of any performance implication.  
F_next: implement a versioned `Pi_wf` adapter with no silent zero-imputation, then test legacy-vs-semantic projection before changing runtime behavior.


## 11. Pi_wf V1 implementation

The versioned semantic adapter is materialized in
`rafaelia/t7_workflow_projection_v1.h`.

Its low-level representation uses:

```text
q[7]          = wrapped Q16-grid coordinates
present_mask  = semantic presence bits
version       = projection contract version
source_ref_hash
normalization_id
```

The decisive invariant is:

```text
TOKEN_VAZIO = coordinate bit absent
numeric 0   = q[i]==0 AND coordinate bit present
TOKEN_VAZIO != numeric 0
```

`Pi_wf_V1` does not mutate `T7State`, does not call `t7_map_input`, and
does not silently fill missing coordinates. It is therefore possible to
compare semantic and legacy projections before selecting a migration policy.

The compatibility helper `t7wf_legacy_raw_coords_v1` mirrors the existing
legacy formulas exactly. `t7wf_compare_legacy_v1` returns a seven-bit mask
of coordinates that are both present in the semantic projection and
numerically equal to the legacy raw mapping.

The regression test additionally executes the real legacy `t7_map_input`
and verifies that the extracted legacy raw coordinates reproduce its current
IIR update. This binds the comparison helper to runtime behavior rather than
to comments alone.

### Migration gate

Changing `Benchmark/raf_toroid.h::t7_map_input` remains forbidden in V1.
A later runtime migration requires an explicit version transition, before/after
fixtures, compatibility policy, and new exact-head receipts.
