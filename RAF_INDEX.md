# Índice dos 56 arquivos C

> **Layout em disco (flat / repo root).** Todos os 56 arquivos de método
> (`RAF_NNN_<slug>.c`) e o cabeçalho comum `RAF_rafaelia_common.h` ficam na
> **raiz do repositório**, lado a lado — não há diretório `methods/` nem
> `include/`. Cada método faz `#include "RAF_rafaelia_common.h"` e compila com:
>
> ```bash
> gcc -c -I. RAF_NNN_<slug>.c
> ```
>
> **Correção (lacuna L12).** As versões anteriores deste índice listavam caminhos
> prefixados com `methods/001_...` — esses caminhos eram **aspiracionais e não
> correspondiam ao layout em disco**. Abaixo, cada entrada usa o nome de arquivo
> real (`RAF_NNN_<slug>.c`) verificado contra o conteúdo da raiz do repositório.

## Cabeçalho comum

- `RAF_rafaelia_common.h` — cabeçalho compartilhado, incluído por todos os 56 métodos

## Métodos

- `RAF_001_acesso_direto_a_ddrx_portx_pinx.c` — Acesso direto a DDRx PORTx PINx
- `RAF_002_toggle_por_escrita_em_pinx.c` — Toggle por escrita em PINx
- `RAF_003_timer_ctc_para_evento_periodico.c` — Timer CTC para evento periódico
- `RAF_004_timer_fast_pwm_por_registrador.c` — Timer Fast PWM por registrador
- `RAF_005_timer_phase_correct_pwm_para_controle_motor.c` — Timer Phase Correct PWM para controle motor
- `RAF_006_input_capture_para_medir_pulso.c` — Input Capture para medir pulso
- `RAF_007_output_compare_para_gerar_onda_sem_cpu.c` — Output Compare para gerar onda sem CPU
- `RAF_008_adc_free_running.c` — ADC free-running
- `RAF_009_adc_com_oversampling.c` — ADC com oversampling
- `RAF_010_adc_com_media_movel_inteira.c` — ADC com média móvel inteira
- `RAF_011_adc_com_filtro_iir_fixed_point.c` — ADC com filtro IIR fixed-point
- `RAF_012_uart_polling_minimo.c` — UART polling mínimo
- `RAF_013_uart_interrupt_driven_com_ring_buffer.c` — UART interrupt-driven com ring buffer
- `RAF_014_spi_full_duplex_por_registrador.c` — SPI full-duplex por registrador
- `RAF_015_spi_burst_transfer.c` — SPI burst transfer
- `RAF_016_i2c_twi_com_timeout.c` — I2C/TWI com timeout
- `RAF_017_watchdog_como_recuperacao_de_travamento.c` — Watchdog como recuperação de travamento
- `RAF_018_watchdog_como_base_temporal_aproximada.c` — Watchdog como base temporal aproximada
- `RAF_019_sleep_mode_com_wake_por_interrupcao.c` — Sleep mode com wake por interrupção
- `RAF_020_brown_out_flag_como_diagnostico_de_alimentacao.c` — Brown-out flag como diagnóstico de alimentação
- `RAF_021_gpio_por_mmap.c` — GPIO por mmap
- `RAF_022_gpio_por_dev_gpiomem.c` — GPIO por /dev/gpiomem
- `RAF_023_gpio_por_dev_mem_controlado.c` — GPIO por /dev/mem controlado
- `RAF_024_leitura_de_contador_arm64_cntvct_el0.c` — Leitura de contador ARM64 cntvct_el0
- `RAF_025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo.c` — Uso de cntfrq_el0 para converter ciclos em tempo
- `RAF_026_memory_barrier_dmb.c` — Memory barrier dmb
- `RAF_027_memory_barrier_dsb.c` — Memory barrier dsb
- `RAF_028_memory_barrier_isb.c` — Memory barrier isb
- `RAF_029_spi_por_registrador_bcm.c` — SPI por registrador BCM
- `RAF_030_i2c_por_registrador_bcm.c` — I2C por registrador BCM
- `RAF_031_pwm_por_clock_manager.c` — PWM por clock manager
- `RAF_032_dma_control_block_chain.c` — DMA control block chain
- `RAF_033_dma_circular.c` — DMA circular
- `RAF_034_fifo_pwm_para_audio.c` — FIFO PWM para áudio
- `RAF_035_gpio_event_detect_por_polling_leve.c` — GPIO event detect por polling leve
- `RAF_036_afinidade_de_thread_em_linux_android.c` — Afinidade de thread em Linux/Android
- `RAF_037_prioridade_de_thread_para_benchmark.c` — Prioridade de thread para benchmark
- `RAF_038_isolamento_de_nucleo_quando_disponivel.c` — Isolamento de núcleo quando disponível
- `RAF_039_medicao_de_p95_e_p99_de_latencia.c` — Medição de p95 e p99 de latência
- `RAF_040_medicao_de_jitter_por_amostra.c` — Medição de jitter por amostra
- `RAF_041_jni_bridge_minimo.c` — JNI bridge mínimo
- `RAF_042_cmake_separado_por_abi.c` — CMake separado por ABI
- `RAF_043_build_arm64_v8a.c` — Build arm64-v8a
- `RAF_044_build_armeabi_v7a.c` — Build armeabi-v7a
- `RAF_045_deteccao_de_abi_em_runtime.c` — Detecção de ABI em runtime
- `RAF_046_syscall_direta_quando_fizer_sentido.c` — Syscall direta quando fizer sentido
- `RAF_047_ring_buffer_nativo_exposto_ao_kotlin_java.c` — Ring buffer nativo exposto ao Kotlin/Java
- `RAF_048_log_binario_em_vez_de_log_textual_pesado.c` — Log binário em vez de log textual pesado
- `RAF_049_benchmark_via_termux_cli.c` — Benchmark via Termux CLI
- `RAF_050_exportacao_de_resultado_em_json.c` — Exportação de resultado em JSON
- `RAF_051_hook_de_teste_para_vectras.c` — Hook de teste para Vectras
- `RAF_052_probe_de_hot_path_no_qemu_tcg.c` — Probe de hot path no QEMU/TCG
- `RAF_053_medicao_de_traducao_vs_execucao_no_qemu.c` — Medição de tradução vs execução no QEMU
- `RAF_054_batching_de_operacoes_repetidas.c` — Batching de operações repetidas
- `RAF_055_cache_local_de_resultado_tecnico.c` — Cache local de resultado técnico
- `RAF_056_comparacao_automatica_contra_implementacao_padrao.c` — Comparação automática contra implementação padrão

## Build / verificação

Compilar todos os 56 métodos para objetos em `/tmp` (prova de build):

```bash
for f in RAF_*.c; do
  gcc -c -I. "$f" -o /tmp/$(basename "$f" .c).o || echo "FAIL $f"
done
```

## Cadeia de custódia (lacuna L12)

- **Data:** 2026-06-17
- **Arquivos `RAF_*.c` em disco (raiz):** 56
- **Entradas neste índice:** 56 (1:1 — cada entrada corresponde a um arquivo real)
- **Cabeçalho comum `RAF_rafaelia_common.h`:** presente na raiz
- **Resultado do loop de build acima (`gcc -c -I.`):** PASS=56 FAIL=0
- **Divergências de nome/arquivo encontradas:** nenhuma (todos os 56 nomes
  do índice existem em disco; os antigos prefixos `methods/` foram corrigidos)
- **Status L12:** RESOLVIDO — índice realinhado ao layout flat em disco
