# Cadeia de custódia — reconciliação de RAF_CHECKLIST_96_ITEMS.md (M0xx)

> **Entrada canônica:** `docs/AGENTES.md` §3 (estados canônicos — PENDING, TOKEN_VAZIO, AUDIT)
> e §4 (disciplina de evidência: `[x]` só onde o corpo da função executa a técnica, nunca por nome).
> Este documento reconcilia os 56 itens M001–M056 contra os arquivos `RAF_0xx_*.c` existentes.

Data: `2026-06-16`.
Escopo: reconciliar os 56 itens M001–M056 de `RAF_CHECKLIST_96_ITEMS.md` contra os 56 arquivos `RAF_0xx_*.c` já existentes no repositório, marcando `[x]` apenas onde o corpo da função executa de fato a técnica descrita no nome do item — não onde apenas o nome/comentário/macro sugere isso.

## Regra de interpretação

Esta passada não inventa PASS. Onde o arquivo existe mas o corpo é um stand-in genérico (placeholder técnico, não a técnica nomeada), o item permanece `[ ]`. Onde não há corpo executável capaz de autovalidação, mas a lógica corresponde à descrição, o item é marcado com nota explícita de `AUDIT`/`COMPILE_OK` em vez de `EXECUTA`.

## Método

1. `ls RAF_0*.c | wc -l` → 56 arquivos confirmados na raiz (não em `methods/` como `RAF_INDEX.md` sugere — divergência de path notada, não corrigida nesta passada).
2. Leitura de cada arquivo candidato a M001–M056.
3. Para grupos suspeitos de corpo repetido: `sed` removendo o token `m0[0-9]+` do nome de função, seguido de `diff`/`md5sum` para comparar corpos normalizados.
4. Para os candidatos genuínos: compilação isolada (`gcc -std=c11 -Wall -Wextra -c ...`) e, quando o arquivo contém uma função de autoverificação com `return 0`/`return -1`, execução via driver mínimo (`main` que chama a função e propaga o `rc`).

## Resultado: 9 itens genuínos confirmados (de 56)

| Item | Arquivo | Evidência | Estado |
|---|---|---|---|
| M001 | `RAF_001_acesso_direto_a_ddrx_portx_pinx.c` | escreve `DDRB`/`PINB` via `RAFA_MMIO8`; gated em `__AVR_ATmega328P__` | AUDIT (compila limpo; não executável neste host x86_64 por falta de alvo AVR) |
| M002 | `RAF_002_toggle_por_escrita_em_pinx.c` | mesmo corpo de toggle DDRB/PINB de M001, semanticamente correto para "toggle por escrita em PINx" | AUDIT (idem M001) |
| M024 | `RAF_024_leitura_de_contador_arm64_cntvct_el0.c` | `mrs %0, cntvct_el0` inline asm, fallback `clock_gettime` | PASS (compilado e executado, rc=0) |
| M048 | `RAF_048_log_binario_em_vez_de_log_textual_pesado.c` | compara `sizeof(_m048_log)` binário vs estimativa textual de 8×24 bytes | PASS (compilado e executado, rc=0) |
| M050 | `RAF_050_exportacao_de_resultado_em_json.c` | conversor decimal manual + buffer estático de 64 bytes, valida chaves `{`/`}` | PASS (compilado e executado, rc=0) |
| M052 | `RAF_052_probe_de_hot_path_no_qemu_tcg.c` | struct de contadores `translated_blocks/executed_blocks/host_cycles/guest_ops` atualizado por chamada | COMPILE_OK (função `void`, sem autoverificação interna; não executado via driver) |
| M054 | `RAF_054_batching_de_operacoes_repetidas.c` | compara `dispatch_count_individual` (64) vs `dispatch_count_batched` (8), assert `batched < individual` | PASS (compilado e executado, rc=0) |
| M055 | `RAF_055_cache_local_de_resultado_tecnico.c` | cache direto de 16 slots, assert `_m055_recompute_count==1` após hit | PASS (compilado e executado, rc=0) |
| M056 | `RAF_056_comparacao_automatica_contra_implementacao_padrao.c` | soma baseline vs soma desenrolada ×4, assert resultados iguais e `ops_optimized<=ops_baseline` | PASS (compilado e executado, rc=0) |

## Resultado: 47 itens deixados sem marcação (corpo não corresponde ao nome)

Confirmado por diff normalizado (removendo o sufixo `mNNN` do nome da função) que os seguintes grupos compartilham corpo byte-idêntico, apesar de cada arquivo declarar macros/registradores específicos do seu domínio nunca usados no corpo:

- **M003–M020** (exceto M001/M002): todos compartilham o mesmo corpo de toggle DDRB/PINB de M001, independentemente de Timer/PWM/ADC/UART/SPI/I2C/Watchdog/Sleep/Brown-out declarados nos comentários.
- **M021–M023, M025–M028, M036–M038, M041–M047, M049** (exceto M024): todos compartilham o mesmo corpo genérico "ler contador duas vezes, retornar 0 se t1>=t0", sem tocar o mecanismo específico nomeado (ex.: M046 "syscall direta" inclui `<sys/syscall.h>` mas nunca chama `syscall()`).
- **M029–M035**: todos compartilham o mesmo corpo `if (!mmio_base) return -1; volatile uint32_t sample = mmio_base[0]; return 0;` — leitura genérica de um único registrador, sem offsets específicos de SPI/I2C/PWM/DMA/FIFO/GPIO-event BCM.
- **M051, M053**: compartilham o corpo de M052 (struct de contadores), mas nem a integração específica com Vectras (M051) nem a separação tradução-vs-execução (M053) está de fato implementada.

Estes 47 itens permanecem `[ ]` nesta passada — marcá-los exigiria reescrever os arquivos para implementar a técnica nomeada, fora do escopo desta passada de reconciliação documental.

## LACUNA conhecida, não tocada

`Apkc/Raf.md` — arquivo de 1 byte, propósito não documentado. Não foi apagado nem preenchido por suposição; permanece como lacuna registrada.

## Divergência de path notada, não corrigida

`RAF_INDEX.md` descreve os 56 métodos como vivendo em `methods/0xx_nome.c`; os arquivos reais estão na raiz do repositório como `RAF_0xx_nome.c`. Fora do escopo desta passada corrigir `RAF_INDEX.md` ou mover arquivos.

## FNext — próximo passo útil

Para qualquer um dos 47 itens não confirmados: reescrever o corpo específico do arquivo (ex.: M029 SPI por registrador BCM deveria de fato escrever/ler os offsets SPI0_CS/SPI0_FIFO do BCM2835, não um único `mmio_base[0]`) e então repetir o ciclo compile+run+diff usado aqui antes de marcar `[x]`.
