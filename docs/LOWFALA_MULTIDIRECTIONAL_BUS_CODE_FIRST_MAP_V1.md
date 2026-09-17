# LowFala-adjacent Language Bus and Multidirectional Runtime Map — V1

**Status:** `REFERENCE / CODE_FIRST / DOC_LAG_EXPLICIT`  
**Area:** RafPolimata language/runtime/low-level architecture  
**Logical owner:** low-level language/runtime maintainers  
**Canonical relation:** `docs/INDEX.md -> this map -> current source`  
**Claim boundary:** documentation locator only; no new runtime PASS is claimed.

## 1. Why this map exists

Parts of the historical documentation lag the current code. In particular, older documents still describe a 12-language cut, while the current `Apkc/lang_profile.h` defines `LP_COUNT = 23` and adds GPU/DSP/NPU-oriented capability fields and execution families.

The operational rule for this area is therefore:

```text
current source
-> current tests/contracts
-> current narrow documentation
-> historical documentation only as provenance
```

Do not use an older language count or old backend map to override the source currently present in `main`.

## 2. LowFala boundary

RafPolimata does not currently contain the LowFala compiler/VM authority itself. The research bridge at:

```text
research/MULTILINGUAL_PHYSICS_PHONETICS_V1/README.md
```

records LowFala as a separate authority in `rafaelmeloreisnovo/ChipQuantum` and describes its executable path as:

```text
FALA -> TOKEN -> AST -> BYTECODE -> VM
```

The same document explicitly states that the planned:

```text
FONEMA -> MORFEMA -> SEMANTICA
```

layer must not be treated as implemented there.

Inside RafPolimata, the closest operational family is the ApkC language/profile/dispatch machinery described below.

## 3. Language bus: current source of truth

### 3.1 Declarative entry table

Canonical file:

```text
Apkc/lang_profile.h
```

Current declared profile IDs:

```text
LP_ASM, LP_C, LP_CPP, LP_RS, LP_KT, LP_JAVA, LP_PY, LP_SH,
LP_PL, LP_JS, LP_PHP, LP_JSX, LP_GO, LP_RB, LP_SWIFT, LP_GROOVY,
LP_CLJ, LP_GLSL, LP_CL, LP_HLSL, LP_WGSL, LP_DSP, LP_TFLITE
```

with:

```text
LP_COUNT = 23
```

Each profile is required by `lang_profile_validate()` to belong to exactly one execution family among:

```text
use_asm
use_script
use_fork
use_gpu_spv
use_gpu_cl
use_gpu_wgsl
use_dsp
use_npu
```

Additional capability fields include `use_branchless`, `dex_output`, `arm64_only`, `use_d8` and `jsx_node`.

This is the first fan-out layer: one recognized language/profile is routed into one declared family. Unknown or invalid profiles fail closed.

## 4. Direct branchless frontend bus

Canonical file:

```text
Apkc/apkc_language_dispatch.h
```

The canonical language identity remains the `LP_*` index from `lang_profile.h`; no private second numbering is permitted.

The current scoped direct route first attempts the bounded common semantic subset:

```text
return arithmetic fragment
```

and, when not applicable, routes selected branchless language profiles through language-specific structural frontends.

Current switch cases observed in source:

```text
LP_PY
LP_GO
LP_RS
LP_C
LP_JS
LP_JAVA
LP_SWIFT
```

The result metadata distinguishes:

```text
APKC_FRONTEND_NONE
APKC_FRONTEND_LANGUAGE_SPECIFIC
APKC_FRONTEND_BOUNDED_SEMANTIC_SUBSET
```

and keeps semantic proof separate from frontend selection.

This does **not** prove full semantics for all 23 profiles.

## 5. Direct language compiler implementation

Canonical file:

```text
Apkc/compiler_language_direct.h
```

The code generator emits the linear machine instruction set directly. The source contains compiler/scanner structures for Python, Go, Rust, C, JavaScript, Java, Swift and an additional Kotlin helper, plus `UniversalCompiler` compatibility logic.

The source comment describes this route as direct tokens-to-instructions with no AST/IR intermediate layer. That statement is a source-level design claim; equivalence and completeness remain bounded by the actual supported grammar/tests.

## 6. Runtime dispatch bus — the multidirectional fan-out

Canonical file:

```text
Benchmark/raf_runtime_router.h
```

Current runtime backends:

```text
RAF_BACKEND_GENERIC_C
RAF_BACKEND_ARM32_NEON
RAF_BACKEND_ARM64_NEON
RAF_BACKEND_GPU_BATCH
RAF_BACKEND_SYSCALL_DIRECT
RAF_BACKEND_STORAGE_BUFFER
```

The router receives:

```text
caps
degraded
min_batch_gpu
batch
state
```

and returns:

```text
backend
fallback
fail_safe
failover
rollback
mitigation
```

The route is selected from usable capabilities with fallback to generic C. Candidate/validated states can enter non-generic paths; non-live state falls back. Degraded capability, unavailable GPU threshold and backend errors are represented separately through failover/mitigation/rollback logic.

### Important interpretation

This is the closest code-level referent to the metaphor of a bus that can "turn toward every side": it is a **fan-out/fan-in capability router across several execution directions**.

It is not a physical circular bus and it is not literally rotating.

## 7. The file that explicitly calls it a dispatch bus

Canonical file:

```text
Benchmark/raf_bus_throughput.h
```

This benchmark names two distinct bus paths already present in the codebase:

```text
memory bus   : sequential vs strided buffer copy
 dispatch bus: raf_runtime_route() backend dispatch
```

`raf_bus_dispatch_op()` invokes `raf_runtime_route()` and returns the selected backend.

Therefore, when looking for the software "bus" that opens into multiple runtime directions, start here and follow:

```text
Benchmark/raf_bus_throughput.h
        |
        v
Benchmark/raf_runtime_router.h
        |
        +-> GENERIC_C
        +-> ARM32_NEON
        +-> ARM64_NEON
        +-> GPU_BATCH
        +-> SYSCALL_DIRECT
        `-> STORAGE_BUFFER
```

## 8. Literal circular ring — do not confuse with multidirectional dispatch

Canonical file:

```text
RAF_033_dma_circular.c
```

This is a literal two-control-block DMA ring:

```text
CB0 -> CB1 -> CB0 -> ...
```

The last control block points back to the first through `nextconbk`. Its purpose is continuous streaming without CPU intervention when backed by valid bus-accessible physical memory and DMA hardware.

This ring is circular, but it is **not** a many-direction language/runtime selector. It is a two-node cyclic chain.

## 9. Three different meanings of "bus" now separated

| Name | Source | Shape | Meaning |
|---|---|---|---|
| language/profile bus | `Apkc/lang_profile.h` | 23-profile fan-out into execution families | source-language routing |
| runtime dispatch bus | `Benchmark/raf_runtime_router.h` + `Benchmark/raf_bus_throughput.h` | capability fan-out/fallback | runtime backend selection |
| DMA circular ring | `RAF_033_dma_circular.c` | `CB0 -> CB1 -> CB0` | continuous hardware DMA chain |

Do not collapse these three into one claim.

## 10. Documentation lag confirmed

Historical documents containing a 12-language view include, among others:

```text
RAF_CHECKLIST_96_ITEMS.md
docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md
docs/arch/ANDROID_NDK.md
```

Those counts must be treated as historical cuts when they disagree with current `Apkc/lang_profile.h`.

A newer bridge already exists in:

```text
docs/LANGUAGE_COMPLETION_FREESTANDING_METHODOLOGY.md
```

which explicitly classifies the 23 current entries and records the 16-lane plan. Even that document remains a methodology/reference layer and does not supersede source/test evidence for current implementation details.

## 11. Code-first reading route

For this topic, read in this order:

```text
1. Apkc/lang_profile.h
2. Apkc/apkc_language_dispatch.h
3. Apkc/compiler_language_direct.h
4. Benchmark/raf_runtime_router.h
5. Benchmark/raf_bus_throughput.h
6. RAF_033_dma_circular.c
7. docs/LANGUAGE_COMPLETION_FREESTANDING_METHODOLOGY.md
8. research/MULTILINGUAL_PHYSICS_PHONETICS_V1/README.md
9. historical 12-language documents only for provenance
```

If the current code and an older document disagree, preserve the old document as historical evidence and create a delta; do not silently rewrite history.

## 12. Epistemic state

```text
23 profile declarations                         = OBSERVED_IN_SOURCE
runtime router with 6 backend IDs               = OBSERVED_IN_SOURCE
dispatch-bus naming in benchmark header         = OBSERVED_IN_SOURCE
2-CB DMA circular ring                          = OBSERVED_IN_SOURCE
full semantics for all 23 language profiles     = NOT_CLAIMED
physical runtime for every backend/profile      = TOKEN_VAZIO_BY_TARGET
LowFala phoneme->morpheme->semantics completion = NOT_IMPLEMENTED_HERE
"bus rotates in all directions"                 = METAPHOR_MAPPED_TO_FANOUT_ROUTING
```

## R3

- **F_ok:** current code-first locations are now explicit; three different bus meanings are separated; old 12-language cuts are marked as historical.
- **F_gap:** per-profile runtime evidence, complete grammar/semantic coverage, and hardware evidence for GPU/storage/DMA remain target-specific.
- **F_next:** whenever language/runtime source changes, update this locator by delta and regenerate document-governance outputs instead of treating old counts as current truth.
