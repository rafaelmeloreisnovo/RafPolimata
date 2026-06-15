# RafPolimata — Context for AI Agents

## What this project is

Three integrated layers:

1. **`raf_compile.h` / RafCtx** — high-level pipeline: CPU detection, flag matrix,
   language dispatch (`RAF_LANG_*`), audit manifest. Entry: `raf_compile_file()`.

2. **`Apkc/`** — bare-metal APK compiler: freestanding, no heap, no libc.
   12-language table-driven pipeline. Entry: `apkc_main()`.

3. **`rafaelia/verbovivo.c`** — cognitive convergence engine (two layers):
   - Layer 1: Fiber-H (256-bit Hamming hash) + Trinity Core (HDC hypervectors,
     synaptic attention, engram ring buffer) via `vv_scan`/`vv_audit`/`vv_svg`
   - Layer 2: T^7 toroid (7-dim, 42 attractors, phi_ethica) → 1024-dim HDC
     XOR-cyclic expansion → SVG engram via `verbovivo_main(apk_path, svg_out)`

## Invariants — never break these

- No `malloc`/`calloc`/`free` anywhere in `Apkc/` or hot paths (freestanding)
- No libc includes in `Apkc/` (syscalls via `sys.h` `svc`/`swi` only)
- Adding a language = 1 row in `_lang_table[]` (`Apkc/lang_profile.h`), zero else
- Adding an instruction = 1 inline in `arch_arm64.h` + 1 case in `asm_insn64()`
- All CI gates must pass (15+ steps in `.github/workflows/ci.yml`)
- `lang_profile_find()` returns NULL for unknown names — callers must guard

## Extension protocol

- **Add a language**: 1 row in `Apkc/lang_profile.h → _lang_table[]`; set `use_asm`/`use_script`/`use_fork` flags; set `use_d8=1` if output needs DEX conversion; set `jsx_node=1` for two-stage Babel→Node bootstrap
- **Add an ARM64 instruction**: 1 `static inline u32` in `Apkc/arch_arm64.h` + 1 `case` in `asm_insn64()` in `Apkc/apkc.c`
- **Add an AXML element**: extend `Apkc/fmt_axml.h` with new SI_ constant and emitter; pass arrays through `axml_build()`
- **Add ELF section**: extend `Apkc/fmt_elf.h`; update section index cross-references (sh_link, e_shstrndx)

## Canonical states

Fields or comments marked:
- `VOID` = placeholder, not implemented
- `PENDING` = in progress
- `AUDIT` = needs verification against spec
- `RUNTIME` = only known at runtime (e.g., IP address, file size)
- `REFERENCE` = external spec (RFC/ISO/ARM ISA/Android ABI)

## Key files

| File | Role |
|------|------|
| `Apkc/apkc.c` | Main compiler (single TU, ~1300 lines) |
| `Apkc/lang_profile.h` | 12-language dispatch table — the single source of truth |
| `Apkc/lang_script.h` | ARM64 execve bootstrap (18-instruction gen_script_code64) |
| `Apkc/arch_arm64.h` | ARM64 encoders: NEON/FMA/widening/PMULL/SDOT/scatter/scalar-FP (~65% ISA coverage) |
| `Apkc/coherence.h` | Freestanding phi_fst() — Q16 KAM-7 coherence metric + phi_attractor() |
| `Apkc/fmt_elf.h` | ELF64/ELF32 builder: flexible symbols, .rodata, .ARM.attributes |
| `Apkc/fmt_axml.h` | Binary AXML: manifest, uses-permission, uses-feature |
| `Apkc/fmt_zip.h` | ZIP/APK writer with HW CRC32 fallback |
| `Apkc/sys.h` | Freestanding syscalls: read/write/open/fork/execve/waitpid |
| `raf_compile.h` | RafCtx struct + CPU/language detection + APKc bridge |
| `rafaelia/verbovivo.c` | Convergence engine: Fiber-H + T^7 toroid + SVG engram |
| `rafaelia/verbovivo.h` | VerbVivoState, VVHyperVec, FiberHash, VVEngram types |
| `docs/MULTI_AI_METHODOLOGY.md` | How AI systems should collaborate on this repo |
| `docs/IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md` | AI-human protocol |
| `docs/MAPA_ESTRUTURAL_REPOSITORIO.md` | Canonical repo map |

## Dispatch pipeline (technological determinism)

```
source.py  →  lang_profile_from_ext(".py")  →  use_script=1
           →  gen_script_code64("/usr/bin/python3", "-c", src_text)
           →  elf64_build_so(.text=bootstrap, syms[2])
           →  zip_add("lib/arm64-v8a/libmain.so") → out.apk

source.kt  →  lang_profile_from_ext(".kt")  →  use_fork=1, use_d8=1
           →  fork_exec_wait("kotlinc", ...) → /tmp/apkc_out.so
           →  fork_exec_wait("d8", ...) → classes.dex
           →  zip_add("classes.dex") → out.apk

source.jsx →  lang_profile_from_ext(".jsx") →  use_fork=1, jsx_node=1
           →  fork_exec_wait("npx", ["babel",...]) → /tmp/jsx_out.js
           →  gen_script_code64("/usr/bin/node", "-e", js_text)
           →  elf64_build_so(.text=bootstrap) → out.apk
```

## CI

```bash
# Freestanding syntax check (must pass clean):
clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
  -ffreestanding -I Apkc Apkc/apkc.c

# Full CI: see .github/workflows/ci.yml (15+ gates)
```

## Buffer limits (static, stack-allocated)

| Buffer | Size | Purpose |
|--------|------|---------|
| `_code64` | 64K | ARM64 assembled code |
| `_so64_buf` | 32K | ELF64 .so output |
| `_fork_out` | 1M | fork+exec compiler output |
| `_dex_buf` | 200B | Minimal classes.dex |
| dynstr stack | 512B | ELF dynamic symbol names |
| elfhash stack | 64B | ELF hash table (nbucket=1) |
| AXML sv[] | 50 entries | String pool for AXML builder |

## See also

- `Apkc/PROTOCOL.md` — fast onboarding for the APKc subsystem
- `docs/MULTI_AI_METHODOLOGY.md` — multi-AI collaboration methodology

## verbovivo build

```bash
# T^7 toroid pipeline (APK/ELF → SVG engram):
gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN rafaelia/verbovivo.c -lm -o verbovivo
./verbovivo out.apk engram.svg

# Fiber-H pipeline (stdin → audit + SVG):
./verbovivo -s < out.apk > graph.svg
```

## Geometric coherence invariant

After every `build_apk()`, the compiler prints:
```
[phi=0.3142 attractor=17]
```
`phi_fst` (from `Apkc/coherence.h`) = (1 − H_norm) × C_norm in Q16 fixed-point.
- H_norm = unique_byte_count / 256  (entropy proxy)
- C_norm = KAM-7 dot product / ‖freq‖  (coherence vs seed {40503…})
- attractor = (phi ⊕ (phi >> 7)) % 42  (maps to T^7 attractor slot)
