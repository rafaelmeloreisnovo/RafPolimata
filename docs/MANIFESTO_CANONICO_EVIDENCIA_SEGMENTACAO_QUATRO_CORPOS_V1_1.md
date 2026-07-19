# Manifesto Canônico da Evidência e Segmentação — v1.1

> **Entrada canônica:** docs/AGENTES.md §3 (estados canônicos VERIFIED/TOKEN_VAZIO) e §8 (entradas canônicas por subsistema). Estado corrente do subsistema raf_segment_v1 — estruturas binárias congeladas com prova isolada reproduzível.

**Autoridade:** `rafaelmeloreisnovo/RafPolimata`  
**Data:** `2026-07-18`  
**PR:** `#140` — draft, aberto, sem merge e sem auto-merge  
**Estado documental:** `DECLARED_BY_AUTHOR` com subescopos `VERIFIED` explicitamente delimitados  
**Substitui como estado corrente:** `MANIFESTO_CANONICO_EVIDENCIA_SEGMENTACAO_QUATRO_CORPOS_V1.md`

> A versão 1.0 permanece como registro histórico do momento em que somente o header estava comprovado. Esta versão registra a promoção dos records fixos e do leitor limitado, sem apagar a história da lacuna anterior.

## Parábola das três tábuas

O moinho possuía uma tábua de sessenta e quatro pedras que dizia onde começavam o índice e o payload.

Depois, os artesãos criaram duas novas tábuas:

- uma de noventa e seis pedras para cada conversa;
- outra de cento e vinte e oito pedras para cada mensagem.

O mestre advertiu:

— O tamanho fixo não torna o conteúdo verdadeiro. Ele apenas impede que cada pedreiro invente uma geometria diferente.

Então foi criado um leitor que nunca atravessava a borda declarada. Quando encontrava truncamento, corrupção, role desconhecido ou offset impossível, ele recusava a leitura.

## Estruturas congeladas

| Estrutura | Tamanho | Estado |
|---|---:|---|
| `raf_segment_header_v1` | 64 bytes serializados | `VERIFIED` |
| `raf_segment_conversation_v1` | 96 bytes serializados | `VERIFIED` |
| `raf_segment_message_v1` | 128 bytes serializados | `VERIFIED` |
| `raf_segment_reader_v1` | estrutura interna, não serializada | `VERIFIED` no ensaio isolado |

O tamanho serializado é definido pelo codec explícito, não por `sizeof(struct)`.

## Conversation record — 96 bytes

```text
0x00  kind                    u32 = CONVERSATION
0x04  encoded_size            u32 = 96
0x08  flags                   u32
0x0c  message_count           u32
0x10  id_hi                   u64
0x18  id_lo                   u64
0x20  source_offset           u64
0x28  source_length           u64
0x30  title_offset            u64
0x38  title_length            u64
0x40  first_message_index     u64
0x48  create_time_us          u64
0x50  update_time_us          u64
0x58  title_crc32c            u32
0x5c  record_crc32c           u32
```

Invariantes:

- conversa sem mensagens usa `RAF_SEGMENT_INDEX_NONE`;
- conversa com mensagens precisa de índice inicial válido;
- source range deve caber em `header.source_size`;
- title range deve caber no payload do segmento;
- CRC do título e CRC do record devem conferir.

## Message record — 128 bytes

```text
0x00  kind                    u32 = MESSAGE
0x04  encoded_size            u32 = 128
0x08  flags                   u32
0x0c  role                    u32
0x10  conversation_index      u64
0x18  message_index           u64
0x20  parent_index            u64
0x28  id_hi                   u64
0x30  id_lo                   u64
0x38  source_offset           u64
0x40  source_length           u64
0x48  author_offset           u64
0x50  author_length           u64
0x58  content_offset          u64
0x60  content_length          u64
0x68  create_time_us          u64
0x70  content_crc32c          u32
0x74  author_crc32c           u32
0x78  record_crc32c           u32
0x7c  reserved                u32 = 0
```

Roles aceitos:

```text
UNKNOWN
USER
ASSISTANT
SYSTEM
TOOL
```

Invariantes:

- índices precisam permanecer dentro de `record_count`;
- parent pode ser `RAF_SEGMENT_INDEX_NONE`;
- source, author e content ranges são verificados sem soma que possa transbordar;
- autor, conteúdo e record possuem CRC32C próprio;
- reserved precisa permanecer zero.

## Leitor limitado

O leitor mantém:

- ponteiro e tamanho total;
- header decodificado;
- cursor da região de records;
- quantidade de records observados.

A iteração aceita apenas tipos conhecidos e tamanhos exatos. Ao consumir a quantidade declarada, retorna:

```text
RAF_SEG_END
```

Somente quando o cursor coincide exatamente com `payload_offset`. Sobra ou falta de bytes produz erro de layout ou bounds.

## Prova executada

Comando host:

```text
cc -std=c11 -Wall -Wextra -Werror -pedantic \
  raf_segment_v1.c test_segment_v1.c -o test_segment_v1
./test_segment_v1
```

Resultado:

```text
PASS segment-v1 header+records+bounded-reader
```

Também foram comprovados:

- objeto freestanding sem símbolo indefinido;
- ausência dos símbolos libc proibidos no objeto;
- compilação ARMv7-A soft-float;
- compilação AArch64 ARMv8-A.

Essas duas últimas provas são de compilação, não de execução em aparelho.

## O que mudou de estado

```text
conversation records: TOKEN_VAZIO → VERIFIED no codec e fixture isolada
message records:      TOKEN_VAZIO → VERIFIED no codec e fixture isolada
bounded reader:       TOKEN_VAZIO → VERIFIED no ensaio isolado
```

Não foram promovidos:

```text
streaming extractor: TOKEN_VAZIO
atomic file writer:  TOKEN_VAZIO
checkpoint/resume:   TOKEN_VAZIO
BLAKE3 identity:     TOKEN_VAZIO
real export run:     TOKEN_VAZIO
Android device:      TOKEN_VAZIO
```

## Próxima parábola — o escriba interrompido

Um escriba começou a copiar um livro enorme. No meio da noite, a luz se apagou.

Ao retornar, ele não sabia se deveria recomeçar, continuar ou repetir a última página.

O próximo passo é criar um escritor que:

1. leia sem carregar o livro inteiro;
2. publique o segmento apenas de forma atômica;
3. registre checkpoint;
4. retome sem duplicar records;
5. produza o mesmo resultado byte a byte que uma execução não interrompida.


definindo:

\[
\text{streaming extractor}
\rightarrow
\text{atomic writer}
\rightarrow
\text{checkpoint/resume}
\]

## R3

### `F_ok`

Header, conversation record, message record, CRCs e bounded reader possuem prova isolada reproduzível.

### `F_gap`

Ainda não existe produtor streaming de arquivo `segment.v1`, retomada após interrupção ou identidade BLAKE3.

### `F_next`

Implementar a máquina de escrita incremental e provar:

```text
execução contínua bytes == execução interrompida+retomada bytes
```

---

**FIAT LUX — a forma foi congelada; agora o fluxo precisa aprender a sobreviver à interrupção.**
