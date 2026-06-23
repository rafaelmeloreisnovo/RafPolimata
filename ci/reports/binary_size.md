# Relatório de tamanho binário — S21

Data: 2026-06-17T10:02:59Z
Limite .text por arquivo: 4096 bytes

| Arquivo | .text (bytes) | .rodata (bytes) | Total .o (bytes) | Estado |
|---------|:---:|:---:|:---:|:---:|
| RAF_001_acesso_direto_a_ddrx_portx_pinx.c | 0 | 0 | 1288 | PASS |
| RAF_002_toggle_por_escrita_em_pinx.c | 0 | 0 | 1272 | PASS |
| RAF_003_timer_ctc_para_evento_periodico.c | 0 | 0 | 1288 | PASS |
| RAF_004_timer_fast_pwm_por_registrador.c | 0 | 0 | 1280 | PASS |
| RAF_005_timer_phase_correct_pwm_para_controle_motor.c | 0 | 0 | 1312 | PASS |
| RAF_006_input_capture_para_medir_pulso.c | 0 | 0 | 1448 | PASS |
| RAF_007_output_compare_para_gerar_onda_sem_cpu.c | 0 | 0 | 1296 | PASS |
| RAF_008_adc_free_running.c | 0 | 0 | 1256 | PASS |
| RAF_009_adc_com_oversampling.c | 0 | 0 | 1616 | PASS |
| RAF_010_adc_com_media_movel_inteira.c | 0 | 0 | 1992 | PASS |
| RAF_011_adc_com_filtro_iir_fixed_point.c | 0 | 0 | 1664 | PASS |
| RAF_012_uart_polling_minimo.c | 0 | 0 | 1264 | PASS |
| RAF_013_uart_interrupt_driven_com_ring_buffer.c | 0 | 0 | 2912 | PASS |
| RAF_014_spi_full_duplex_por_registrador.c | 0 | 0 | 1288 | PASS |
| RAF_015_spi_burst_transfer.c | 0 | 0 | 1432 | PASS |
| RAF_016_i2c_twi_com_timeout.c | 0 | 0 | 1264 | PASS |
| RAF_017_watchdog_como_recuperacao_de_travamento.c | 0 | 0 | 1408 | PASS |
| RAF_018_watchdog_como_base_temporal_aproximada.c | 0 | 0 | 1464 | PASS |
| RAF_019_sleep_mode_com_wake_por_interrupcao.c | 0 | 0 | 1296 | PASS |
| RAF_020_brown_out_flag_como_diagnostico_de_alimentacao.c | 0 | 0 | 1312 | PASS |
| RAF_021_gpio_por_mmap.c | 0 | 0 | 1800 | PASS |
| RAF_022_gpio_por_dev_gpiomem.c | 0 | 0 | 1816 | PASS |
| RAF_023_gpio_por_dev_mem_controlado.c | 0 | 0 | 1856 | PASS |
| RAF_024_leitura_de_contador_arm64_cntvct_el0.c | 0 | 0 | 1648 | PASS |
| RAF_025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo.c | 0 | 0 | 1664 | PASS |
| RAF_026_memory_barrier_dmb.c | 0 | 0 | 1480 | PASS |
| RAF_027_memory_barrier_dsb.c | 0 | 0 | 1480 | PASS |
| RAF_028_memory_barrier_isb.c | 0 | 0 | 1472 | PASS |
| RAF_029_spi_por_registrador_bcm.c | 0 | 0 | 1272 | PASS |
| RAF_030_i2c_por_registrador_bcm.c | 0 | 0 | 1272 | PASS |
| RAF_031_pwm_por_clock_manager.c | 0 | 0 | 1264 | PASS |
| RAF_032_dma_control_block_chain.c | 0 | 0 | 1880 | PASS |
| RAF_033_dma_circular.c | 0 | 0 | 1864 | PASS |
| RAF_034_fifo_pwm_para_audio.c | 0 | 0 | 1920 | PASS |
| RAF_035_gpio_event_detect_por_polling_leve.c | 0 | 0 | 1608 | PASS |
| RAF_036_afinidade_de_thread_em_linux_android.c | 0 | 0 | 1600 | PASS |
| RAF_037_prioridade_de_thread_para_benchmark.c | 0 | 0 | 1576 | PASS |
| RAF_038_isolamento_de_nucleo_quando_disponivel.c | 0 | 0 | 1880 | PASS |
| RAF_039_medicao_de_p95_e_p99_de_latencia.c | 0 | 0 | 1840 | PASS |
| RAF_040_medicao_de_jitter_por_amostra.c | 0 | 0 | 1800 | PASS |
| RAF_041_jni_bridge_minimo.c | 0 | 0 | 1360 | PASS |
| RAF_042_cmake_separado_por_abi.c | 0 | 0 | 2168 | PASS |
| RAF_043_build_arm64_v8a.c | 0 | 0 | 1256 | PASS |
| RAF_044_build_armeabi_v7a.c | 0 | 0 | 1256 | PASS |
| RAF_045_deteccao_de_abi_em_runtime.c | 0 | 0 | 1272 | PASS |
| RAF_046_syscall_direta_quando_fizer_sentido.c | 0 | 0 | 1456 | PASS |
| RAF_047_ring_buffer_nativo_exposto_ao_kotlin_java.c | 0 | 0 | 1848 | PASS |
| RAF_048_log_binario_em_vez_de_log_textual_pesado.c | 0 | 0 | 1304 | PASS |
| RAF_049_benchmark_via_termux_cli.c | 0 | 0 | 2048 | PASS |
| RAF_050_exportacao_de_resultado_em_json.c | 0 | 0 | 2160 | PASS |
| RAF_051_hook_de_teste_para_vectras.c | 0 | 0 | 1816 | PASS |
| RAF_052_probe_de_hot_path_no_qemu_tcg.c | 0 | 0 | 1296 | PASS |
| RAF_053_medicao_de_traducao_vs_execucao_no_qemu.c | 0 | 0 | 2144 | PASS |
| RAF_054_batching_de_operacoes_repetidas.c | 0 | 0 | 1712 | PASS |
| RAF_055_cache_local_de_resultado_tecnico.c | 0 | 0 | 1920 | PASS |
| RAF_056_comparacao_automatica_contra_implementacao_padrao.c | 0 | 0 | 2048 | PASS |

**Resultado: PASS — todos os arquivos dentro do limite.**
