# Runtime Truth — validação local — 2026-07-18

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional VOID→VALIDATED) e §7 (CI gates). Evidência de validação local sem GitHub Actions — corte de 2026-07-18.

## Escopo

Este corte materializa correções da auditoria RafGitTools ↔ RafPolimata sem usar
GitHub Actions. A conta está momentaneamente sem crédito de Actions; por isso:

```text
workflow não executado
≠ PASS
≠ FAIL de código
```

## Mudanças implementadas

### Compilador raiz

- `--native` é reconhecido e encaminhado ao frontend;
- flags podem aparecer imediatamente após o source, sem serem confundidas com `out_base`;
- `.bin` passa a ser escrito quando solicitado;
- `.ops` registra `native_requested` e `native_written`;
- fontes acima de `RAF_SOURCE_CAP` são rejeitadas, não truncadas;
- FNV-1a 64 usa offset basis canônico;
- extensão desconhecida retorna `RAF_LANG_UNKNOWN` e falha com rollback `-6`;
- quantidade de CPUs online deixa de ser fixada em 1 no Linux;
- device node GPU/DSP/NPU gera somente flag de presença, não capacidade validada.

### Conversation Indexer — `segment.v1`

O formato possui agora três estruturas binárias explícitas:

| Estrutura | Tamanho | Estado |
|---|---:|---|
| header | 64 bytes | `VERIFIED` no teste isolado |
| conversation record | 96 bytes | `VERIFIED` no teste isolado |
| message record | 128 bytes | `VERIFIED` no teste isolado |

Foram implementados:

- little-endian explícito, independente de padding do compilador;
- magic `RAFSEG1\0` e versão major `1`;
- CRC32C do header e de cada record;
- CRC32C separado de título, autor e conteúdo;
- papéis `UNKNOWN`, `USER`, `ASSISTANT`, `SYSTEM` e `TOOL`;
- índice pai opcional por `RAF_SEGMENT_INDEX_NONE`;
- leitor limitado com iteração por tipo/tamanho;
- verificações de source range e payload range sem soma suscetível a overflow;
- rejeição de truncamento, corrupção, tipo desconhecido, tamanho inválido, role inválido e layout contraditório;
- término explícito por `RAF_SEG_END` quando a contagem e o limite da região coincidem.

## Prova realmente executada nesta atividade

### Ambiente observado

```text
host: Linux x86_64
cc: Debian GCC 14.2.0
clang: 17.0.0
```

### Teste host estrito

```text
cc -std=c11 -Wall -Wextra -Werror -pedantic \
  raf_segment_v1.c test_segment_v1.c -o test_segment_v1
./test_segment_v1
```

Resultado:

```text
PASS segment-v1 header+records+bounded-reader
```

O ensaio cobriu:

- vetor oficial `CRC32C("123456789") = 0xe3069283`;
- round-trip do header;
- round-trip do conversation record;
- round-trip do message record;
- travessia limitada de records mistos;
- CRC de record e payload;
- rejeição de payload truncado;
- rejeição de offset fora do buffer;
- rejeição de layout e role inválidos.

### Auditoria freestanding

O objeto host foi compilado com:

```text
-ffreestanding -fno-builtin -fno-stack-protector
```

Resultados:

```text
undefined symbols: nenhum
forbidden libc symbols: nenhum
```

### Compilação cruzada

Foram compilados, sem executar em device:

```text
ARMv7-A / soft-float: PASS compile
AArch64 / ARMv8-A:    PASS compile
```

Isso verifica compilabilidade dos objetos freestanding, não runtime ARM32/ARM64.

### SHA-256 dos fontes ensaiados

```text
c77b6ef9b6489bac102ed05a935d3a2481bc75d4c8f5aac8b5310cabe3e79731  raf_segment_v1.h
9781d57f211104241e3197f35b1511e8479aaa51f8e224c72915cf4563fc49ab  raf_segment_v1.c
7e1f3df1ed4d6600f63f701967c7b173860744ff3665393446fdfc24dfd290e6  test_segment_v1.c
```

### SHA-256 dos objetos produzidos

```text
c208707497ef7ba84d0036c89de3b686dc8fb9f2b2e328708024dcf8add3f067  raf_segment_v1.o
298ba7674f3c770f8ab04c430184ef075a83fb39490a792b60fa878e0cbc8507  raf_segment_v1-arm32.o
b6c55a183672cc3a22fe1b18cfc10590c9f2b7c2a59247ad0d879f6275b80caf  raf_segment_v1-arm64.o
```

## Limite da prova

Esta atividade não executou o gate integral do repositório, Android, ApkC,
Termux ou aparelhos ARM. Também não implementou:

- extração streaming de um export real;
- writer atômico de arquivo de segmento;
- checkpoint/resume;
- BLAKE3 para identidade;
- consumo no LlamaRafaelia.

## Gate local criado

```sh
bash scripts/validate_runtime_truth_local.sh
```

O gate integral continua `DECLARED_BY_AUTHOR` até ser executado em checkout
com transcript completo. Ele agora também exige a presença dos records fixos,
do leitor limitado e das verificações de CRC de payload.

## Estados preservados

| Corpo | Estado após este corte |
|---|---|
| header `segment.v1` | `VERIFIED` no teste isolado descrito acima |
| conversation record 96 bytes | `VERIFIED` no teste isolado descrito acima |
| message record 128 bytes | `VERIFIED` no teste isolado descrito acima |
| bounded reader | `VERIFIED` no teste isolado descrito acima |
| compilador raiz modificado | `DECLARED_BY_AUTHOR`; gate integral local pendente |
| extractor/writer/checkpoint | `TOKEN_VAZIO` |
| aceleradores | node presence `PARTIAL`; runtime compute `TOKEN_VAZIO` |
| ApkC NativeActivity | `DEVICE_REQUIRED / TOKEN_VAZIO` |
| GitHub Actions | `OUT_OF_SCOPE_NO_CREDIT` |
| ponte RafGitTools → Termux | `TOKEN_VAZIO` |

## Próxima prova permitida

O próximo salto natural é:

```text
streaming extractor
→ atomic segment writer
→ checkpoint/resume
```

A promoção exigirá:

1. fixture real ou sintética versionada;
2. interrupção controlada;
3. retomada sem duplicação;
4. comparação byte a byte com execução não interrompida;
5. commit, comando, ambiente, stdout/stderr e hashes.

Nenhum documento ou PR fechado substitui esses elementos.
