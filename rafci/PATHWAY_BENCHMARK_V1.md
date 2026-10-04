# RafCI Pathway Benchmark V1

**Authority:** `rafaelmeloreisnovo/RafPolimata`  
**State:** source contract / benchmark admissibility  
**Global claim:** `claim_allowed=false`

## START HERE — reconstruction in one screen

```text
SOURCE
  -> ARTIFACT / TRANSFORM IDENTITY
  -> PATHWAY CLASS
  -> REQUIRED GATES
  -> MEASUREMENT PROTOCOL
  -> FALSIFIERS
  -> COMPARABILITY
  -> PARETO FRONT
  -> RECEIPT
  -> SCOPED CLAIM CEILING
```

The benchmark does **not** turn every Android layer into freestanding code. It keeps one authorial truth contract and measures hosted adapters without collapsing evidence classes.

```text
freestanding core != NDK != JNI != ART JIT/AOT != SDK Build Tools != R8
                  != QEMU != physical Android/Termux != physical bare-metal
```

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`  
`TOKEN_VAZIO != 0`  
`IMPLEMENTED_UNTESTED != PASS`  
`INCOMPARABLE != WORSE`

## Why this pathway is higher leverage

The smallest sustainable unit is not another implementation for each tool. It is a single admissibility contract that every implementation must pass. Future NDK/JNI/JIT/R8/SDK experiments therefore reuse the same questions:

1. What exact source, bytes, rules and runtime produced the result?
2. What evidence class can this pathway establish?
3. Which claims are structurally forbidden?
4. What was actually measured, and what remains `TOKEN_VAZIO`?
5. Is the comparison scientifically admissible?
6. Which observation would falsify the result?

Only after those questions close may performance, size, latency or energy enter the comparison.

## Canonical pathway classes

| Pathway | Evidence class | Maximum claim |
|---|---|---|
| `freestanding_native` | authorial freestanding core | `FREESTANDING_STRUCTURAL` |
| `android_ndk` | hosted native toolchain adapter | `ANDROID_NATIVE_HOSTED` |
| `jni_bridge` | managed/native boundary | `MANAGED_NATIVE_BOUNDARY` |
| `art_jit_aot` | managed runtime compiler | `MANAGED_RUNTIME` |
| `android_sdk_buildtools` | hosted packaging transform | `PACKAGING_TRANSFORM` |
| `r8_dex_optimizer` | hosted DEX/resource transform | `DEX_TRANSFORM` |
| `physical_termux_native` | physical userspace runtime | `PHYSICAL_USERSPACE` |
| `qemu_emulated` | emulated runtime | `EMULATED_RUNTIME` |
| `baremetal_firmware` | physical firmware/boot runtime | `PHYSICAL_BAREMETAL` |

`pathways.v1.json` owns the machine-readable definitions.

## Epistemic operators

The requested verbs are different operations, not synonyms:

| Operator | Output | Can close a gate? |
|---|---|---|
| infer | hypothesis | no |
| measure | observation | yes, when accepted by that gate |
| analyze | derived evidence | no by itself |
| examine | inspection evidence | yes |
| check / validate | gate decision | yes |
| condition | precondition state | no |
| observe | source-bound observation | yes |
| doubt | falsifier candidate | no |
| catalyze | next-experiment priority | no |
| heuristic | priority hint | no |

A heuristic may decide **what to test next**. It cannot decide that a test passed.

## Benchmark law

### Admissibility before ranking

A pathway is eligible for ranking only when every MUST gate for its class is satisfied and no falsifier fired. Missing required evidence remains `TOKEN_VAZIO`.

### Comparability before speed

Direct comparison requires matching `workload_id`, `semantics_digest`, `protocol_version`, `artifact_role`, `hardware_class` and `runtime_state`, unless an explicit normalization was pre-registered. Otherwise the result is `INCOMPARABLE`, not faster/slower/better/worse.

For ART, JIT/AOT measurements keep cold, warm and profile-guided states separate.

### Pareto before arbitrary scoring

Default selection order:

```text
correctness/gates
  -> provenance + identity
  -> reproducibility
  -> measurement completeness / uncertainty
  -> measured friction/cost
  -> performance / size / energy within a comparable group
```

No scalar weighted winner is permitted until its objective and weights are pre-registered.

## Measurement contract

Timing has a hard floor of 31 valid samples and a recommended default of 101. A precision receipt records at least `iterations`, `median`, `p95`, `p99`, `MAD`, `min`, `max`, `failures`, clock/source identity, hardware class and runtime state.

PMU, cache, thermal and energy values are reported only when measured. Unavailable or unreadable counters remain `TOKEN_VAZIO`; they are never imputed as zero.

## Android-specific falsifiers

The contract rejects or refuses promotion when NDK/native Android is labeled freestanding without freestanding evidence; JNI signature/argument/return/exception shape drifts; JIT cold/warm/profile states are collapsed; SDK packaging is labeled runtime success; R8 transformation/size is labeled native or physical success; R8 transform provenance is incomplete; QEMU is labeled physical; Android/Termux userspace is labeled bare-metal; exact HEAD/hash/ABI changes across a same-artifact experiment; precision is claimed below the sample floor; a heuristic/inference becomes PASS evidence; or a weighted winner is declared without preregistered objective and weights.

## Operational discipline mapping

These are working controls, **not certification claims**:

- RFC-style MUST/SHOULD/MAY semantics; no IETF conformance claim.
- ISO/IEC 27000-family risk/control thinking for boundaries and failure modes.
- ISO 8000-family data-quality thinking for identity, provenance, completeness and unknowns.
- ISO 9001-style traceability for controlled changes, receipts, corrective action and rollback.
- Six Sigma DMAIC/MSA-style discipline for repeatability, variation and causal analysis.
- IEEE-style reproducibility and measurement discipline for engineering evidence.

Certification/conformance remains `TOKEN_VAZIO` unless independently audited for the exact standard and scope.

## Gate receiver / receipt

```sh
python3 rafci/tools/verify_pathways_v1.py \
  --selftest \
  --receipt build/rafci/pathways/receipt.json
```

A successful selftest proves only the pathway dictionary and falsifiers. Its receipt deliberately preserves:

```text
contract_gate=PASS
benchmark_execution=TOKEN_VAZIO
physical_execution=TOKEN_VAZIO
baremetal_execution=TOKEN_VAZIO
claim_allowed=false
```

Real benchmark receipts remain separate execution evidence bound to exact source, artifact, toolchain, protocol and hardware/runtime identities.

## Highest information gain per engineering effort

1. same-artifact ARM64 physical receipt, because the physical acquisition route already exists;
2. JNI boundary benchmark with exact signature/argument/return validation;
3. ART cold vs warm vs profile-guided timing on one workload/device class;
4. NDK dependency/purity inventory against the freestanding reference artifact;
5. R8 before/after identity, rules/mapping digest, size and semantic regression binding;
6. SDK packaging transform provenance and bit-level output inventory;
7. matched QEMU vs physical experiment only as two distinct evidence classes;
8. real bare-metal firmware boot evidence only after a concrete boot target exists.

The order is information gain per effort, not importance.

## Rollback

This subsystem is additive. Roll back by reverting or superseding the pathway-contract PR. Never rewrite historical receipts, relabel a failed falsifier as PASS, or delete contradictory evidence.
