# RMR-CTI / Ω → RafPolimata compiler bridge

> **Entrada canônica:** `docs/AGENTES.md` §8 (entradas canônicas por subsistema — `Apkc/omega_classifier.h` e `Apkc/codegen_select.h`) e §5 (pipeline operacional — invariante: source bytes → signed Omega ops profile → CI evidence).

Status: `RUNTIME_WITH_TEST_GATE`

## Purpose

This bridge integrates deterministic byte classification from the RMR-CTI/Ω
family into RafPolimata's compiler and ASM code-generation paths without
copying hosted UI, CSV, arena or floating-point code into the freestanding
compiler.

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

## Root compiler manifest

`raf_compile` now classifies the source bytes before lowering and writes a
signed `ops_schema=2` record containing:

```text
omega_entropy_milli
omega_phi_q16
omega_attractor
omega_flags
omega_path
omega_path_name
```

The numeric Omega fields participate in `ops_signature`, so changing a source
classification changes the operational fingerprint. Missing source input starts
as `VOID`; a successfully read source is classified from its real bytes.

This applies to every language currently recognized by the root compiler. It
does not imply that all those languages already have source-dependent native
lowering: `raf_precomp.c` remains an explicit deterministic `return 42` anchor.
The APKc language profiles are a separate and more advanced packaging path.

## Proof gates

```text
tools/raf_omega_classifier_test.c
tools/raf_codegen_select_test.c
scripts/test_ops_manifest.sh
scripts/validate_ops_manifest.py
scripts/compare_ops_manifest.py
.github/workflows/ci.yml
```

The gates verify:

1. empty stream → `VOID`;
2. degenerate repeated stream → `FORGOTTEN`;
3. text/binary flags are distinguished;
4. metrics and selector replay deterministically;
5. attractor and selector stay in bounds;
6. MOV encodings remain bit-distinct and semantically equivalent;
7. Omega fields are range-checked, name-matched and covered by the `.ops`
   signature;
8. two builds of the same source preserve identical stable Omega fields.

## Boundaries

This bridge does **not** prove:

- that the five paths are universal semantic classes;
- that `phi` measures ethics or meaning;
- that `ΔP≈0.18` is a compiler invariant;
- that different instruction encodings have identical power, timing or
  microarchitectural leakage;
- that equivalent architectural semantics imply equivalent side channels;
- that the root `raf_compile` already performs complete language parsing and
  source-dependent lowering.

For security-sensitive or constant-time code, force a fixed encoding family
member and audit the target microarchitecture independently.

## Placement of the remaining Omega programs

| Program | Correct compiler role |
|---|---|
| `omega_neuro_full.c` | hosted post-build/source profiler; not part of the freestanding encoder because it uses heap, libc, floating point, token sets and timing |
| `omega_forest.c` | artifact/corpus routing after metrics exist; its IC/PP/CV and k-means rules are not instruction-selection semantics |
| `omega_frames_export.c` | export/curation bridge for memory frames; belongs after classification, not inside assembly emission |
| `omega_layersbit.h` | strongest future candidate for an optional full 4096-bit compiler profile when table size and stack/BSS cost are explicitly accepted |

## Invariant

```text
source bytes
→ signed Omega ops profile
→ RMR-CTI measurement discipline
→ verified ASM equivalence family
→ reproducible machine-code choice
→ CI evidence
```

FIAT LUX.
