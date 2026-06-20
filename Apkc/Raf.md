# Apkc/Raf.md — ApkC Extension Reference

Fast onboarding for extending the ApkC freestanding APK compiler.
See `Apkc/PROTOCOL.md` for full protocol; see `CLAUDE.md` for invariants.

## Extension points

### Add a language (`lang_profile.h`)

One row in `_lang_table[]` — zero changes elsewhere:

```c
{ "go",  ".go",  0,0,1,0,0, "go", (const char*[]){"build","-buildmode=c-shared","-o",TMP_SO,NULL}, TMP_SO }
```

Fields (in order): `name`, `ext`, `use_asm`, `use_script`, `use_fork`,
`use_d8`, `jsx_node`, `cmd`, `argv`, `output_path`.
Set `LP_COUNT` to `LP_COUNT + 1` and add a `#define LP_<LANG> <N>` constant.

### Add an ARM64 instruction (`arch_arm64.h` + `apkc.c`)

1. Add one `static inline uint32_t a64_<mnemonic>(...)` encoder in `arch_arm64.h`.
2. Add one `case TOKEN_<MNEMONIC>:` in `asm_insn64()` in `apkc.c`.
3. Add at least one reference-byte test (compare word against `objdump -d` output).

### Add an AXML element (`fmt_axml.h`)

1. Add a `SI_<ELEMENT>` string-pool constant.
2. Add an emitter call inside `axml_build()`.
3. Pass the new attribute arrays through the function signature.

### Add an ELF section (`fmt_elf.h`)

1. Extend the `ELFBuf`-based builder with the new section header.
2. Update `sh_link` / `e_shstrndx` cross-references.
3. Rerun `apkc_validate.sh` — binary size gate must remain green.

### Add an equivalence family (`codegen_select.h`)

For `N` semantically-identical encodings of the same operation:

1. Verify all N encodings produce the same destination value and flags.
2. Add a `codegen_select(em->buf, em->pos, N)` dispatch call.
3. Log the chosen variant only in the real-emission pass (`_codegen_log_on`).

## Buffer limits (stack-allocated, never exceed)

| Buffer | Size | Purpose |
|--------|------|---------|
| `_code64` | 64 KB | ARM64 assembled code |
| `_so64_buf` | 32 KB | ELF64 `.so` output |
| `_fork_out` | 1 MB | `fork+exec` compiler output |
| `_dex_buf` | 200 B | Minimal `classes.dex` |

## Validation

```bash
bash scripts/apkc_validate.sh          # F0-F10 validation matrix
bash tools/rafbbs/rafbbs.sh proof_chain # chain-of-custody update
```
