# RAF TraceBuf C/ASM/HEX

Modulo autoral para codificar eventos do proprio processo em buffer fornecido pelo chamador.

## Escopo

- C puro, header-only.
- Sem alocacao dinamica.
- Sem printf/string helpers externos.
- Sem biblioteca externa.
- Sem transporte embutido.
- Sem captura de processos externos.
- Hexadecimal fixo para tag, code, value e tick.
- Inline ASM seguro apenas para barreira de compilador e leitura de tick ARM64 quando disponivel.

## Arquivo

- `Benchmark/raf_tracebuf.h`

## Modelo operacional

```text
entrada: tag, code, value
estado: buffer do chamador
saida textual: RAFTRACE t=0x... tag=0x... code=0x... value=0x...
saida binaria: struct raf_trace_event_t
falha: RAF_TRACEBUF_TRUNC quando o buffer nao comporta a saida
```

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

## Criterios de desempenho

1. Hot path sem heap.
2. Buffer controlado pelo chamador.
3. Hexadecimal fixo sem formatador externo.
4. Caminho truncado explicito, sem overflow silencioso.
5. Baseline C antes de qualquer especializacao.
6. Inline ASM somente quando houver ganho ou acesso a registrador especifico.
7. Qualquer sink de transporte deve ser modulo separado e revisavel.

## Limite de claim

Este modulo prova codificacao local de evento em buffer. Nao prova runtime Android completo, nao prova coleta externa e nao substitui logcat/device proof.
