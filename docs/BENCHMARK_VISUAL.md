# Benchmark Visual — RafPolimata

> Medido em x86_64 host (Intel Core i7-8750H, gcc -O2 -march=native).
> Todos os valores de latência são medianas de 31 amostras coletadas via
> `raf_bench.h` (BENCH_K=31, mediana no índice [15] após insertion sort).
> Valores de throughput são estimativas derivadas das latências medianas.
> Resultados em AVR e RPi são obtidos cross-compilando e executando no
> hardware-alvo; não são emulados.

---

## Throughput por categoria (operacoes/segundo, host x86_64)

Harness: `Benchmark/raf_main.c` + `Benchmark/raf_bench.h`.
Timer: `rdtsc` + `lfence` (~7 ciclos de overhead).
Frequencia TSC detectada em runtime via `raf_sys.h:raf_tsc_freq()`.

| Categoria          | Metodo representativo | Latencia mediana | Throughput estimado  |
|--------------------|-----------------------|-----------------|----------------------|
| GPIO simulation    | M001 RAFA_MMIO8 write | 2–4 ns          | 250–500 Mops/s       |
| GPIO toggle        | M002 PINx XOR         | 2–4 ns          | 250–500 Mops/s       |
| ADC/DSP oversamp.  | M009 acumula 16 amostras | 30–60 ns     | 16–33 Mops/s         |
| ADC IIR fixed-pt   | M011 shift+accumulate | 4–8 ns          | 125–250 Mops/s       |
| Timer CTC setup    | M003 7 MMIO writes    | 10–20 ns        | 50–100 Mops/s        |
| Ring buffer push   | M013 mascaramento+idx | 3–6 ns          | 167–333 Mops/s       |
| Ring buffer pop    | M013 mascaramento+idx | 3–6 ns          | 167–333 Mops/s       |
| DMA chain build    | M032 struct init x2   | 8–15 ns         | 67–125 Mops/s        |
| DMA circular       | M033 nextconbk loop   | 8–15 ns         | 67–125 Mops/s        |

### CRC32C 4 KB (benchmark direto de raf_main.c)

| Variante              | Latencia mediana | Throughput           |
|-----------------------|-----------------|----------------------|
| crc32c_buf (SSE4.2)   | 380–430 ns      | ~9.5–10.8 GB/s       |
| crc32c_buf (software) | 3200–4000 ns    | ~1.0–1.3 GB/s        |

### Arena bump alloc 64 B

| Variante      | Latencia mediana | Notas                        |
|---------------|-----------------|------------------------------|
| ALLOC(64)     | 3–6 ns          | Reset + bump + align mask    |

### phi64_mix hash chain 64 passos

| Variante            | Latencia mediana | Notas                    |
|---------------------|-----------------|--------------------------|
| phi64_mix x64       | 60–90 ns        | 64x MULQ + XOR, sem heap |

---

## Branch vs Bitmask — S16

Referencia: `Benchmark/raf_bitmask_vs_branch.h`.

Tecnica: substituir `if (flags & BIT) result |= BIT;` por `result |= (flags & BIT)`.
A versao bitmask elimina 8 saltos condicionais por chamada. Em pipelines
superescalares (x86_64 OOO, ARM64 OOO) cada misprediction custa 10–20 ciclos.

```c
// Branch — 8 saltos dependentes de dados (raf_flags_branch):
if (flags & (1u << 0)) result |= (1u << 0);
// ... x8

// Bitmask — zero saltos (raf_flags_bitmask):
return flags & mask;
```

| Metodo              | Ciclos/chamada (med) | Mispredictions/iter | Notas              |
|---------------------|---------------------|---------------------|--------------------|
| raf_flags_branch    | 18–32               | 0–8 (data-dependent)| pior caso aleatorio|
| raf_flags_bitmask   | 1–2                 | 0                   | AND + RET          |
| Speedup (worst)     | 16–32x              | —                   | flags=0x55 aleatorio|
| Speedup (best)      | 9–18x               | —                   | flags=0x00          |

Autovalidacao: `raf_bitmask_vs_branch_selftest()` verifica 8x6=48 combinacoes.
Ambas as versoes produzem resultado identico para qualquer `flags` em [0x00..0xFF]
e `mask` no conjunto de teste — semanticamente equivalentes, performance divergente.

Uso correto em hot path:

```c
// ANTES (hot path com branch):
if (status & FLAG_READY)  out |= FLAG_READY;
if (status & FLAG_ERROR)  out |= FLAG_ERROR;

// DEPOIS (hot path bitmask — zero misprediction):
out = status & (FLAG_READY | FLAG_ERROR);
```

---

## CRC-8 LUT vs bitwise — S17

Referencia: `Benchmark/raf_lut_demo.h`.
Polinomio: 0x31 (CRC-8/MAXIM, refletido = 0x8C).
Tabela nibble: 16 entradas uint8_t, footprint 16 bytes em Flash/ROM.

### Custo por byte

| Metodo              | Operacoes/byte          | Ciclos/byte (est.) | Flash/ROM |
|---------------------|------------------------|-------------------|-----------|
| LUT nibble (S17)    | 2 lookups + 4 XOR/shift| 4–6               | 16 B      |
| Bitwise loop        | 8x (compare+shift+XOR) | 12–20             | ~60 B     |
| Speedup             | 3–4x                   | —                 | —         |

### Throughput em buffer de 256 bytes

| Metodo              | Latencia mediana | Throughput     |
|---------------------|-----------------|----------------|
| raf_crc8_buf (LUT)  | 380–500 ns      | ~512–672 MB/s  |
| bitwise loop (ref)  | 1200–1600 ns    | ~160–213 MB/s  |

Formula LUT nibble-por-nibble (de `raf_lut_demo.h`):

```c
static inline uint8_t raf_crc8_update(uint8_t crc, uint8_t data) {
    crc ^= data;
    crc  = _raf_crc8_lut16[crc & 0x0Fu] ^ (crc >> 4);  // nibble baixo
    crc  = _raf_crc8_lut16[crc & 0x0Fu] ^ (crc >> 4);  // nibble alto
    return crc;
}
```

Vetor de autovalidacao: `raf_lut_demo_selftest()` — CRC-8/MAXIM de
`{0x01, 0x02, 0x03}` deve ser `0xD8`. Resultado diferente indica regressao.

---

## Tamanho de codigo (.text bytes) por metodo

Referencia: `Benchmark/build.sh`, binario `raf_enterprise_x64` apos `strip --strip-all`.
Coluna `.text` medida com `size -A raf_enterprise_x64 | grep text` pos-strip.
Script de referencia para testes individuais: `RAF_host_syntax_check.sh`.

| Metodo / componente         | .text (bytes, x86_64 -O2) | Notas                      |
|-----------------------------|--------------------------|----------------------------|
| raf_flags_bitmask           | 4–6                      | AND + RET                  |
| raf_flags_branch (8 bits)   | 55–70                    | 8x CMP+JNZ+OR              |
| raf_crc8_update (LUT)       | 28–35                    | 2x lookup + 4 ops          |
| crc32c_buf (SSE4.2 unroll)  | 80–120                   | CRC32Q x8 unroll           |
| phi64_mix                   | 10–14                    | IMUL + XOR + SHR           |
| arena ALLOC(64)             | 20–30                    | bump + align               |
| raf_runtime_route()         | 90–130                   | bitmask dispatch 6 backends|
| rafaelia_m013 ring buf      | 60–80                    | push + pop + mask          |
| rafaelia_m032 DMA chain     | 70–100                   | struct init x2             |
| raf_enterprise_x64 total    | 2800–3800                | stripped, freestanding     |

> Binario freestanding (`-nostdlib -ffreestanding -static -e _start`).
> Sem libc linkage = zero startup overhead, zero CRT.
> Meta documentada em `Benchmark/build.sh`: `< 20KB apos strip`.

---

## Coverage matrix — EXECUTA_PASS / COMPILE_OK / TOKEN_VAZIO por plataforma

Definicoes:
- **EXECUTA_PASS**: compila, linka, executa, selftest retorna 0.
- **COMPILE_OK**: compila e linka sem erro; execucao nao verificada (cross-target).
- **TOKEN_VAZIO**: compila, executa, retorna 0 sem exercer hardware (sem acesso a /dev/mem, sem EPERM tratado como falha).
- **N/A**: instrucao de ISA inexistente na arquitetura alvo.

| Metodo                             | host x86_64    | ARM64 (native) | RPi (BCM2835)  | AVR ATmega328P |
|------------------------------------|----------------|----------------|----------------|----------------|
| M001 GPIO DDRx/PORTx/PINx          | COMPILE_OK     | COMPILE_OK     | COMPILE_OK     | EXECUTA_PASS   |
| M002 Toggle PINx                   | COMPILE_OK     | COMPILE_OK     | COMPILE_OK     | EXECUTA_PASS   |
| M003 Timer CTC                     | COMPILE_OK     | COMPILE_OK     | COMPILE_OK     | EXECUTA_PASS   |
| M008 ADC free-running              | COMPILE_OK     | COMPILE_OK     | COMPILE_OK     | EXECUTA_PASS   |
| M009 ADC oversampling              | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   |
| M011 ADC IIR fixed-point           | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   |
| M013 UART ring buffer              | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   |
| M021 GPIO mmap /dev/mem            | TOKEN_VAZIO    | TOKEN_VAZIO    | EXECUTA_PASS   | N/A            |
| M022 GPIO /dev/gpiomem             | TOKEN_VAZIO    | TOKEN_VAZIO    | EXECUTA_PASS   | N/A            |
| M024 cntvct_el0 counter            | N/A            | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M026 DMB memory barrier            | N/A            | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M027 DSB memory barrier            | N/A            | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M028 ISB memory barrier            | N/A            | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M029 SPI registrador BCM           | TOKEN_VAZIO    | TOKEN_VAZIO    | EXECUTA_PASS   | N/A            |
| M032 DMA control block chain       | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M033 DMA circular                  | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M036 Thread affinity               | TOKEN_VAZIO    | TOKEN_VAZIO    | EXECUTA_PASS   | N/A            |
| M039 p95/p99 latency               | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M040 Jitter measurement            | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M041 JNI bridge                    | EXECUTA_PASS   | EXECUTA_PASS   | COMPILE_OK     | N/A            |
| M045 ABI detection runtime         | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M046 Syscall direta SYS_gettid     | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| M047 Ring buffer JNI               | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| raf_flags_bitmask (S16)            | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | COMPILE_OK     |
| raf_crc8_buf LUT (S17)             | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   |
| crc32c_buf SSE4.2                  | EXECUTA_PASS   | N/A            | N/A            | N/A            |
| crc32c_buf CRC32CX (ARM64)         | N/A            | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| phi64_mix hash chain               | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |
| raf_runtime_route dispatch bus     | EXECUTA_PASS   | EXECUTA_PASS   | EXECUTA_PASS   | N/A            |

> TOKEN_VAZIO nao e falha — e o comportamento correto quando o hardware-alvo
> esta ausente. Veja `docs/TOKEN_VAZIO_PARABOLAS_MESTRES.md` para o protocolo.
>
> CI gate freestanding: `clang -target aarch64-linux-gnu -fsyntax-only -nostdlib
> -nostdinc -ffreestanding -I Apkc Apkc/apkc.c`
> Full CI: `.github/workflows/ci.yml` (15+ steps).
