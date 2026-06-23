# RAF TraceBuf C/ASM/HEX

Modulo autoral para codificar eventos do proprio processo em buffer fornecido pelo chamador.

## Escopo

- C puro, header-only.
- Sem alocacao dinamica.
- Sem printf/string helpers externos.
- Sem biblioteca externa.
- Sem transporte externo embutido.
- Sem captura de processos externos.
- Hexadecimal fixo para tag, code, value e tick.
- Inline ASM seguro apenas para barreira de compilador e leitura de tick ARM64 quando disponivel.

## Arquivo agregador

- `Benchmark/raf_tracebuf.h`

## Blocos de especialidade

| Bloco | Arquivo | Especialidade | Criterio de uso |
|---|---|---|---|
| Flags e feature gates | `Benchmark/raf_trace_flags.h` | macros, magic, estados, inline e deteccao ARM64 | incluir em qualquer modulo TraceBuf |
| Buffer | `Benchmark/raf_trace_buffer.h` | buffer do chamador, room check, truncamento explicito | hot path sem heap e sem overflow silencioso |
| HEX | `Benchmark/raf_trace_hex.h` | hexadecimal fixo 32/64-bit | quando precisar saida textual deterministica |
| Evento | `Benchmark/raf_trace_event.h` | struct de evento, barreira, tick ARM64 opcional | quando tag/code/value precisam virar unidade auditavel |
| Emissao | `Benchmark/raf_trace_emit.h` | encode binario e textual | quando converter evento para buffer |
| Sink callback | `Benchmark/raf_trace_sink.h` | entrega buffer para callback do chamador | quando quiser plugar arquivo/stdout/ring buffer sem acoplar transporte ao nucleo |
| Agregador | `Benchmark/raf_tracebuf.h` | inclui todos os blocos | compatibilidade e uso simples |

## Modelo operacional

```text
entrada: tag, code, value
estado: buffer do chamador
saida textual: RAFTRACE t=0x... tag=0x... code=0x... value=0x...
saida binaria: struct raf_trace_event_t
falha: RAF_TRACEBUF_TRUNC quando o buffer nao comporta a saida
sink: callback opcional do chamador recebe bytes ja codificados
```

## Meios preenchidos

| Meio antes faltante | Implementacao atual | Arquivo |
|---|---|---|
| Agregador pequeno | `raf_tracebuf.h` apenas inclui modulos | `Benchmark/raf_tracebuf.h` |
| Especializacao por flags | macros e feature gate ARM64 | `Benchmark/raf_trace_flags.h` |
| Buffer sem heap | room check, truncamento e push byte/mem/cstr | `Benchmark/raf_trace_buffer.h` |
| HEX sem formatter | hex32/hex64 fixos | `Benchmark/raf_trace_hex.h` |
| Evento auditavel | magic/tick/tag/code/value | `Benchmark/raf_trace_event.h` |
| Emissao binaria/textual | encoder para buffer | `Benchmark/raf_trace_emit.h` |
| Sink desacoplado | callback fornecido pelo chamador | `Benchmark/raf_trace_sink.h` |
| Smoke gate | compilacao e execucao em CI | `tools/raf_tracebuf_smoke.c` + `.github/workflows/ci.yml` |

## Flags sugeridas

```bash
gcc -std=c11 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -fno-stack-protector \
  -fvisibility=hidden -Os -I. <arquivo.c>
```

Para leitura de assembly gerado:

```bash
gcc -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin \
  -fno-stack-protector -fvisibility=hidden -Os -I. -S <arquivo.c> -o out.s
```

## Flags de especializacao

| Flag | Uso | Observacao |
|---|---|---|
| `-Os` | reduzir tamanho do hot path | preferivel em Android/Termux restrito |
| `-O2` | medir throughput quando o tamanho nao for gargalo | comparar contra `-Os` antes de promover |
| `-ffreestanding` | evitar pressupostos hosted | mantem disciplina de baixo nivel |
| `-fno-builtin` | nao substituir funcoes por builtins implicitos | reforca controle autoral |
| `-fno-stack-protector` | reduzir prologo/epilogo em microteste | usar apenas em modulo controlado |
| `-fvisibility=hidden` | reduzir superficie exportada | util para `.so` e JNI futuro |
| `-S` | gerar assembly | inspecao de instrucoes e regressao |

## CI gate

```bash
gcc -std=c11 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -fno-stack-protector \
  -fvisibility=hidden -Os -I. \
  tools/raf_tracebuf_smoke.c \
  -o build_raf_tracebuf_smoke
./build_raf_tracebuf_smoke
```

## Criterios de desempenho

1. Hot path sem heap.
2. Buffer controlado pelo chamador.
3. Hexadecimal fixo sem formatador externo.
4. Caminho truncado explicito, sem overflow silencioso.
5. Baseline C antes de qualquer especializacao.
6. Inline ASM somente quando houver ganho ou acesso a registrador especifico.
7. Sink de transporte real deve permanecer modulo separado e revisavel.

## Limite de claim

Este modulo prova codificacao local de evento em buffer e entrega opcional por callback do chamador. Nao prova runtime Android completo, nao prova coleta externa e nao substitui prova de device/runtime.
