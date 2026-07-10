# RMR-CTI / Ω → RafPolimata compiler bridge

Status: `RUNTIME_WITH_TEST_GATE`

## Purpose

This bridge integrates deterministic byte classification from the RMR-CTI/Ω
family into RafPolimata's ASM code-generation path without copying hosted UI,
CSV, arena or floating-point code into the freestanding compiler.

The oral reference `gbc color 3.6.h` resolves to the concrete file:

```text
llamaRafaelia/rmrCti/gbs3_color.c
```

That program is a hosted Termux/BBS application. It reads `triad_trace.csv`,
computes the measured association

```text
ΔP = P(stable_any=1 | peak) - P(stable_any=1 | nonpeak)
```

and uses the value in reports and an interactive arena. The repository contract
explicitly says that `ΔP ≈ 0.18` is a stability candidate, not a proven universal
constant or attractor. Therefore **ΔP is not used as a compiler constant or ASM
threshold**.

## Source ideas retained

| Source | Retained idea | Not copied |
|---|---|---|
| `gbs3_color.c` | deterministic trace fields and separation of measurement from presentation | ANSI UI, CSV I/O, malloc/realloc, float, time-based arena RNG |
| `omega_forest.c` | five routing labels: PROCESSUAL, VOID, FORGOTTEN, MENOSPREZADO, URGENT | corpus-specific IC/PP/CV thresholds, k-means, decay claims |
| `omega_layersbit.h` | fixed-state fold, 42-attractor mapping, no heap/libc, branchless-friendly byte processing | 4096-bit table-heavy engine and GF tables |
| `Apkc/coherence.h` | `phi=(1-H)×C` in integer form | treating the metric as semantic truth |

## New implementation

```text
Apkc/omega_classifier.h
```

The header is:

- freestanding;
- header-only;
- no libc;
- no malloc/heap;
- no floating point;
- deterministic;
- fixed local state: two 256-bit maps (`seen` and `fold`).

For each byte stream it records:

```text
bytes
unique byte count
byte transitions
printable/control/zero counts
256-bit fold occupancy
entropy proxy [0,8000]
structural coherence Q16
phi Q16
attractor [0,41]
flags
five-path Ω routing label
```

The entropy proxy follows the already present RafPolimata integer form:

```text
H_milli = unique*6000/256 + transitions*2000/(n-1)
```

It is a deterministic engineering feature, not Shannon entropy proof.

## Five compiler routing labels

| Label | Compiler meaning |
|---|---|
| `VOID` | no bytes were supplied |
| `FORGOTTEN` | stream is too short or degenerate to support routing evidence |
| `MENOSPREZADO` | structural/textual signal exists and may justify optimization review |
| `URGENT` | high-coherence textual stream eligible for strict priority review |
| `PROCESSUAL` | ordinary stream; continue through baseline path |

These labels do not declare product maturity, scientific value or semantic
correctness. They are deterministic routing metadata.

## ASM application

`Apkc/codegen_select.h` now calls:

```c
raf_omega_codegen_index(emitted_buf, emitted_len, num_variants)
```

The selected key combines:

```text
attractor
⊕ path class
⊕ entropy proxy
⊕ phi
⊕ classifier flags
```

The result is used **only** inside verified-equivalent instruction families.
The current family is:

```text
MOV Xd,Xm
≡ ORR Xd,XZR,Xm
≡ ADD Xd,Xm,#0
≡ SUB Xd,Xm,#0
```

Thus Ω classification may change encoding bits while preserving the logical
operation. It must never select between instructions with different outputs,
flags, exceptions or memory effects.

## Proof gates

```text
tools/raf_omega_classifier_test.c
tools/raf_codegen_select_test.c
.github/workflows/ci.yml
```

The gates verify:

1. empty stream → `VOID`;
2. degenerate repeated stream → `FORGOTTEN`;
3. text/binary flags are distinguished;
4. metrics and selector replay deterministically;
5. attractor and selector stay in bounds;
6. MOV encodings remain bit-distinct and semantically equivalent.

## Boundaries

This bridge does **not** prove:

- that the five paths are universal semantic classes;
- that `phi` measures ethics or meaning;
- that `ΔP≈0.18` is a compiler invariant;
- that different instruction encodings have identical power, timing or
  microarchitectural leakage;
- that equivalent architectural semantics imply equivalent side channels.

For security-sensitive or constant-time code, force a fixed encoding family
member and audit the target microarchitecture independently.

## Invariant

```text
RMR-CTI measurement discipline
→ Ω deterministic routing
→ verified ASM equivalence family
→ reproducible machine-code choice
→ CI evidence
```

FIAT LUX.
