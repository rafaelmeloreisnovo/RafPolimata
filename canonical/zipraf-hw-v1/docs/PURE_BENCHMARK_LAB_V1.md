# ZIPRAF Pure Benchmark Lab V1

**Area:** hardware / benchmark / freestanding evidence  
**Logical owner:** RafPolimata + ZIPRAF-HW canonical integration  
**Status:** `IMPLEMENTED_SOURCE / CURRENT_HEAD_EXECUTION_PENDING`  
**Canonical entry:** `canonical/zipraf-hw-v1/README.md`  
**Governance:** `CLOSURE_L12` for physical/runtime evidence.  
**Claim:** `claim_allowed=false` until the corresponding runtime/device gates exist.

## Purpose

This layer makes benchmark *analysis* freestanding instead of pretending that
clocks, PMUs, schedulers or thermal sensors are OS-neutral.

The core receives observations from the caller:

```text
caller observations
→ PREHOT/HOT sample arrays
→ factor masks + quality masks
→ deterministic integer analysis
→ 152-byte ZIPRAF benchmark receipt
```

It does **not** call a clock, shell, filesystem, allocator, PMU, scheduler,
provider SDK or environment API.

Therefore:

```text
PURE BENCHMARK MATH != MEASUREMENT ADAPTER != PHYSICAL HARDWARE
```

## PREHOT / HOT protocol

Each promoted phase requires exactly 31 valid samples.

- **CONTROL** — optional explicit reference condition.
- **PREHOT** — measured state before the declared sustained-hot condition.
- **HOT** — measured state after the declared stabilization/heating condition.
- **POSTHOT** — optional recovery/cooldown phase.

For 31 samples the pure core reports exact order statistics:

```text
min  = sorted[0]
p05  = sorted[1]
med  = sorted[15]
p95  = sorted[29]
max  = sorted[30]
```

A missing sample does not become zero. The phase remains `TOKEN_VAZIO`.

## Random Plays without hidden entropy

`zh_bench_order_index()` creates a seeded rotate/reverse permutation for up to
8 variants. Every round is reproducible from `seed + round`; every variant is
visited exactly once per round.

This is an anti-order-bias tactic, **not** a cryptographic RNG and not evidence
that all scheduler/cache effects disappeared.

## 11 layers / 32 interference mechanisms

The numeric catalog is
`registry/benchmark_interference_catalog.v1.json`.

It separates:

0. pure math;
1. source;
2. compiler/toolchain;
3. ISA;
4. microarchitecture;
5. cache/memory;
6. power/thermal;
7. OS/scheduler;
8. virtualization/translation;
9. observer/I/O;
10. physical/external environment.

Each of the 32 factors has its own bit. Factor state uses:

```text
UNKNOWN  != INACTIVE != ACTIVE
```

In particular, `INACTIVE` is not `FAIL`.

## Wide-open result dimensions

Every receipt can identify independently:

- speed group: math / ISA / memory / OS / virtualization / I/O / power / observer;
- measurement unit: raw ticks / cycles / ns / bytes/s / operations/s;
- PREHOT statistics;
- HOT statistics;
- rational speed relation `prehot_median / hot_median` without floating point;
- which interference factors were actually observed;
- which observed factors were active;
- which of 16 quality gates were observed;
- which observed quality gates passed;
- deterministic ordering seed and variant count.

The ratio is stored as numerator/denominator. The core does not manufacture a
floating-point “speedup” and does not compare incompatible units.

## 16 quality gates

The receipt has separate observed/pass masks for source identity, artifact
identity, correctness oracle, timer validity, sample completeness, factor-state
recording, order-seed recording, physical identity, thermal/frequency
observation, stability repetition, independent reproduction, optimizer state,
virtualization state, OS state and preservation of raw samples.

An unobserved gate is never inferred as PASS.

## Optimizer-neutral lane

The canonical pure-object lane compiles with:

```text
-O0 -ffreestanding -fno-builtin -fno-stack-protector
-fno-vectorize -fno-slp-vectorize
```

This records a compiler profile. It does not claim that a CPU has no predictor,
prefetcher, cache, DVFS or other acceleration.

Hardware/software boosters are therefore treated as measured factors, not
silently attributed performance.

## ZIPRAF wire receipt

`ZBR1` is an explicit 152-byte little-endian codec. No native struct layout is
persisted.

It carries metadata, PREHOT/HOT summaries and the exact rational median
relation. Raw samples are deliberately external to the compact receipt; the
`RAW_SAMPLES_PRESERVED` quality bit may be passed only when they are actually
retained by the execution lane.

## Evidence boundary

A synthetic known-vector/selftest proves codec/statistical semantics only.

A physical performance statement additionally requires same-artifact identity,
real timer/unit identity, hardware/device identity and the relevant factor
observations. Otherwise those dimensions remain `TOKEN_VAZIO (CLOSURE_L12)`.

## R3

`F_ok` = source contract for pure benchmark statistics, seeded interleaving,
factor/quality catalog and explicit ZIPRAF receipt.

`F_gap` = exact-head multi-ISA gate + real physical PREHOT/HOT receipts.

`F_next` = execute the exact-head no-helper matrix, then connect one physical
adapter without moving clock/OS/provider authority into the pure core.
