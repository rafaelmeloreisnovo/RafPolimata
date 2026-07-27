# RAF Semantic Language Core + Seven Architectures — V2

**Estado:** `IMPLEMENTED_LOCAL_PASS` · `claim_allowed=false` · sem instalação/launch

## 1. Separação canônica

```text
fonte de programação
→ semântica computacional canônica
→ RAF Semantic IR
→ backend selecionado pelo registry de arquiteturas
→ objeto/ELF/APK
→ execução observada
→ receipt
```

A semântica do programa não depende da ISA. Gerar para uma arquitetura não prova execução; executar nativamente não prova virtualização; cross-compilar não prova funcionamento em aparelho.

## 2. Sete arquiteturas ativas

| ID | Arquitetura | Largura | Papel atual |
|---|---|---:|---|
| `aarch64` | AArch64 / ARM64 | 64 | primária Android e servidor |
| `armv7a` | ARMv7-A / AArch32 | 32 | compatibilidade física `armeabi-v7a` |
| `x86_64` | x86-64 / AMD64 | 64 | host e virtualização |
| `riscv64` | RISC-V RV64GC | 64 | pesquisa e expansão |
| `mips64r6el` | MIPS64 Release 6 LE | 64 | pesquisa/compatibilidade |
| `s390x` | IBM Z / s390x | 64 | pesquisa big-endian |
| `loongarch64` | LoongArch64 | 64 | pesquisa e expansão |

`i386`, `IA-32`, `x86-32` e `80386` estão aposentados e são rejeitados quando aparecem na matriz ativa.

## 3. Semântica das palavras

A nova camada lexical separa:

```text
forma superficial
→ normalização Unicode NFC/casefold
→ lema
→ classe gramatical
→ morfemas declarados
→ sentidos numerados
→ domínio e tags semânticas
→ fonologia declarada
→ lexical_id SHA-256
```

A fonologia registra:

- sistema IPA;
- transcrição fonêmica ampla;
- dialeto/escopo;
- sequência de fonemas;
- sílabas;
- índice de tonicidade;
- estado da revisão.

Não existe conversão automática grafema→fonema nesta fase. Ausência de pronúncia declarada produz `TOKEN_VAZIO_PHONEME_NOT_DECLARED`; o sistema não inventa IPA.

## 4. Seed V1

O registry inicial contém seis lexemas em português brasileiro:

```text
luz
prova
estado
vazio
semântica
fonema
```

São sete sentidos controlados e 33 unidades fonêmicas declaradas. As transcrições são amplas e mantêm `TOKEN_VAZIO_HUMAN_PHONETIC_REVIEW_PENDING` para revisão linguística externa.

## 5. Prova local de 2026-07-26

```yaml
unit_tests: 19_PASS
semantic_frontends: 10
semantic_vectors: 4
semantic_equivalence: PASS
freestanding_c_object: PASS
active_architectures: 7
active_i386_occurrences: 0
lexeme_records: 6
sense_records: 7
phoneme_units: 33
automatic_grapheme_to_phoneme: false
install_executed: false
launch_executed: false
claim_allowed: false
```

Execução multi-ISA permanece `TOKEN_VAZIO_RECEIPT_PENDING` até os toolchains/QEMU correspondentes existirem com proveniência e todos os vetores forem observados.

## 6. Próximos gates

1. criar emissores mínimos por arquitetura sem duplicar a semântica;
2. materializar toolchains com versões e hashes;
3. executar os mesmos vetores em cada ISA;
4. ligar ARMv7-A/AArch64 ao ApkC;
5. ampliar a semântica lexical com relações, contexto, polissemia e corpus;
6. adicionar adaptadores fonológicos por idioma somente com regras versionadas e fixtures revisadas.

## 7. Invariantes

```yaml
i386_active: false
architecture_count: 7
semantic_ir_is_isa_independent: true
word_meaning_is_not_program_execution: true
phonemic_broad_is_not_narrow_phonetics: true
missing_phoneme_is_token_vazio: true
generated_is_not_executed: true
claim_allowed: false
```
