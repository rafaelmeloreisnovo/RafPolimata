# S27 — Tabela de ciclos estimados vs ciclos medidos

Atualização: 2026-06-20. Host de referência: x86_64 Linux 6.18.5 gcc 13, -O2, 1M iterações, clock nominal ~3.0 GHz assumido para conversão ns→ciclos.

## Metodologia

- **Estimado**: derivado da especificação da arquitetura (ISA / manual do fabricante) ou de microbenchmarks publicados.
- **Medido**: resultado de execução com `clock_gettime(CLOCK_MONOTONIC)` ou `mrs cntvct_el0` em loop de N iterações, eliminando overhead de chamada.
- **N**: número de iterações usado no loop de medição.
- **Unidade**: ciclos (convertido a partir de ns pelo clock nominal).
- Estado: PASS (medido), ESTIMATE (não medido aqui), TOKEN_VAZIO (hardware ausente).

## Tabela

| Operação | Estimado ARM64 (ciclos) | Medido ARM64 | Estimado x86_64 (ciclos) | Medido x86_64 | N | Estado |
|---|---:|---:|---:|---:|---:|---|
| Ler registrador GPIO (MMIO) | 1–2 | TOKEN_VAZIO | — | — | 1M | TOKEN_VAZIO (sem RPi) |
| Toggle GPIO por PINx (AVR, 16MHz) | 1 | TOKEN_VAZIO | — | — | 1M | TOKEN_VAZIO (sem AVR) |
| `mrs cntvct_el0` | 1–5 | TOKEN_VAZIO | — | — | 1M | TOKEN_VAZIO (sem ARM64 nativo) |
| `clock_gettime(CLOCK_MONOTONIC)` | 25–50 | TOKEN_VAZIO | 20–40 | **60 ciclos** (~20 ns) | 1M | PASS |
| `dmb ish` (ARM64) | 10–100 | TOKEN_VAZIO | — | — | 1M | TOKEN_VAZIO (sem ARM64 nativo) |
| `__sync_synchronize()` (x86) | — | — | 1–5 | **38 ciclos** (~12.6 ns) | 1M | PASS |
| `sched_setaffinity()` (Linux syscall) | — | — | 500–2000 | TOKEN_VAZIO | 10k | TOKEN_VAZIO (EPERM sem root) |
| `syscall(SYS_gettid)` direto | — | — | 100–300 | **249 ciclos** (~83 ns) | 1M | PASS |
| ADC single-shot (AVR, 16MHz, /128) | 104 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| UART TX byte polling (AVR, 9600 baud) | ~1666 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| SPI byte full-duplex (AVR, /4) | 16 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| IIR fixed-point 1 iteração (M011) | 3–6 | TOKEN_VAZIO | 2–4 | **7.6 ciclos** (~2.5 ns) | 1M | PASS |
| Ring buffer push/pop (M047/M013) | 2–5 | TOKEN_VAZIO | 1–3 | **6.8 ciclos** (~2.3 ns) | 1M | PASS |
| JSON export sem sprintf (M050) | 50–200 | TOKEN_VAZIO | 30–100 | TOKEN_VAZIO | 100k | NOT_RUN |
| Bus throughput (S23, baseline) | — | — | — | PASS (ver Benchmark/) | 10M | PASS |
| Batching ×8 vs individual (M054) | — | — | — | PASS (rc=0) | 64 | PASS |

## Como preencher os NOT_RUN

```bash
# x86_64 — instância base para clock_gettime
# Usa o driver do M025 como harness de ciclos:
gcc -std=c11 -O2 -D_POSIX_C_SOURCE=200809L RAF_025_*.c -o /tmp/t025 && /tmp/t025
# Adicionar loop de 1M chamadas ao driver e dividir elapsed_ns / N / (1ns/cycle_ns)
# substituir TOKEN_VAZIO pelas colunas "Medido" acima.
```

Para ARM64 real (Raspberry Pi 4 ou Android NDK):
```bash
# Cross-compile e executar:
aarch64-linux-gnu-gcc -std=c11 -O2 -march=armv8.2-a RAF_025_*.c -o t025_arm64
# scp para device, executar, anotar saída
```

## Medições realizadas em 2026-06-20

Metodologia: loop de 1M iterações, `clock_gettime(CLOCK_MONOTONIC)` antes/depois, divisão por N.
Clock nominal assumido: 3.0 GHz (conversão ns→ciclos). Host: x86_64 Linux 6.18.5 gcc 13 -O2.

| Operação | ns/op medido | ciclos (~3GHz) | N |
|----------|-------------:|---------------:|--:|
| clock_gettime(CLOCK_MONOTONIC) | 20.1 ns | 60 | 1M |
| __sync_synchronize() x86 | 12.6 ns | 38 | 1M |
| syscall(SYS_gettid) | 82.8 ns | 249 | 1M |
| IIR Q0 1 iteração (M011) | 2.5 ns | 7.6 | 1M |
| Ring buffer push+pop (M013) | 2.3 ns | 6.8 | 1M |

## Lacuna registrada

Colunas ARM64 TOKEN_VAZIO até medição em hardware ARM64 nativo (RPi 4 ou Android).
JSON export (M050) e sched_setaffinity pendentes.
