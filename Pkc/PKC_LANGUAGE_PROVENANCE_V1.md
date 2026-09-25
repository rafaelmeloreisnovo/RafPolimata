# PKC language provenance V1

Status: **evidence registry / no automatic semantic promotion**  
Governance: `CLOSURE_L11`  
claim_allowed=false

## Correction

The initial PKC V1 work classified Greek, Hebrew and Aramaic as if their source
material still had to be found. That classification was too coarse.

A larger historical LowFala source has now been located:

- repository: `rafaelmeloreisnovo/Vectras-VM-Android`
- ref inspected: `master`
- path: `Incluir/compiladorlowFala.txt`
- Git blob: `c05847c7b05f4657a156c131e2a43322aeec2ec0`
- GitHub object size: `106563` bytes
- observed text length through the connector: `99879` characters
- observed lines: `2280`

The file identifies itself as a FALA -> FONEMA -> TOKEN -> AST -> BYTECODE ->
ASM -> VM -> OUTPUT monolith.

## Directly observed language evidence

The source contains, among other material:

- a `PHONEME_MANIFOLD` with explicit **HEBRAICO** entries;
- a substantial explicit **GREGO** alphabet/phoneme section;
- an explicit **ARAMAICO** section;
- `ar_biblia` / "aramaico bíblico" grammar metadata;
- a low-level C lexer seed described as supporting UTF-8 units for
  "hebraico/grego/aramaico";
- Greek and Hebrew lexical/semantic examples and VM vocabulary;
- morphology material for Hebrew and Greek;
- opcodes including VERB and NOUN.

The monolith itself starts as a Bash generator/container and embeds multiple
seed languages, including C, Python and ASM. It must therefore not be described
as one single pure-C translation unit.

## Relationship to the extracted C core

A separate C-oriented LowFala implementation exists at:

`rafaelmeloreisnovo/ChipQuantum:src/lowfala/`

with tokenizer, AST, bytecode, VM, backend, failsafe and benchmark modules.

At the inspected revision, those C modules do **not** yet expose the same
Greek/Hebrew/Aramaic lexical tables present in the 106563-byte monolith.

Therefore:

```text
SOURCE_PRESENT
    !=
EXTRACTED_C_CORE_LANGUAGE_PARITY
    !=
PKC_EXECUTABLE_LEXICON_PARITY
```

## Current language state

| language | source evidence | extracted-C parity | PKC lowering |
|---|---|---|---|
| Portuguese | present | baseline | implemented baseline |
| English | present | baseline | implemented baseline |
| Hebrew | strong | pending | pending |
| Greek | strong | pending | pending |
| Biblical Aramaic | present but less complete | pending | pending |
| Syriac-script specific coverage | not proven by this inspection | pending | TOKEN_VAZIO |

## Promotion gate

A language moves from source evidence to executable PKC support only when:

```text
source provenance
-> Unicode/code-point contract
-> normalized lexical table
-> VERB/NOUN operation mapping
-> deterministic parser tests
-> PKC IR parity
-> RAFIR parity
-> target instruction byte parity
-> receipt
```

This preserves the LowFala material without turning historical/reference
semantics into an untested compiler claim.
