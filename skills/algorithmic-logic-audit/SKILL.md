# Algorithmic Logic Audit Skill — RAFAELIA V1

**Status:** `METHOD_DEFINED`  
**Scope:** algorithms, source code, pseudocode, state machines, binary formats, parsers, compilers, runtimes, numerical kernels and mathematical transforms.  
**Default:** `claim_allowed=false` until the exact property being claimed is supported by the applicable proof/test/evidence gate.  
**Parent skill:** `skills/academic-falsifiability/SKILL.md`.

## 1. Purpose

Audit what an algorithm **actually does**, rather than what its name, comments, README, symbolism or intended architecture says it does.

Canonical separation:

```text
INTENTION != SPECIFICATION != IMPLEMENTATION != EXECUTION != EVIDENCE != PERFORMANCE != NOVELTY
```

The skill reconstructs the executable logic, extracts invariants, searches for counterexamples, identifies stubs and semantic mismatches, and returns the smallest falsifiable next action.

It never promotes absence to zero and never promotes an unexecuted path to a PASS.

## 2. Activation

Use this skill when the request contains one or more of:

- analyze/audit/review an algorithm or logic;
- find stubs, gaps, uncertainty, incoherence or wrong behavior;
- compare specification with implementation;
- validate encode/decode, compress/decompress, pack/unpack or transform/inverse pairs;
- inspect ARM/ISA addressing, register order, endianness, alignment or ABI behavior;
- verify parser/state-machine/control-flow correctness;
- evaluate a numerical or mathematical implementation;
- determine whether a claimed property is really implemented or only documented.

## 3. Evidence hierarchy

Prefer evidence in this order:

```text
E0 name/comment/README
E1 source inspection
E2 static derivation/model
E3 executable unit/adversarial test
E4 clean-environment reproduction
E5 differential/reference comparison
E6 independent replication
```

A higher label must not be inferred from a lower one.

If only E1 exists, say `STATIC_ONLY`. If execution is unavailable, record `TOKEN_VAZIO_RUNTIME` rather than assuming behavior.

## 4. First operation: freeze identity and boundary

Before judging correctness, capture:

- repository + path;
- branch/tag/commit or file SHA;
- language/runtime/ISA when relevant;
- declared version/format/protocol;
- entry points and outputs;
- external dependencies;
- files intentionally out of scope.

Never audit an ambiguous moving target as if it were a fixed artifact.

## 5. Reconstruct the algorithm

For each material algorithm, produce a compact model:

```text
inputs
→ preconditions
→ state
→ transformation steps
→ branch/transition conditions
→ outputs
→ errors/failure states
→ side effects
→ inverse/consumer (if any)
```

Extract explicitly:

1. **domain** — valid inputs;
2. **codomain** — possible outputs;
3. **state variables** — mutable state and ownership;
4. **invariants** — properties expected to survive every step;
5. **termination rule** — why loops/recursion finish;
6. **failure semantics** — fail-open, fail-closed or silent;
7. **observability** — what evidence proves the path ran.

If any item cannot be recovered, preserve it as `TOKEN_VAZIO_<TYPE>`.

## 6. Seven-axis audit kernel

Every material algorithm is tested against the seven axes below. Each axis records `PASS | FAIL | PARTIAL | TOKEN_VAZIO | NOT_APPLICABLE` and the supporting pointer.

### A1 — Semantic fidelity

Question: does implementation match the declared operation?

Check:

- comments/spec vs executable branch behavior;
- units, sign conventions, endianness and ordering;
- copied or superseded versions;
- placeholder/simulated primitives presented under production names;
- algorithm description inconsistent with actual code.

### A2 — Domain totality and boundaries

Question: is behavior defined over the claimed input domain?

Probe:

- empty input;
- singleton/minimum input;
- maximum/boundary values;
- sentinel/reserved values;
- malformed/truncated input;
- overflow/underflow/wraparound;
- Unicode/binary/null bytes when applicable;
- resource limits and truncation.

A passing happy path never proves totality.

### A3 — State and control-flow coherence

Question: can state evolve only through valid transitions?

Check:

- unreachable/dead states;
- stale state reused between calls;
- mutation of caller-owned data;
- missing restoration on exceptions;
- branch ordering;
- off-by-one transitions;
- error paths returning success;
- state machine transitions without guards.

For low-level code additionally check register lifetime, aliasing, clobbers, stack discipline and ABI assumptions.

### A4 — Inverse, round-trip and conservation

Question: what must be preserved across a transformation?

For a pair `f` and `g`, test the exact required property:

```text
g(f(x)) == x                 # left inverse / round-trip
f(g(y)) == y                 # right inverse when required
size/accounting conserved
checksum/hash coverage exact
index/address mapping injective where claimed
```

Do not call an encoding reversible until reserved markers/escape rules are proven for the full domain.

For data movement, also test duplication, omission, reordering and alias collision.

### A5 — Numerical, algebraic and resource correctness

Question: is the mathematical/computational property implemented correctly and within bounds?

Check:

- exact recurrence/equation vs code;
- integer width and promotion;
- floating-point stability and NaN/Inf behavior;
- fixed-point scaling and saturation;
- dimension/unit consistency;
- asymptotic cost vs actual allocation/copy behavior;
- whole-file buffering vs claimed streaming;
- recursion/loop upper bounds;
- claimed compression/performance vs format overhead.

Known mathematics must not be relabeled as novelty merely because notation changed.

### A6 — Determinism, concurrency and adversarial safety

Question: which outputs are reproducible, and under what threat model?

Check:

- timestamps, randomness, UUIDs, locale, filesystem order and hash-map order;
- races, shared mutable state and non-atomic updates;
- parser ambiguity;
- integer-controlled allocation/DoS paths;
- checksums confused with authentication;
- recomputable integrity metadata presented as authenticity;
- validation occurring after dangerous use;
- malformed input that changes parser framing.

Rule:

```text
integrity != authenticity != confidentiality
```

### A7 — Evidence, stubs and claim coherence

Question: does the evidence support the exact claim?

Search for:

- `TODO`, `FIXME`, `pass`, `NotImplemented`, `stub`, `mock`, `simulated`, placeholder constants;
- tests that do not assert the claimed property;
- examples labeled as tests;
- CI jobs that never executed steps;
- generated artifacts with missing generator provenance;
- local PASS promoted to independent validation;
- benchmark without baseline, workload, repetitions or environment.

Every unresolved material item becomes an explicit gap; it is never silently averaged away.

## 7. Counterexample-first adapters

Use the strongest applicable adapter.

### 7.1 Encoder / compressor / binary format

Test at minimum:

- all reserved marker bytes as literals;
- runs immediately below/at/above threshold;
- empty input;
- arbitrary binary data;
- truncated header/payload/checksum;
- inconsistent declared lengths;
- unknown version;
- corrupted payload with recomputed weak checksum when threat model allows it;
- `decode(encode(x)) == x`.

### 7.2 Parser / JSON / text protocol

Test:

- whitespace variants;
- escaped quotes/backslashes;
- delimiters inside strings;
- nested structures;
- duplicate/missing keys;
- truncated input;
- oversized lengths;
- invalid UTF-8 when input is bytes.

### 7.3 ARM / ISA / address transformation

Test:

- architectural register order vs display/storage order;
- little/big-endian interpretation;
- aligned and unaligned addresses;
- base + index + scale arithmetic;
- sign/zero extension;
- wraparound at word width;
- relocation/readdressing applied exactly once and at the correct phase;
- aliasing of dynamic addresses;
- caller/callee-saved register assumptions.

Do not infer semantic register order from incidental container/list order.

### 7.4 Numerical / scientific kernel

Test:

- analytically solvable cases;
- zero/identity/symmetry cases;
- dimensional consistency;
- extreme magnitude;
- sensitivity to precision;
- reference implementation or independent formula;
- negative control and falsifier.

### 7.5 State machine / orchestrator

Test:

- every legal transition;
- every illegal transition;
- duplicate event/replay;
- interruption and resume;
- rollback;
- partial persistence;
- conflicting concurrent update;
- terminal states receiving new input.

## 8. Specification-to-code mismatch classes

Use these normalized finding codes:

- `SPEC_IMPL_DIVERGENCE` — documentation and code perform different operations;
- `DOMAIN_HOLE` — claimed input domain contains an undefined/incorrect case;
- `NON_INVERTIBLE_PATH` — claimed inverse/round-trip fails;
- `STATE_LEAK` — state survives or mutates unexpectedly;
- `ORDERING_ERROR` — sequencing/register/index/address order is wrong;
- `BOUNDARY_ERROR` — off-by-one, truncation, overflow or malformed boundary handling;
- `FAIL_OPEN` — error path can still report success or continue unsafely;
- `NONDETERMINISM_UNDECLARED` — variable output is not part of the declared contract;
- `RESOURCE_CLAIM_MISMATCH` — implementation contradicts streaming/memory/performance claim;
- `CRYPTO_SEMANTIC_MISMATCH` — integrity/authenticity/confidentiality claim exceeds primitive;
- `STUB_OR_SIMULATION` — placeholder is present in a material path;
- `EVIDENCE_GAP` — implementation may be plausible but execution/proof is absent;
- `CLAIM_OVERREACH` — evidence is narrower than the stated conclusion;
- `TOKEN_VAZIO` — available evidence cannot decide.

## 9. Severity

Severity is about consequence, not aesthetic quality.

- `S0 INFO` — style/clarity or non-material observation;
- `S1 LOW` — maintainability/observability weakness;
- `S2 MEDIUM` — bounded logic/spec mismatch without demonstrated loss/corruption;
- `S3 HIGH` — incorrect result, silent truncation, non-invertibility, fail-open or major state error;
- `S4 CRITICAL` — realistic corruption, arbitrary memory/safety failure, security boundary bypass or invalid scientific promotion with high consequence.

If consequence is unknown, do not inflate severity: use `TOKEN_VAZIO_IMPACT`.

## 10. Required finding record

Each finding MUST contain:

```json
{
  "finding_id": "ALG-...",
  "severity": "S0|S1|S2|S3|S4",
  "class": "DOMAIN_HOLE",
  "artifact": {"repo": "...", "path": "...", "ref": "..."},
  "claim_or_invariant": "...",
  "observed_logic": "...",
  "counterexample_or_trigger": "...",
  "evidence": [],
  "uncertainty": [],
  "token_vazio": [],
  "claim_allowed": false,
  "smallest_fix_or_test": "..."
}
```

The `observed_logic` field must describe behavior, not intent.

## 11. Promotion gates

A property such as `ROUNDTRIP_CORRECT`, `DETERMINISTIC`, `STREAMING`, `INVERTIBLE`, `FAIL_CLOSED`, `ARM_ORDER_CORRECT`, `PERFORMANCE_GAIN` or `SCIENTIFICALLY_VALIDATED` is allowed only if its specific gate passes.

Examples:

```text
ROUNDTRIP_CORRECT
= domain declared
& inverse defined
& adversarial marker/boundary cases pass
& property test/reference proof supports the claimed domain

DETERMINISTIC
= same normalized inputs
& same declared environment
& nondeterministic sources removed/frozen/declared
& repeated outputs match at the stated comparison level

PERFORMANCE_GAIN
= fixed workload
& fixed baseline
& environment recorded
& warmup/repetition/statistic declared
& correctness preserved
& measured delta supports the exact claim
```

Do not use a single global `claim_allowed=true` to imply unrelated properties passed.

## 12. Mandatory output

Every audit returns, in this order:

1. **Scope/identity** — exact artifacts and refs inspected;
2. **Algorithm map** — reconstructed executable logic;
3. **Invariants** — declared and inferred, clearly separated;
4. **Findings** — ordered by severity and dependency;
5. **Stubs/TOKEN_VAZIO register**;
6. **Counterexamples/tests** — executed or proposed, never conflated;
7. **Claim matrix** — property → evidence level → state;
8. **F_ok / F_gap / F_next**.

Use compact tables when many findings exist.

## 13. Anti-regression rules

- Preserve negative findings append-only until a new event demonstrates closure.
- A later PASS does not erase the earlier FAIL; it closes it with provenance.
- Do not rewrite historical versions to make the current architecture appear cleaner.
- Do not collapse duplicate-looking files before hashing/identity comparison.
- A documentation fix cannot close an implementation bug.
- A code fix cannot close a runtime claim until the relevant execution is observed.
- A local execution cannot close an independent-replication requirement.

## 14. Example derived from a real RAFAELIA pattern

If an RLE encoder reserves `0xFF` as a three-byte marker but emits short runs/literals directly, then a literal `0xFF` followed by two ordinary bytes may be consumed by the decoder as a run marker.

Correct audit outcome:

```text
class = NON_INVERTIBLE_PATH
property = decode(encode(x)) == x
counterexample = construct binary input containing literal marker + 2 bytes
state = FAIL until escaping/framing is implemented and round-trip tests pass
```

This does not imply the entire format is worthless. It precisely bounds the failed invariant and the next repair.

## 15. Retroalimentação contract

Every run closes with:

```text
F_ok   = properties actually sustained by source/proof/execution
F_gap  = failures + contradictions + critical TOKEN_VAZIO
F_next = smallest test/fix that most reduces uncertainty without widening scope
```

A rigorous `TOKEN_VAZIO` is preferable to a confident fiction.
