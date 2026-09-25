# PKC — freestanding wordcode layer (V1)

Status: **CANDIDATE / claim_allowed=false**\n\nGovernance: `CLOSURE_L11` (operational/language gaps) + `CLOSURE_L12` (legal terms).

This directory is a new append-only layer beside `Apkc/`. The expansion of the
initials "PKC" is intentionally **TOKEN_VAZIO_OWNER_NAMING** until the owner
records the canonical name.

## Goal

A tiny language surface that can be written as words and lowered without libc,
libm, heap, syscalls, external runtime or third-party libraries:

```text
MOVER R0 7
MOVER R1 3
SOMAR R2 R0 R1
RETORNAR
```

and the same semantics in English:

```text
MOVE R0 7
MOVE R1 3
ADD R2 R0 R1
RETURN
```

The grammar is deliberately small:

```text
statement := VERB NOUN*
NOUN      := register | integer
```

No adjective category exists in V1. Punctuation is not needed.

## Invariants

- no libc / libm / heap / syscall in PKC core;
- caller-owned buffers only;
- no external runtime;
- no tail-call requirement in the strict build contract;
- shadowing is a compile-time error in the strict build profile;
- zero undefined symbols is a release gate, not an assumption;
- PT/EN lexical aliases lower to the **same operation IDs**;
- source words are recognized by fixed 32-bit hashes so the core need not retain
  the human words as binary string symbols;
- comments/documentation preserve the readable vocabulary;
- a missing contract capability is data: warning + missing-bit mask;
- TOKEN_VAZIO is never converted to zero.

## Language status

| Lexicon | State |
|---|---|
| Portuguese ASCII verbs | IMPLEMENTED_BASELINE |
| English ASCII verbs | IMPLEMENTED_BASELINE |
| Greek | TOKEN_VAZIO_CORPUS_VALIDATION |
| Hebrew | TOKEN_VAZIO_CORPUS_VALIDATION |
| Syriac/Aramaic | TOKEN_VAZIO_CORPUS_VALIDATION |
| accents / Unicode normalization | TOKEN_VAZIO_UTF8_NORMALIZATION |

The repository already contains reference script samples in
`configs/language-matrix.v1.json`. Reference samples are not treated as a
validated programming lexicon.

## Relationship to ApkC

`Pkc/pkc_wordcode.h` emits a small architecture-neutral operation record.
`Apkc/apkc_pkc_bridge.h` lowers that record to the existing
`RafIr32InsnV1` Stage0 path. The existing ApkC parser is not replaced.

```text
human words
   -> PKC wordcode parser
   -> PKC IR
   -> ApkC bridge
   -> RAFIR_TINY_V1
   -> existing ARM32 encoder / ELF builder
```

## Licensing/provenance boundary

Berne is a copyright-protection framework, not the software license text.
See `docs/legal/PKC_USE_POLICY_DRAFT_V1.md`. Until a final owner-approved
license text exists:

```text
LICENSE_STATUS = TOKEN_VAZIO_FINAL_LEGAL_TEXT
COMMERCIAL_CLEAR = false
claim_allowed = false
```
