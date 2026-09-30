# Evidence Garden V2 — falsifier closure, void permutations and open-world guard

**Governance:** `CLOSURE_L2` for runtime evidence; `CLOSURE_L12` for physical/device scope.  
**Lifecycle:** `AUDIT / CURRENT_COMMIT_EVIDENCE_REQUIRED`.  
**Machine matrix:** `Benchmark/evidence_garden/falsifier_matrix.v2.json`.  
**Executable audit:** `scripts/evidence_garden_falsifier_audit.py`.  
**Negative suite:** `tests/test_evidence_garden_falsifiers.py`.

## 1. Why V2 exists

A passing positive benchmark is not enough. Every claim can fail because an identity is absent, an oracle is wrong, an executable is missing, an observer changes the experiment, a statistical interval is unsupported, a reproduction receipt belongs to another source, or a stronger physical statement is being inferred from a weaker virtual observation.

V2 therefore treats the informational void itself as a testable object.

```text
KNOWN != COMPLETE
ABSENT != ZERO
UNOBSERVED != FALSE
IMPLEMENTED != EXECUTED
EXECUTED != REPRODUCED
REPRODUCED != UNIVERSAL
```

The target is not “prove there can never be another falsifier.” A finite test suite cannot establish that. The valid closure is narrower:

```text
KNOWN_FALSIFIER_CLOSURE = PASS
UNKNOWN_UNKNOWN          = TOKEN_VAZIO
UNIVERSAL_COMPLETENESS   = FORBIDDEN
```

## 2. Alpha/Omega open-world guard

The symbolic `Alpha/Omega` name means the declared beginning and end of the **current audit domain**, not the end of all possible knowledge.

```text
ALPHA = first declared source/assumption
OMEGA = last declared claim/gate
VOID  = information not established inside the route
```

The route is complete only in the bounded sense that every declared element between those endpoints has a disposition. Anything not yet known to be part of the route remains outside that finite closure.

This is the same discipline behind the repository invariant:

```text
SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM
TOKEN_VAZIO != 0
```

## 3. Human symbolic traditions are semantic bridges, not evidence gates

Human traditions often use paired or cyclic symbols for completeness, transformation and return: beginning/end, fullness/emptiness, complementary poles, triads, circles, octants and recursive cycles. In this repository those images may help name topology or navigation, but they do not promote technical claims.

Examples such as Alpha/Omega, light/darkness, Bagua-style directional decomposition, Taoist complementarity, triadic symbolism or liturgical phrases may be used as **metaphorical indexing only**.

```text
analogy != mechanism
symbol != measurement
tradition != experimental receipt
meaning != causal proof
```

## 4. Void dimensions

V2 declares twelve dimensions in which absence can break or narrow a result:

```text
source_identity
config_contract
artifact_identity
command_identity
correctness_oracle
observer_channel
performance_samples
intervention_attribution
statistical_support
reproduction_identity
physical_visibility
claim_scope
```

Each dimension is linked to at least one falsifier and one Evidence Garden station.

## 5. Pairwise permutations

For `12` declared void dimensions, V2 audits every unordered pair:

```text
C(12,2) = 66
```

The pairwise audit does **not** claim that 66 pairs exhaust every future interaction. It establishes that no pair inside the declared V2 dimension set is left without a fail-closed disposition.

Composition rule:

```text
blocking ∧ anything -> claim cannot PASS
bounded_unknown ∧ bounded_unknown -> retain TOKEN_VAZIO / OBSERVED_UNPROMOTED
```

Higher-order interactions remain eligible for new tests when evidence, contradiction or a newly discovered failure mechanism creates a new dimension or falsifier.

## 6. Executable falsifier families

The negative suite includes configuration rejection, missing source/artifact/command identity, incorrect exit/oracle, missing required observer, optional observer degradation, invalid sample geometry, timeout, small-n uncertainty, reproduction identity mismatch, receipt schema mismatch and attempted claim promotion.

A failed negative test is a real regression. It must not be converted to green by weakening the expected state.

## 7. Stop rule

The audit may stop for the current commit only when:

```text
matrix_structure              = PASS
declared_dimension_coverage   = PASS
declared_station_coverage     = PASS
pairwise_void_disposition     = PASS
executable_negative_suite     = PASS
positive_framework_suite      = PASS
current_commit_CI             = PASS
UNKNOWN_UNKNOWN               = TOKEN_VAZIO
claim_allowed                 = false
```

New information reopens the relevant route automatically. This is not regression; it is the intended open-world model.

## 8. Observation and biological hypotheses

A striking observation can legitimately create a hypothesis, but it does not choose its own mechanism.

For example:

```text
observation: animal approached a familiar person
H1: individual visual recognition
H2: learned association with feeding routine
H3: motion / contrast / polarization cues
H4: timing or environmental cue
H5: another sensory channel
Hmagnetic: magnetic-field detection
```

The observation alone cannot promote `Hmagnetic` over the alternatives. Each mechanism needs a discriminating experiment. This is exactly the Evidence Garden distinction between `OBSERVATION`, `HYPOTHESIS`, `FALSIFIER`, `EXECUTION` and `CLAIM`.

## R3

`F_ok`: known-domain falsifier matrix, 12 void dimensions, 24 declared falsifiers, 66 pairwise void permutations, strict config validation, receipt contract validation and negative-suite routing are materialized.  
`F_gap`: higher-order and not-yet-conceived falsifier classes remain open-world `TOKEN_VAZIO`; physical visibility remains separately gated.  
`F_next`: execute the full current-commit CI and preserve the generated falsifier audit receipt; any failure becomes the next smallest reproducible correction.
