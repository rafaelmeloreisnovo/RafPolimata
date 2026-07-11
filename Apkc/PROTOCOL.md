# APKc Subsystem — Quick-Start Protocol

For full context read `CLAUDE.md` at the repo root first — it points to
`docs/AGENTES.md`, the unified session guide (invariants, non-collision rules,
CI gates, startup/shutdown checklist). This document then covers the APKc
subsystem specifically — everything needed to continue work without reading
the entire ~1300-line `apkc.c`.

## What APKc does

Takes a source file (`.c`, `.rs`, `.kt`, `.py`, `.jsx`, etc.) and produces an
Android APK. Completely freestanding: no malloc, no libc, no heap. All memory
is static or stack.

Entry point: `apkc_main(argc, argv)` in `apkc.c`.

---

## 3 Dispatch Modes

Every language maps to exactly one mode via the `_lang_table[]` in
`lang_profile.h`:

### 1. `use_asm = 1` — Internal assembler (`.s` only)
The source is fed to the internal 2-pass ARM assembler (`assemble()`).
No external tools needed. Produces both ARM64 and ARM32 `.so`.

### 2. `use_script = 1` — Execve bootstrap (py/sh/perl/js/php)
Source text is embedded **inline** in the ELF `.text` section via
`gen_script_code64(compiler, arg1, src_text, code_buf, cap)`.

The generated ARM64 bootstrap (18 instructions) does:
```
sub sp, sp, #48       ; allocate stack frame
adr x0, interp_str   ; /usr/bin/python3 (etc.)
adr x1, argv_ptr      ; argv = [interp, arg1, src_ptr, NULL]
mov x2, xzr           ; envp = NULL
mov x8, #221           ; NR_execve
svc #0
; execve failure fallback:
add sp, sp, #48       ; restore stack  ← critical, added in fix
mov x0, xzr
ret
```
Pool layout (after 18 instructions = 72 bytes):
- `interp_str` — null-terminated interpreter path
- `arg1_str` — e.g. `"-c"` or `"-e"`
- `src_text` — the actual script source

No fork. No file I/O. No libc. The APK runs `execve()` on first launch.

### 3. `use_fork = 1` — External compiler (c/cpp/rs/kt/java/jsx)
`fork_exec_wait(compiler, args, outfile, buf, cap)` in `apkc.c`:
1. `os_fork()` → clone(SIGCHLD=17) NR=220
2. Child: `os_execve(compiler, args, NULL)` NR=221
3. Parent: `os_waitpid(pid, &st, 0)` NR=260 → reads `outfile` into `buf`

Post-processing flags:
- `dex_output=1` (Kotlin/Java): output stored in `_dex_buf` as `classes.dex`
- `use_d8=1` (Kotlin/Java): a second fork runs `d8` on the JAR to get real DEX
- `jsx_node=1` (JSX): output of babel (`/tmp/jsx_out.js`) feeds into
  `gen_script_code64("/usr/bin/node", "-e", ...)` for the final bootstrap

---

## Lexer / Token Cycle (assembler mode only)

```
src bytes → Lex() → tokens → parse loop:
  case TOK_MNEM:
    insn = asm_insn64(mnem_str) or asm_insn32(mnem_str)
    → emits u32 to code_buf[pass][off]
  case TOK_LABEL:
    pass 0: record label_tbl[i] = current_offset
    pass 1: emit opcode with label_tbl[i] resolved
  case TOK_DIR:
    .word / .byte / .ascii / .section / .global → handled inline
```

Pass 0: count bytes, record label offsets.
Pass 1: emit instructions and back-patch label references (B, BL, LDR-lit, CBZ, etc.).

---

## NEON Register Parsing

`reg_neon(tok)` → returns register number 0..31.

Recognizes:
- `v0`..`v31` — 128-bit vector registers
- `q0`..`q31` — alias for v0..v31 (same encoding)

Arrangement specifiers (`.16b`, `.4s`, `.2d`, etc.) are parsed inline in
`asm_insn64()` after the register token.

---

## Backpatch Labels

```c
/* pass 0 */
if (tok == TOK_LABEL) label_tbl[nlabels++] = { name, cur_off };

/* pass 1, on branch instruction */
i32 delta = label_tbl[i].off - cur_off;
insn = a64_b(delta >> 2);   /* or a64_bl, a64_cbz, etc. */
```

Labels cannot be forward-referenced in ARM32 mode (single-pass for Thumb2).
ARM64 mode is always 2-pass.

---

## Static Buffer Limits

| Symbol | Size | Holds |
|--------|------|-------|
| `_code64[0x10000]` | 64K | ARM64 assembled code |
| `_code32[0x10000]` | 64K | ARM32 assembled code |
| `_so64_buf[0x8000]` | 32K | ELF64 .so output |
| `_so32_buf[0x8000]` | 32K | ELF32 .so output |
| `_fork_out[0x100000]` | 1M | fork+exec compiler output |
| `_dex_buf[200]` | 200B | Minimal stub classes.dex |
| `_axml_buf[4096]` | 4K | Binary AXML manifest |
| dynstr (stack) | 512B | ELF dynamic symbol names (max 8 symbols) |

Exceeding any of these silently truncates — no allocation, no realloc.

---

## ELF Section Layout (ARM64)

```
Section  Index  Type               Flags  Content
──────────────────────────────────────────────────────
(null)     0    SHT_NULL           —      —
.text      1    SHT_PROGBITS       AX     ARM64 code
.rodata    2    SHT_PROGBITS       A      read-only data (optional)
.hash      3    SHT_HASH           A      symbol hash (nbucket=1)
.dynsym    4    SHT_DYNSYM         A      ElfSym entries
.dynstr    5    SHT_STRTAB         A      symbol name strings
.dynamic   6    SHT_DYNAMIC        AW     DT_NEEDED/DT_SONAME/DT_NULL
.shstrtab  7    SHT_STRTAB         —      section name strings
.ARM.attr  8    SHT_ARM_ATTRIBUTES —      ABI tags: ARMv8-A, VFPv4, NEONv2

e_shstrndx = 7
.hash   sh_link = 4  (.dynsym)
.dynsym sh_link = 5  (.dynstr)
.dynamic sh_link = 5 (.dynstr)
```

**CRITICAL**: if you add or remove a section, update ALL sh_link values and
`e_shstrndx` manually. The builder in `fmt_elf.h` does not auto-resolve indices.

---

## AXML String Pool

Static pool has 34 entries (SI_COUNT_EX=34):

```
SI_PKG=0  SI_APP=1  SI_ACT=2  SI_INT=3  SI_CAT=4  SI_SDK=5
SI_MAN=6  SI_VER=7  SI_PKG_S=8 ... SI_EL_PERM=30  SI_EL_FEAT=31
SI_REQUIRED=32  SI_TRUE=33
```

Dynamic strings (permission/feature names passed at runtime) are appended as
entries 34..N using local stack arrays `sv[50]` and `str_off[50]`.

`_ax_resid[33]` maps string pool indices to Android resource IDs. Entry 32
(SI_REQUIRED) = `0x0101021E`. Entry 33 (SI_TRUE) has no resID (it's a value).

---

## Common Mistakes

1. **Forgetting `add sp, sp, #48` before ret in bootstrap**: execve failure path
   must restore the stack frame or the parent call will see corrupt SP.

2. **lang_profile_find() returning NULL**: always guard:
   ```c
   if (!prof) { pr_err("unknown -lang value\n"); return 1; }
   ```

3. **Incorrect ELF sh_link after adding a section**: count sections, reassign
   cross-references. Easy to miss `.hash sh_link` after inserting `.rodata`.

4. **AXML resource map off by 1**: `_ax_resid[]` must have `AX_RESID_COUNT`
   entries. Adding a new attribute with a resID requires extending both the
   `SI_*` constant and `_ax_resid[]`.

5. **cc_args order for clang**: `-o <outfile>` must come BEFORE the source file.
   The table has `-o` as the last entry in `cc_args`; `apkc.c` appends outfile
   then source when building the exec args array.
