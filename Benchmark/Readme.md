# Benchmark RAFAELIA — foco low-level (ARM64/x86-64/ARM32)

## Correções aplicadas para ARM32 (Termux Android 10+)

- `raf_sys.h` agora usa `clock_gettime(CLOCK_MONOTONIC)` em ARM32 para timer do benchmark.
- Motivo: `PMCCNTR` (coprocessor p15) frequentemente é bloqueado no userland Android/Termux e gera resultado inconsistente.
- Com isso, ARM32 mede em nanossegundos reais (tick = 1ns lógico, `freq=1_000_000_000`).

## Arquivos críticos e intenção de design

- `raf_sys.h`: syscall ABI por arquitetura, sem libc/stdio.
- `raf_crc32c.h`: CRC32C por ISA (ARM64/x86 hardware, ARM32 software branchless por XOR/máscara).
- `raf_hash.h`: FNV1a + PHI64 + corrente tipo Merkle/HashVivo, sem heap.
- `raf_main.c`: orquestra benchmark, sem malloc/GC.
- `build.sh` e `build2.sh`: rotas de compilação para host nativo e ARM32/ARM64.

## Compilar no Termux ARM32

```bash
cd Benchmark
bash build2.sh
```

ou

```bash
cd Benchmark
bash build.sh
```

## Objetivo de fricção reduzida

- Sem heap dinâmico (`malloc/free`) no hot path.
- Sem runtime de GC.
- Dependência de syscalls mínimas para I/O e relógio.
- Núcleos críticos com operações primitivas (`xor`, `shift`, `mul`, `mov`) favorecendo determinismo e previsibilidade.
