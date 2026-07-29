# RafBBS Operator Console — Diretório Operacional do RafPolimata

**Estado:** `ACTIVE`  
**Proprietário lógico:** `automation-maintainer`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · tools`](../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)

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
