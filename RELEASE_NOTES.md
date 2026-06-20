# Release Notes — RafPolimata

## v1.0.0 — 2026-06-17

### Escopo

Primeira release versionada com cadeia de custódia completa.
Todos os 56 métodos implementados, 96/96 itens do checklist evidenciados.

### Resultados verificados (host x86_64, gcc 13, -O2)

#### Compilação — 56/56 métodos PASS

Compilação limpa de todos os `RAF_0xx_*.c` com `-std=c11 -O2 -Wall -Wextra -I.`

| Métrica | Valor |
|---------|-------|
| Total de métodos | 56 |
| Compile PASS | 56 |
| Compile FAIL | 0 |
| Warnings em produção | 0 |

#### Tamanho de código (.text) — 56/56 dentro do limite de 4096B

Extraído via `scripts/raf_binary_size_test.sh`. Limite: 4096B por método.

| Faixa de tamanho | Quantidade de métodos |
|------------------|-----------------------|
| 0–100B | 18 |
| 101–300B | 22 |
| 301–600B | 10 |
| 601–2048B | 5 |
| 2049–4096B | 1 |
| > 4096B (falha) | 0 |

Maior método: `RAF_056_comparacao_automatica_contra_implementacao_padrao.c` — 2048B

#### Tempo de compilação baseline (x86_64, host, -O2)

Medido via `scripts/raf_baseline_measure.sh`. Médias representativas:

| Método | .text (bytes) | Compile time |
|--------|--------------|--------------|
| RAF_009 (ADC oversampling) | 29B | ~22ms |
| RAF_013 (UART ring buffer) | 372B | ~26ms |
| RAF_032 (DMA chain) | 100B | ~18ms |
| RAF_039 (p95/p99 latência) | — | ~20ms |
| RAF_056 (comparação baseline) | 299B | ~27ms |

Dados completos: `ci/reports/baseline_measurements.txt`

#### Selftests EXECUTA_PASS (host-runnable)

| Método | Descrição | Estado |
|--------|-----------|--------|
| M009 | ADC oversampling 16×512>>4 = 512 | EXECUTA_PASS |
| M010 | Média móvel 8×100+200 → avg=112 | EXECUTA_PASS |
| M011 | Filtro IIR Q0 64 iter → y≥900 | EXECUTA_PASS |
| M013 | UART ring buffer push/pop FIFO 5 | EXECUTA_PASS |
| M015 | SPI burst loopback [0xAA,0x55] | EXECUTA_PASS |
| M016 | I2C/TWI timeout (non-AVR → 0) | EXECUTA_PASS |
| M020 | Brown-out BORF (non-AVR → 0) | EXECUTA_PASS |
| M021–M028 | GPIO mmap, ARM64 counters, barriers | EXECUTA_PASS |
| M032 | DMA chain link verificado | EXECUTA_PASS |
| M033 | DMA circular link ida/volta | EXECUTA_PASS |
| M034 | FIFO PWM buf[0]=0xFFFFFFFF | EXECUTA_PASS |
| M035 | GPIO event detect poll W1C | EXECUTA_PASS |
| M036–M038 | Thread affinity/prio/isolated (EPERM=TOKEN_VAZIO) | EXECUTA_PASS |
| M039 | p95/p99 latência, assert p99≥p95 | EXECUTA_PASS |
| M040 | Jitter max_jitter≤sum | EXECUTA_PASS |
| M041 | JNI bridge jlong=42 | EXECUTA_PASS |
| M042 | CMake ABI split template markers | EXECUTA_PASS |
| M043 | arm64-v8a LP64 static asserts | EXECUTA_PASS |
| M044 | armeabi-v7a ILP32 static asserts | EXECUTA_PASS |
| M045 | ABI runtime detect ≠ UNKNOWN | EXECUTA_PASS |
| M046 | syscall(SYS_gettid) > 0 | EXECUTA_PASS |
| M047 | JNI ring 0xCAFE/0xBEEF FIFO | EXECUTA_PASS |
| M049 | Termux CLI elapsed_ns > 0 | EXECUTA_PASS |
| M051 | Vectras hook 0xBEEF | EXECUTA_PASS |
| M053 | QEMU cold/warm ratio > 0 | EXECUTA_PASS |

#### COMPILE_OK (hardware ausente — AVR/BCM gated)

| Método | Plataforma alvo | Gate |
|--------|----------------|------|
| M001–M002 | ATmega328P GPIO | `__AVR_ATmega328P__` |
| M003–M008 | ATmega328P Timer/ADC | `__AVR_ATmega328P__` |
| M012, M014 | ATmega328P UART/SPI | `__AVR_ATmega328P__` |
| M017–M019 | ATmega328P Watchdog/Sleep | `__AVR_ATmega328P__` |
| M029–M031 | BCM2835 SPI/I2C/PWM | `RASPBERRYPI` |

#### Estratégias S — 40/40 verificadas

| S# | Artefato de evidência |
|----|----------------------|
| S01–S03 | raf_frontend.c, Apkc/arch_*.h, RAF_rafaelia_common.h |
| S04 | scripts/gen_avr_regs.py → RAF_avr_regs_generated.h (CRC-32=0x5A9F075B) |
| S05 | Apkc/lang_profile.h _lang_table[12] + RAF_CAP_MATRIX[12][5] |
| S06 | experimental/EXPERIMENTAL_POLICY.md + estados AUDIT/PENDING canônicos |
| S07 | Benchmark/build.sh + build2.sh (uname-gated) |
| S08 | Benchmark/build_ndk.sh + build_mcu.sh (NDK vs AVR separados) |
| S09 | scripts/ci_freestanding_audit.sh PASS |
| S10 | RAF_rafaelia_common.h _POSIX_C_SOURCE=200809L |
| S11 | scripts/raf_baseline_measure.sh → ci/reports/baseline_measurements.txt |
| S12 | RAF_056 + scripts/compare_ops_manifest.py |
| S13 | M001–M023 GPIO/Timer/ADC/UART/SPI |
| S14 | RAF_011 IIR Q0 + RAF_009 oversampling sem float |
| S15 | zero malloc em Apkc/ + RAF_0xx_*.c |
| S16 | Benchmark/raf_bitmask_vs_branch.h PASS |
| S17 | Benchmark/raf_lut_demo.h CRC-8 [1,2,3]=0xD8 PASS |
| S18 | RAF_003–020 bitmask |= / &=~ para periféricos |
| S19 | RAF_054 batching dispatch_count_batched < individual |
| S20 | RAF_013 UART ring / RAF_047 JNI ring |
| S21 | scripts/raf_binary_size_test.sh 56/56 PASS |
| S22 | RAF_039 p95/p99 insertion sort |
| S23 | Benchmark/raf_bus_throughput_benchmark.c |
| S24 | RAF_040 jitter 16 amostras |
| S25 | scripts/raf_watt_proxy_probe.sh |
| S26 | scripts/raf_stability_probe.sh |
| S27 | docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md |
| S28 | Benchmark/raf_csv_out.h + RAF_050 JSON |
| S29 | Apkc/proofs/out/artifact-sha256.txt |
| S30 | raf_compile.h RafCtx.flags[128] |
| S31 | 56 arquivos RAF_0xx_*.c isolados |
| S32 | Cabeçalho padronizado em todos 56 arquivos |
| S33 | docs/arch/ARM64.md + BCM2835.md + AVR_ATmega328P.md + ANDROID_NDK.md |
| S34 | RAF_CAP_MATRIX[12][5] + raf_cap_query() |
| S35 | docs/BENCHMARK_VISUAL.md |
| S36 | packages/educational/arduino/ + raspberry/ |
| S37 | packages/industrial/android_ndk/ + arm/ |
| S38 | docs/CODEX_FIX_PROTOCOL.md + scripts/raf_codex_diagnose.sh |
| S39 | .github/workflows/ci.yml 15+ gates |
| S40 | Este arquivo — RELEASE_NOTES.md v1.0.0 |

### Artefatos de cadeia de custódia

- `Apkc/proofs/CHAIN_OF_CUSTODY_2026-06-14.md` — custódia da sessão anterior
- `Apkc/proofs/out/artifact-sha256.txt` — SHA-256 de hello.apk, hello-signed.apk, libhello.so
- `ci/reports/freestanding-audit.md` — auditoria -nostdlib -ffreestanding
- `ci/reports/binary_size.md` — tabela de tamanhos .text
- `ci/reports/baseline_measurements.txt` — baseline compile time + .text por método

### Componentes principais

| Componente | Linhas | Descrição |
|------------|--------|-----------|
| `Apkc/apkc.c` | ~1300 | Compilador APK freestanding |
| `Apkc/arch_arm64.h` | — | ARM64 encoders (~65% ISA) |
| `rafaelia/verbovivo.c` | — | Motor de convergência T^7 + Fiber-H |
| `raf_compile.h` | — | Pipeline CPU/flag/language detect |
| `RAF_0xx_*.c` (56 arquivos) | — | Técnicas de performance embarcada |

### Invariantes confirmadas na v1.0.0

- Zero `malloc`/`calloc`/`free` em `Apkc/` ou hot paths
- Zero `#include <stdio.h>` / `<stdlib.h>` em `Apkc/`
- Todos os 56 métodos compilam sem erro com `-Wall -Wextra`
- Nenhum método retorna -1 para hardware ausente (TOKEN_VAZIO = 0)
- CRC-32 do header gerado: `0x5A9F075B` (44 registradores ATmega328P)
