# Multi-AI Methodology — RafPolimata

## Purpose

This document defines how Claude, Codex/Copilot, ChatGPT, and other AI coding
systems should collaborate on this repository. Any AI can pick up the codebase
and continue without reading 30+ documents.

**Start here**: Read `docs/AGENTES.md`. It consolidates architecture,
invariants, key files, CI gates, non-collision rules, escalation protocol,
and per-session checklist in one document. `CLAUDE.md` at repo root has a
pointer to it with per-session checklist and conflict log references.

---

## Capability Matrix

| Task | Claude | Codex/Copilot | ChatGPT |
|------|--------|---------------|---------|
| Architecture decisions | primary | support | consult |
| Freestanding C (no libc) | primary | ✓ | review |
| ARM64 ISA encoders | primary | validate | reference |
| LangProfile table extension | primary | ✓ | review |
| ELF/AXML binary format | primary | ✓ | reference |
| CI scripts / workflows | ✓ | ✓ | support |
| Documentation (MD) | ✓ | ✓ | ✓ |
| Semantic / philosophical | review | — | primary |
| Code review | ✓ | ✓ | ✓ |

**Primary** = leads the change, owns correctness.  
**Support** = implements under direction.  
**Validate** = verifies against spec, does not originate.  
**Reference** = provides lookup, does not write code.

---

## Handoff Protocol

1. **Commit message format**: `type(scope): what + why` — not just what.
   - ✓ `feat(Apkc): add PHP to _lang_table — completes 12-language OS coverage`
   - ✗ `update lang_profile.h`

2. **PR lifecycle**:
   - Draft PR = work in progress, CI may be red, do not merge
   - Remove draft = all CI green, ready for review
   - Merge to `Main` (capital M) — not `main`

3. **VOID/PENDING signals**: Comments `/* VOID */` or `/* PENDING */` in code are
   explicit signals for the next AI to continue. Do not remove without implementing.

4. **TOKEN_VAZIO** in comments = deliberate uncertainty documented for later
   disambiguation — not an error.

5. **Before any change**: run the freestanding syntax check:
   ```bash
   clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
     -ffreestanding -I Apkc Apkc/apkc.c
   ```
   If this fails, the change is not mergeable.

---

## Non-Collision Rules

These rules prevent two AIs from breaking each other's work:

1. **Never edit `Apkc/*.h` format files without running syntax-check** after every
   edit. One missing semicolon in a freestanding header cascades to 100+ errors.

2. **Never add a language without a row in `_lang_table[]`** in
   `Apkc/lang_profile.h`. The entire pipeline is driven by that table.

3. **Never add an ARM64 instruction encoder** without:
   - The `static inline u32` function in `arch_arm64.h`
   - A corresponding `case` (or `if` block) in `asm_insn64()` in `apkc.c`

4. **Never commit a NULL-dereference path**. `lang_profile_find()` returns NULL
   for unknown names — guard with: `if (!prof) { pr_err(...); return 1; }`

5. **Never change ELF section indices** (sh_link, e_shstrndx, section count)
   without updating ALL cross-references. Current ARM64 ELF section layout:
   ```
   0=null  1=.text  2=.rodata  3=.hash  4=.dynsym  5=.dynstr
   6=.dynamic  7=.shstrtab  8=.ARM.attributes
   e_shstrndx = 7
   ```

6. **Buffer limits are hard**: never write past static buffer sizes. All are
   documented in `CLAUDE.md` under "Buffer limits".

---

## Conflict Resolution

When two AIs propose incompatible changes:

1. Create `docs/DECISION_LOG.md` with both proposals, pros/cons, and a recommended
   resolution.

2. Escalate to human when:
   - Change breaks an invariant from `CLAUDE.md`
   - Architectural scope (new subsystem, major API change)
   - License or security implications

3. If escalation is needed but human is unavailable: commit to a branch named
   `decision/<topic>` with a PR description explaining the conflict. Do not merge.

---

## Canonical Entry Points by Subsystem

| Subsystem | Entry File | Key Function |
|-----------|-----------|-------------|
| APKc pipeline | `Apkc/lang_profile.h` | `lang_profile_from_path()` |
| ARM64 assembly | `Apkc/arch_arm64.h` | `asm_insn64()` in apkc.c |
| ELF generation | `Apkc/fmt_elf.h` | `elf64_build_so()` |
| AXML generation | `Apkc/fmt_axml.h` | `axml_build()` |
| ZIP/APK packing | `Apkc/fmt_zip.h` | `zip_open()` / `zip_add()` |
| Script bootstrap | `Apkc/lang_script.h` | `gen_script_code64()` |
| Syscalls | `Apkc/sys.h` | `os_read()`, `os_fork()`, etc. |
| High-level compile | `raf_compile.h` | `raf_compile_file()` |

---

## Codebase Snapshot (for AI orientation)

```
RafPolimata/
├── CLAUDE.md                    ← START HERE (AI entry point)
├── raf_compile.h                ← high-level RafCtx pipeline
├── Apkc/
│   ├── apkc.c                   ← main compiler (~1300 lines, single TU)
│   ├── lang_profile.h           ← 12-language dispatch table
│   ├── lang_script.h            ← execve bootstrap generator
│   ├── arch_arm64.h             ← ARM64 instruction encoders
│   ├── arch_arm32.h             ← ARM32 (Thumb2) encoders
│   ├── fmt_elf.h                ← ELF64/ELF32 builder
│   ├── fmt_axml.h               ← binary AXML builder
│   ├── fmt_zip.h                ← ZIP/APK writer
│   ├── sys.h                    ← freestanding syscall wrappers
│   └── PROTOCOL.md              ← APKc subsystem quick-start
├── rafaelia/
│   └── verbovivo.c              ← HDC cognitive engine
├── docs/
│   ├── MULTI_AI_METHODOLOGY.md  ← this file
│   ├── MAPA_ESTRUTURAL_REPOSITORIO.md
│   └── IA_AGENTE_HUMANOS_TECNICO_FORMALIDADE.md
└── .github/workflows/ci.yml    ← 15+ CI gates
```
