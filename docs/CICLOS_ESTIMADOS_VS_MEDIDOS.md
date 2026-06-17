# S27 — Tabela de ciclos estimados vs ciclos medidos

Atualização: 2026-06-17. Host de referência: x86_64 Linux (esta tabela; referenciar coluna ARM64 apenas quando medido em hardware real).

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
| `clock_gettime(CLOCK_MONOTONIC)` | 25–50 | TOKEN_VAZIO | 20–40 | TOKEN_VAZIO | 1M | NOT_RUN |
| `dmb ish` (ARM64) | 10–100 | TOKEN_VAZIO | — | — | 1M | TOKEN_VAZIO (sem ARM64 nativo) |
| `__sync_synchronize()` (x86) | — | — | 1–5 | TOKEN_VAZIO | 1M | NOT_RUN |
| `sched_setaffinity()` (Linux syscall) | — | — | 500–2000 | TOKEN_VAZIO | 10k | NOT_RUN |
| `syscall(SYS_gettid)` direto | — | — | 100–300 | TOKEN_VAZIO | 1M | NOT_RUN |
| ADC single-shot (AVR, 16MHz, /128) | 104 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| UART TX byte polling (AVR, 9600 baud) | ~1666 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| SPI byte full-duplex (AVR, /4) | 16 | TOKEN_VAZIO | — | — | 1k | TOKEN_VAZIO (sem AVR) |
| IIR fixed-point 1 iteração (M011) | 3–6 | NOT_RUN | 2–4 | NOT_RUN | 1M | NOT_RUN |
| Ring buffer push/pop (M047/M013) | 2–5 | NOT_RUN | 1–3 | NOT_RUN | 1M | NOT_RUN |
| JSON export sem sprintf (M050) | 50–200 | NOT_RUN | 30–100 | NOT_RUN | 100k | NOT_RUN |
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

## Lacuna registrada

Esta tabela é um esqueleto — os valores "NOT_RUN" devem ser preenchidos quando há acesso a hardware ARM64 nativo ou Android físico. A coluna x86_64 pode ser completada em qualquer host Linux com este repositório clonado.
