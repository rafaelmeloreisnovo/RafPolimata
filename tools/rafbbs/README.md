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
- `rafbbs_context_core.h`: seeded context/path initialization with authorial bounded copies and no memset/snprintf/libc;
- `rafbbs_command_core.h`: deterministic command-outcome policy independent of `system()`/shell/provider execution;
- `rafbbs_manifest_core.h`: deterministic text-manifest rendering into a caller-owned fixed buffer;
- `rafbbs_filepicker_core.h`: static catalog/selection state without hosted string/runtime calls;
- `rafbbs_theme.h`: deterministic status-to-ANSI mapping without hosted headers;
- `rafbbs_log_core.h`: caller-owned deterministic syslog-line composition with authorial 64-bit divmod;
- `rafbbs_format_core.h`: finite typed formatter for text, signed i32 and fixed-width hex32; no stdarg/vsnprintf/general printf grammar;
- `rafbbs_runlog_core.h`: deterministic persisted run-log header/artifact/gap byte rendering, including fixed-width CRC/hash hex, without stdio or filesystem;
- `rafbbs_pipeline_core.h`: pipeline specs/flags and exact-byte lookup with no execution callback;
- `rafbbs_cli_core.h`: deterministic CLI action routing with no hosted terminal/string runtime;
- `rafbbs_tui_core.h`: deterministic single-key TUI routing with no stdio, terminal or execution dependency;
- `rafbbs_baremetal.h`: fixed-buffer output and binary manifest value model;
- `rafbbs_manifest_bin_core.h`: canonical 96-byte little-endian binary-manifest encode/decode;
- `rafbbs_crc32_core.h` and `rafbbs_sha256_core.h`: pure algorithms;
- `rafbbs_crc32.h`, `rafbbs_sha256.h`, `rafbbs_time_posix.h`, `rafbbs_manifest.h`, `rafbbs_manifest_bin.h`, `rafbbs_filepicker.h`, `rafbbs_log.h`, `rafbbs_pipeline.h`, `rafbbs_cli.h` and `rafbbs_tui.h`: hosted/provider adapters only. `rafbbs_log.h` no longer owns persisted run-log formatting through `fprintf` and no longer uses `stdarg/vsnprintf` for live details. Typed wrappers delegate text/i32/hex32 formatting to `rafbbs_format_core.h`; console and FILE remain hosted.

The freestanding build uses `-nostdinc -ffreestanding -fno-builtin -fno-stack-protector` and rejects unresolved symbols in the declared pure objects. A compiler, `nm`, shell or CI runner is a factory/evidence tool for this gate, not a runtime dependency of those objects. Full toolchain self-hosting remains `TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY)`. Physical bare-metal execution remains `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)` until separately evidenced.

Run:

```sh
sh freestanding/tests/verify_authorial_zero_dep.sh
```

The gate enumerates eighteen pure modules plus a caller-owned probe, rejects host/runtime leakage, and compiles the same probe for six OS-neutral ISA targets. It checks the object for unresolved helpers and keeps hosted adapters outside the pure set.

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


## Pipeline spec core freestanding

A topologia semântica do pipeline foi separada da autoridade de execução.
`rafbbs_pipeline_core.h` contém apenas `RafPipelineSpec`, flags, comparação
exact-byte e lookup por índice. Ele não inclui `RafContext`, não conhece
`RafStatus` e não armazena function pointer.

`rafbbs_pipeline.h` permanece hosted e é o único responsável por vincular
spec→handler e executar comandos/arquivos/providers. O exact-head successor
também inclui o log-core e o pipeline-core no mesmo probe 6-ISA para impedir
que um módulo declarado puro seja apenas listado sem ser compilado nos targets.
Execução física continua `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.


## CLI route core freestanding

A decisão de rota do CLI foi separada da execução hosted. `rafbbs_cli_core.h`
recebe somente `argc/argv` do caller e retorna uma ação tipada:
HELP/LIST/RUN/LOGS/MANIFEST/FILES/INVALID. O argumento de RUN é apenas uma
referência caller-owned; o core não imprime, não aloca, não lê relógio, não
abre arquivo e não executa comando.

`rafbbs_cli.h` continua sendo o adapter POSIX para terminal, hora civil,
filesystem, observação git e `system()`. O parser puro foi falsificado localmente
em x86_64, i686, ARMv7-A, AArch64, RV32 e RV64 com `unresolved=0`.
Execução física permanece `TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.

## TUI route core freestanding

A decisão de tecla da TUI foi separada da apresentação hosted. `rafbbs_tui_core.h`
mapeia `Q/L/F/1/2/3/default` para uma ação tipada e, quando aplicável, para um
pipeline estático. Entrada por `fgets`, desenho Unicode/stdio, file picker
apresentado ao usuário e execução de pipeline continuam exclusivamente em
`rafbbs_tui.h`.

O default preserva o comportamento anterior: ENTER, tecla desconhecida ou `1`
roteiam para `encoders`; `2` para `roundtrip`; `3` para `apkc_validate`.
O falsificador compila sob `-nostdinc` e o probe 6-ISA rejeita símbolos externos.
Isso não torna terminal/console freestanding e não promove execução física.

## Seeded context core freestanding

A inicialização determinística de `RafContext` saiu do adapter POSIX.
`rafbbs_context_core.h` recebe observações já prontas do caller
(`run_id/pipeline/host/arch/branch/commit/start`) e monta estado, watchdog e
caminhos `run/manifest/bin` com cópia limitada autoral.

`rafbbs_cli.h` continua responsável por observar hora civil, relógio monotônico,
arquitetura do host, `git` e filesystem. Portanto
`CONTEXT_SEMANTICS != CLOCK/GIT/FILESYSTEM_PROVIDER`.
O falsificador cobre campos exatos, caminhos, estado zerado e truncation flag;
o probe 6-ISA continua rejeitando helpers externos. Isso não reivindica relógio,
git ou filesystem freestanding.

## Command-result policy core freestanding

A decisão de estado após tentativa de comando foi separada da execução hosted.
`rafbbs_command_core.h` recebe somente três fatos do caller:
`executed`, `rc` e `optional`. O resultado é uma decisão tipada
`status/limited/failed`: ausência de execução → `TOKEN_VAZIO`; rc=0 →
`PASS`; falha opcional → `SKIP`; falha obrigatória → `FAIL`.

`rafbbs_pipeline.h` continua dono de watchdog/rollback operacional, logging e
`raf_host_exec/system()`. Cópias estáticas de command/input/gaps passaram a usar
a cópia limitada autoral já provada no context core, removendo `snprintf("%s")`
dessas rotas. Portanto `COMMAND_POLICY != COMMAND_EXECUTION`.



## Persisted run-log byte core freestanding

The canonical persisted run-log text layout is now split from filesystem I/O.
`rafbbs_runlog_core.h` renders the header, status, artifact fields, fixed-width
lowercase CRC/hash state and gap section into caller-owned memory. Missing gaps
are represented explicitly as `none=TOKEN_VAZIO`; buffer exhaustion increments
the existing dropped counter and fails closed at the hosted persistence boundary.

`rafbbs_log.h` keeps only host concerns for this path: opening/closing the FILE
and writing already-rendered bytes. It no longer uses `fprintf` to define the
persisted format. Live detail formatting is now closed over typed authorial text/i32/hex32 helpers; console I/O
and the POSIX monotonic observation remain hosted adapters and are not promoted
to freestanding. The pure renderer is compiled under `-nostdinc` and exercised
in the six-ISA no-undefined-symbol probe. Physical/device persistence remains
`TOKEN_VAZIO (CLOSURE_L12_DEVICE_RUNTIME_EVIDENCE)`.


## Typed live-log formatter freestanding

The remaining `stdarg/vsnprintf` dependency in RafBBS logging was removed instead
of reimplementing general `printf`. The observed grammar was finite: literal
text, one string, one signed integer, or one fixed-width 32-bit hexadecimal
value. `rafbbs_format_core.h` implements exactly those typed operations over
caller-owned `RafLogText` buffers.

Hosted call sites now use `raf_log`, `raf_log_s`, `raf_log_i32` and
`raf_log_hex32`. This preserves the emitted text while making unsupported
format grammar unrepresentable at the API boundary. The signed formatter handles
the minimum 32-bit value without signed overflow and reuses the authorial bitwise
divmod path, avoiding hidden divide helpers on 32-bit targets.

This removes `<stdarg.h>` and `vsnprintf` from `rafbbs_log.h`. Console output,
POSIX time observation and FILE persistence remain explicit hosted adapters.
