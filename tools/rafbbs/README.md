# RafBBS Operator Console — Diretório Operacional do RafPolimata

**Estado:** `ACTIVE`  
**Proprietário lógico:** `automation-maintainer`  
**Repositório:** [`rafaelmeloreisnovo/RafPolimata`](https://github.com/rafaelmeloreisnovo/RafPolimata) — `tools/rafbbs/README.md`

O `tools/rafbbs/` é a camada operacional humana e automatizável do RafPolimata. Ele não substitui o núcleo técnico: organiza comandos reais em uma rotina simples, guiada, observável e comprovável.

Princípio: GUI/BBS para operar; CLI para reutilizar; Syslog para entender; log TXT para registrar; manifesto para provar; `TOKEN_VAZIO` para não mentir.

## Build

```sh
sh tools/rafbbs/rafbbs_build.sh
```

## Uso

```sh
tools/rafbbs/rafbbs
tools/rafbbs/rafbbs --help
tools/rafbbs/rafbbs list
tools/rafbbs/rafbbs run encoders
tools/rafbbs/rafbbs run roundtrip
tools/rafbbs/rafbbs run apkc_validate
tools/rafbbs/rafbbs logs
tools/rafbbs/rafbbs manifest
```

Sem argumentos, o programa abre um menu textual BBS/DOS Shell/cyberpunk simples. A GUI chama as mesmas rotinas da CLI para manter reprodutibilidade.

## Estados oficiais

`PASS`, `FAIL`, `STEP`, `INFO`, `WARN`, `AUDIT`, `SKIP`, `RUNTIME`, `REFERENCE`, `PENDING`, `TOKEN_VAZIO`, `HASH`, `DONE` e `PASS_LIMITED`.

Estados finais permitidos: `PASS`, `FAIL`, `PASS_LIMITED`, `SKIP`, `AUDIT` e `TOKEN_VAZIO`.

O RafBBS nunca transforma ausência de evidência em `PASS`. Quando algo não roda no host atual, o estado correto é `SKIP`, `AUDIT` ou `TOKEN_VAZIO`. Em host x86, por exemplo, o teste C ARM é marcado como `SKIP` e rotinas Android/logcat como `TOKEN_VAZIO`, produzindo `PASS_LIMITED` quando a parte obrigatória local passou.

## Formato do Syslog

Cada linha segue formato previsível:

```text
[TEMPO] [STATUS] [MÓDULO] [DETALHE]
00:00.001 INFO         boot       RafBBS iniciado
00:00.041 STEP         apkc       compilando Apkc/apkc.c
00:00.302 PASS         apkc       binário temporário criado
00:00.803 HASH         proof      crc32 calculado
```

O Syslog é pedagógico: mostra o pipeline respirando e registra comando, entrada, lacunas, hashes e status final.

## Logs e manifesto

Cada execução grava:

- `tools/rafbbs/logs/run-YYYYMMDD-HHMMSS.txt`
- `tools/rafbbs/logs/manifest-YYYYMMDD-HHMMSS.txt`

O manifesto registra pipeline, data lógica via `run_id`, commit, branch, host, arquitetura, comando interno, entrada, saída, status final, CRC32, tempo total, log associado e lacunas.

## Pipelines iniciais

- `encoders`: chama `python3 tests/test_arm64_encoders.py`; em host ARM também chama `cc -std=c11 -Wall -Wextra -Werror -I Apkc tests/test_arm64_encoders.c -o /tmp/test_arm64_encoders && /tmp/test_arm64_encoders`.
- `roundtrip`: chama `sh tests/test_asm_roundtrip.sh`.
- `apkc_validate`: chama `sh scripts/apkc_validate.sh`.

Também existem registros placeholder honestos para `proof_chain`, `lang_matrix`, `verbovivo` e `export_manifest`, marcados como `TOKEN_VAZIO` até integração real.

## Núcleo freestanding, failsafe e rollback

A camada nova separa primitivas de operação em `rafbbs_freestanding.h`: watchdog por ticks, anel fixo de rollback, flags low-level e contrato explícito `NO_HEAP/NO_GC`. Esse núcleo não chama sistema operacional e pode ser compilado como objeto `-ffreestanding -fno-builtin` para validar compatibilidade bare-metal.

O executável POSIX continua existindo apenas como adaptador operacional para chamar scripts reais do repositório. Quando `RAFBBS_FREESTANDING_MODE` é definido, comandos externos não são executados e viram `TOKEN_VAZIO`, preservando honestidade de prova.

## File picker mínimo

`rafbbs files` e a opção `F` na TUI mostram entradas conhecidas em tabela estática, sem varredura dinâmica nem heap. A fase seguinte pode trocar essa lista por uma tabela gerada em build-time.

## Authorial zero-dependency V1

The pure RafBBS slice is now structurally separated from hosted I/O:

- `rafbbs_types.h`: zero-include authorial scalar types;
- `rafbbs_status.h`: status vocabulary without hosted headers;
- `rafbbs_time.h`: monotonic timestamp/elapsed arithmetic with explicit validity;
- `rafbbs_freestanding.h`: watchdog/rollback/flags;
- `rafbbs_core.h`: caller-owned RafContext using `RafMonoTime`, not `struct timespec`;
- `rafbbs_manifest_core.h`: deterministic text-manifest rendering into a caller-owned fixed buffer;
- `rafbbs_filepicker_core.h`: static catalog/selection state without hosted string/runtime calls;
- `rafbbs_theme.h`: deterministic status-to-ANSI mapping without hosted headers;
- `rafbbs_log_core.h`: caller-owned deterministic syslog-line composition with authorial 64-bit divmod;
- `rafbbs_baremetal.h`: fixed-buffer output and binary manifest value model;
- `rafbbs_manifest_bin_core.h`: canonical 96-byte little-endian binary-manifest encode/decode;
- `rafbbs_crc32_core.h` and `rafbbs_sha256_core.h`: pure algorithms;
- `rafbbs_crc32.h`, `rafbbs_sha256.h`, `rafbbs_time_posix.h`, `rafbbs_manifest.h`, `rafbbs_manifest_bin.h`, `rafbbs_filepicker.h` and `rafbbs_log.h`: hosted/provider adapters only.

The freestanding build uses `-nostdinc -ffreestanding -fno-builtin -fno-stack-protector` and rejects unresolved symbols in the declared pure objects. A compiler, `nm`, shell or CI runner is a factory/evidence tool for this gate, not a runtime dependency of those objects. Full toolchain self-hosting remains `TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)`. Physical bare-metal execution remains `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)` until separately evidenced.

Run:

```sh
sh freestanding/tests/verify_authorial_zero_dep.sh
```

The gate enumerates thirteen pure modules plus a caller-owned probe, rejects host/runtime leakage, and compiles the same probe for six OS-neutral ISA targets. It checks the object for unresolved helpers and keeps hosted adapters outside the pure set.

The monotonic-time boundary is intentionally split: `rafbbs_time.h` owns only
representation, validity and elapsed arithmetic; `rafbbs_time_posix.h` owns
`clock_gettime(CLOCK_MONOTONIC)`. The freestanding build also compiles
`rafbbs_time_core_test.c` under `-nostdinc` and executes its arithmetic on the
CI host. This is semantic host execution only; physical ARM/device timing remains
`TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.

The text-manifest boundary follows the same rule: `rafbbs_manifest_core.h`
owns deterministic byte rendering, lowercase fixed-width CRC/hash formatting,
explicit invalid elapsed output and overflow accounting. `rafbbs_manifest.h`
owns text `FILE`/filesystem persistence.

The binary-manifest boundary is equally explicit: `rafbbs_manifest_bin_core.h`
owns the canonical 96-byte little-endian wire image and roundtrip validation;
`rafbbs_manifest_bin.h` only persists already-encoded bytes. This removes native
struct-layout/endianness dependence while preserving the historical little-endian
byte representation used on project ARM/x86 hosts. Codec PASS does not claim a
freestanding filesystem or provider.

## SHA256 autoral

Além de CRC32, o RafBBS calcula SHA256 por implementação local sem dependência externa para entradas conhecidas. SHA256 não remove `TOKEN_VAZIO`: ele só assina evidência existente.

## Testes operacionais

```sh
sh tools/rafbbs/rafbbs_test.sh
```

O teste cobre build, help, listagem, file picker, TUI, watchdog, rollback, SHA256 conhecido e compilação do núcleo freestanding.

## Entrega dos 10 passos enterprise/bare-metal

Os 10 passos solicitados foram materializados em `ENTERPRISE_BAREMETAL.md` e em módulos de código:

1. separação `rafbbs_host.h`/`rafbbs_baremetal.h`;
2. saída byte-a-byte por buffer fixo;
3. failover SHA256 → CRC32 → `TOKEN_VAZIO`;
4. file picker por tabela estática;
5. `proof_chain` real com `AUDIT/PASS_LIMITED`;
6. watchdog preventivo;
7. rollback paliativo;
8. manifesto binário compacto;
9. flags por arquitetura;
10. testes failsafe/failover/freestanding/no-heap.

Use:

```sh
sh tools/rafbbs/rafbbs_build.sh host
sh tools/rafbbs/rafbbs_build.sh freestanding
sh tools/rafbbs/rafbbs_test.sh
```

## Próximo ciclo recorrente implementado

Este ciclo adiciona manifesto binário gravável, callback byte-a-byte para saída bare-metal, tabela constante de flags por arquitetura, alvo `commandless`, fixture binária, teste de overflow do buffer, `hash_state` em log/manifest e reforço do failover `SHA256 → CRC32 → TOKEN_VAZIO`.


## Filepicker core freestanding

A seleção do catálogo conhecido foi separada da apresentação hosted:

- `rafbbs_filepicker_core.h`: catálogo estático e índice selecionado, sem libc/string/heap/syscall;
- `rafbbs_filepicker.h`: somente adaptação de apresentação com `printf`.

O core preserva a seleção atual quando recebe escolha fora de `1..5`. O gate autoral compila esse core com `-nostdinc` nas seis ISAs declaradas e rejeita símbolos externos. Isso não promove execução física: `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.


## Theme/status rendering freestanding

`rafbbs_theme.h` já era estruturalmente autoral e não precisava de reescrita.
Este successor apenas o inclui no conjunto provado: `-nostdinc`, seis ISAs,
unresolved helpers = 0 e falsificador de mapeamento status→ANSI. Console I/O
continua fora do core e execução física continua `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.


## Log-line core freestanding

A composição determinística da linha de syslog agora é autoral e independente do host.
`rafbbs_log_core.h` recebe tempo/status/módulo/detalhe e escreve em buffer do caller;
`rafbbs_log.h` permanece adapter para relógio POSIX, `va_list/vsnprintf`, console e FILE.

O primeiro protótipo foi falsificado em i686 porque divisão C de 64 bits produziu
`__udivdi3`. Esse protótipo foi rejeitado. O successor usa divmod bit-a-bit autoral;
o falsificador local observou `unresolved=0` em x86_64, i686, ARMv7-A, AArch64,
RV32 e RV64. Execução física continua `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.
