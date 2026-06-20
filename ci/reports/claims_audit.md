# Claims Traceability Audit — B7

Data: 2026-06-20T08:02:16Z

| Arquivo | Linha | Claim | Status |
|---------|:-----:|-------|--------|
| docs/APKC_FLAGS_LIMITS_AND_COMMANDS.md | 179 | `Se quiser aceitar C no futuro, precisa de um frontend C real...` | AUDIT (sem ref rastreável) |
| docs/APKC_FLAGS_LIMITS_AND_COMMANDS.md | 326 | `print("PASS" if header == calc else "FAIL")...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 5 | `| F0 | source exists | PASS | Apkc/hello.s.txt |...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 7 | `| F2 | generate hello.apk | PASS | Apkc/proofs/out/apkc-gene...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 8 | `| F3 | unzip parses | PASS | Apkc/proofs/out/unzip.txt |...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 9 | `| F4 | AXML parses | PASS | Apkc/proofs/out/aapt-xmltree.txt...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 10 | `| F5 | DEX SHA-1 matches | PASS | Apkc/proofs/out/dex-sha1.t...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 11 | `| F6 | ELF readelf parses | PASS/SKIP | readelf-arm32.txt PA...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 12 | `| F7 | APK signs | PASS | Apkc/proofs/out/apksigner-verify.t...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 13 | `| F8 | APK installs/package visible | PASS limitado | Apkc/p...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 15 | `| F10 | proof archived | PASS | Apkc/proofs/CHAIN_OF_CUSTODY...` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 19 | `- PASS exige comando executado e artefato verificavel....` | AUDIT (sem ref rastreável) |
| docs/APKC_PROTOCOL.md | 24 | `- PASS limitado registra evidencia positiva parcial sem tran...` | AUDIT (sem ref rastreável) |
| docs/APKC_SIGNING_POLICY.md | 8 | `> chaves e definição honesta do que hoje é `PASS`, o que ...` | AUDIT (sem ref rastreável) |
| docs/APKC_SIGNING_POLICY.md | 31 | `| **Debug keystore** (`CN=ApkC Debug, O=Rafael, C=BR`, RSA 2...` | AUDIT (sem ref rastreável) |
| docs/APKC_SIGNING_POLICY.md | 55 | `Leitura honesta: o que está `PASS` (v1/v2/v3) é assinatura...` | AUDIT (sem ref rastreável) |
| docs/APKC_SIGNING_POLICY.md | 109 | `permanece `PENDING`/`TOKEN_VAZIO` — não se converte em `P...` | AUDIT (sem ref rastreável) |
| docs/APKC_TARGET_ENVIRONMENTS.md | 39 | `  runtime ainda `PASS limitado`, ver `docs/LACUNAS_PROFUNDAS...` | AUDIT (sem ref rastreável) |
| docs/APKC_TARGET_ENVIRONMENTS.md | 54 | `| **Stock Android (NativeActivity)** | OK em escopo (nativo ...` | AUDIT (sem ref rastreável) |
| docs/APKC_TERMUX_ARM32_PROOF.md | 99 | `print("PASS" if header == calc else "FAIL")...` | AUDIT (sem ref rastreável) |
| docs/APKC_TERMUX_ARM32_PROOF.md | 108 | `PASS...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 24 | `| Assinatura APK | `apksigner verify --verbose` | PASS (v1/v...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 25 | `| Instalação real | `adb install -r` em device autorizado ...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 48 | `> "destrava" quando os gates abaixo estão **PASS** com arte...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 50 | `> Regra de honestidade: nenhum gate é marcado PASS sem arte...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 55 | `| Faixa heurística | Gates/artefatos que devem estar PASS |...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 57 | `| **US$ 25k–75k** (atual) | Arquitetura + auditoria freest...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 66 | `- **P(k) falsifiability gate** — bloqueante, veredicto `PA...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 70 | `- **verbovivo build + smoke** — `PASS` (step...` | AUDIT (sem ref rastreável) |
| docs/APKC_VALUE_AND_GAPS.md | 73 | `- **API/ABI matrix** — minSdkVersion provado nos bytes do ...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 154 | `## Coverage matrix — EXECUTA_PASS / COMPILE_OK / TOKEN_VAZ...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 157 | `- **EXECUTA_PASS**: compila, linka, executa, selftest retorn...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 158 | `- **COMPILE_OK**: compila e linka sem erro; execucao nao ver...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 164 | `| M001 GPIO DDRx/PORTx/PINx          | COMPILE_OK     | COMP...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 165 | `| M002 Toggle PINx                   | COMPILE_OK     | COMP...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 166 | `| M003 Timer CTC                     | COMPILE_OK     | COMP...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 167 | `| M008 ADC free-running              | COMPILE_OK     | COMP...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 168 | `| M009 ADC oversampling              | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 169 | `| M011 ADC IIR fixed-point           | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 170 | `| M013 UART ring buffer              | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 171 | `| M021 GPIO mmap /dev/mem            | TOKEN_VAZIO    | TOKE...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 172 | `| M022 GPIO /dev/gpiomem             | TOKEN_VAZIO    | TOKE...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 173 | `| M024 cntvct_el0 counter            | N/A            | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 174 | `| M026 DMB memory barrier            | N/A            | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 175 | `| M027 DSB memory barrier            | N/A            | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 176 | `| M028 ISB memory barrier            | N/A            | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 177 | `| M029 SPI registrador BCM           | TOKEN_VAZIO    | TOKE...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 178 | `| M032 DMA control block chain       | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 179 | `| M033 DMA circular                  | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 180 | `| M036 Thread affinity               | TOKEN_VAZIO    | TOKE...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 181 | `| M039 p95/p99 latency               | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 182 | `| M040 Jitter measurement            | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 183 | `| M041 JNI bridge                    | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 184 | `| M045 ABI detection runtime         | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 185 | `| M046 Syscall direta SYS_gettid     | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 186 | `| M047 Ring buffer JNI               | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 187 | `| raf_flags_bitmask (S16)            | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 188 | `| raf_crc8_buf LUT (S17)             | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 189 | `| crc32c_buf SSE4.2                  | EXECUTA_PASS   | N/A ...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 190 | `| crc32c_buf CRC32CX (ARM64)         | N/A            | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 191 | `| phi64_mix hash chain               | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BENCHMARK_VISUAL.md | 192 | `| raf_runtime_route dispatch bus     | EXECUTA_PASS   | EXEC...` | AUDIT (sem ref rastreável) |
| docs/BRANCH_PROTECTION_MAIN_POLICY.md | 7 | `Preservar o estado funcional já provado do ApkC:...` | AUDIT (sem ref rastreável) |
| docs/BRANCH_PROTECTION_MAIN_POLICY.md | 63 | `print('DEX_SHA1_PASS')...` | AUDIT (sem ref rastreável) |
| docs/BRANCH_PROTECTION_MAIN_POLICY.md | 76 | `Se alguma ferramenta estiver ausente, registrar `TOKEN_VAZIO...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 8 | `Esta passada não inventa PASS. Onde o arquivo existe mas o ...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 23 | `| M024 | `RAF_024_leitura_de_contador_arm64_cntvct_el0.c` | ...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 24 | `| M048 | `RAF_048_log_binario_em_vez_de_log_textual_pesado.c...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 25 | `| M050 | `RAF_050_exportacao_de_resultado_em_json.c` | conve...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 26 | `| M052 | `RAF_052_probe_de_hot_path_no_qemu_tcg.c` | struct ...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 27 | `| M054 | `RAF_054_batching_de_operacoes_repetidas.c` | compa...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 28 | `| M055 | `RAF_055_cache_local_de_resultado_tecnico.c` | cach...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 29 | `| M056 | `RAF_056_comparacao_automatica_contra_implementacao...` | AUDIT (sem ref rastreável) |
| docs/CHAIN_OF_CUSTODY_2026-06-16_CHECKLIST_RECONCILE.md | 38 | `- **M051, M053**: compartilham o corpo de M052 (struct de co...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 11 | `- Estado: PASS (medido), ESTIMATE (não medido aqui), TOKEN_...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 20 | `| `clock_gettime(CLOCK_MONOTONIC)` | 25–50 | TOKEN_VAZIO |...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 22 | `| `__sync_synchronize()` (x86) | — | — | 1–5 | **38 ci...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 24 | `| `syscall(SYS_gettid)` direto | — | — | 100–300 | **2...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 28 | `| IIR fixed-point 1 iteração (M011) | 3–6 | TOKEN_VAZIO ...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 29 | `| Ring buffer push/pop (M047/M013) | 2–5 | TOKEN_VAZIO | 1...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 31 | `| Bus throughput (S23, baseline) | — | — | — | PASS (v...` | AUDIT (sem ref rastreável) |
| docs/CICLOS_ESTIMADOS_VS_MEDIDOS.md | 32 | `| Batching ×8 vs individual (M054) | — | — | — | PASS...` | AUDIT (sem ref rastreável) |
| docs/CODEX_FIX_PROTOCOL.md | 80 | `## Padrões de correção aprovados...` | AUDIT (sem ref rastreável) |
| docs/COERENCIA_FECHAMENTO_SESSAO_APKC.md | 209 | `  verbovivo.c: VOID (citado em CLAUDE.md, não implementado)...` | AUDIT (sem ref rastreável) |
| docs/COERENCIA_FECHAMENTO_SESSAO_APKC.md | 215 | `  verbovivo.c: implementado — Fiber-H (Layer 1) + T^7 toro...` | AUDIT (sem ref rastreável) |
| docs/CONVERGENCIA_UNICA_METODOLOGICA.md | 34 | `metáfora pedagógica ≠ mecanismo comprovado...` | AUDIT (sem ref rastreável) |
| docs/CONVERGENCIA_UNICA_METODOLOGICA.md | 272 | `| Prova | logs APKc/proofs | traceability matrix | release e...` | AUDIT (sem ref rastreável) |
| docs/CONVERGENCIA_UNICA_METODOLOGICA.md | 298 | `PASS       = execução ou arquivo validado com origem...` | AUDIT (sem ref rastreável) |
| docs/ESTADO_ATUAL_E_VALOR_CONSERVADOR.md | 29 | `APK existe != runtime provado...` | AUDIT (sem ref rastreável) |
| docs/ESTADO_ATUAL_E_VALOR_CONSERVADOR.md | 61 | `| 39 mnemonics ARM32 | `PENDING_CATALOG` | lista formal: imp...` | AUDIT (sem ref rastreável) |
| docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md | 32 | `| Genérico C | baseline, portabilidade, auditoria | quando ...` | AUDIT (sem ref rastreável) |
| docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md | 82 | `O seletor runtime implementado em `Benchmark/raf_runtime_rou...` | AUDIT (sem ref rastreável) |
| docs/EXCELENCIA_OPERACIONAL_GPU_SIMD_GOVERNANCA.md | 93 | `O arquivo `assets/raf_operational_seal.svg` é um selo visua...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 6 | `> Este documento não é avaliação financeira. É mapa de ...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 13 | `O projeto já sabe dizer a verdade: `PASS`, `TOKEN_VAZIO`, `...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 29 | `| Prova source→binary | ✅ PASS (2026-06-17) | `tools/raf...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 31 | `| ARM64 ELF validado | ◐ ELF AArch64 provado | `.so` em AP...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 36 | `| Java/DEX pipeline | ◐ javac PASS, d8 TOKEN_VAZIO | `scri...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 39 | `> lacunas (10 PASS, 5 AVANÇADO, 3 TOKEN_VAZIO honesto com c...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 54 | `| Código-fonte em commit | PASS — `Apkc/apkc.c` no reposi...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 55 | `| Hash do binário apkc | PASS — SHA-256 em `CHAIN_OF_CUST...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 81 | `| Pacote instalado/visível | PASS limitado |...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 98 | `**O gap:** `adb-install.txt` registra `package:com.rafael.te...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 117 | `| ELF ARM32 readelf | PASS — `readelf-arm32.txt` |...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 215 | `| ASM  | use_asm | TOKEN_VAZIO¹ | PASS | PASS limitado |...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 301 | `| `grep -n "malloc\|free\|calloc" rafaelia/verbovivo.c` | PA...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 325 | `PASS — build limpo (exit 0), execução sem crash, saída ...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 335 | `Em produto, isso deve ser `FAIL` ou `PASS_LIMITED` explícit...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 449 | `quando `verdict == 'FAIL'`. Veredicto atual: `PASS` (rrmse=0...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 486 | `> decide o que é `PASS` real versus `TOKEN_VAZIO` honesto c...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 494 | `| ✅ PASS (código/prova/doc) | L1, L5, L6, L10, L12, L13, ...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 502 | `| **L1** source→binary | ✅ PASS | `tools/raf_source_to_b...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 506 | `| **L5** 39 mnemonics ARM32 | ✅ PASS | `Apkc/arch_arm32.h`...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 507 | `| **L6** artefatos inconsistentes | ✅ PASS | `tools/raf_cl...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 511 | `| **L10** caminhos de intérprete | ✅ PASS (doc) | `docs/A...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 512 | `| **L11** Java/Kotlin DEX | ◐ AVANÇADO | `scripts/java_de...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 513 | `| **L12** catálogo 56 RAF | ✅ PASS | `RAF_INDEX.md` reali...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 514 | `| **L13** raf_compile IR | ✅ PASS (reposicionado) | `raf_p...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 515 | `| **L16** erro bloqueante vs degradação | ✅ PASS | `Apkc...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 516 | `| **L17** corpus regressivo | ◐ AVANÇADO | `tests/fixture...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 517 | `| **L18** assinatura release | ✅ PASS (política) | `docs/...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 518 | `| **L19** release navegável | ✅ PASS | `Makefile` raiz: `...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 519 | `| **L20** valuation→gates | ✅ PASS (doc) | `docs/APKC_VA...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 524 | `python3 tests/test_arm32_encoders.py        # L5  → 16 PAS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 525 | `python3 tests/test_format_fixtures.py        # L17 → 3 PAS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 526 | `python3 tests/test_zip_negative.py           # L17 → PASS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 527 | `bash tools/raf_source_to_binary_proof.sh     # L1  → 2 PAS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 528 | `bash tools/raf_clean_proof_run.sh            # L6  → 5 PAS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 529 | `bash scripts/java_dex_pipeline_probe.sh      # L11 → 1 PAS...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 530 | `make encoders verbovivo-demo syntax          # L19 → todos...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 537 | `- Sem libc em `Apkc/` — `clang -fsyntax-only -nostdlib -no...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 539 | `- `TOKEN_VAZIO` nunca convertido em `PASS` por omissão (L2/...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 573 | `- Nenhum gate marcado PASS sem artefato verificável...` | AUDIT (sem ref rastreável) |
| docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md | 574 | `- TOKEN_VAZIO é estado legítimo; nunca converter em PASS p...` | AUDIT (sem ref rastreável) |
| docs/OPERACAO_COMPILAR_PRECOMPILAR.md | 31 | `## Mitigações implementadas...` | AUDIT (sem ref rastreável) |
| docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md | 38 | `1. `EXEC_PASS`: comando executado e evidência gravada....` | AUDIT (sem ref rastreável) |
| docs/PROTOCOLO_DOIS_CICLOS_OMEGA.md | 49 | `| 3 | Medir coerência e entropia | Compilar/testar sem heap...` | AUDIT (sem ref rastreável) |
| docs/PROTOCOLO_FALSIFICABILIDADE_PK.md | 42 | `- PASS se:...` | AUDIT (sem ref rastreável) |
| docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md | 13 | `5. o que pode ser implementado no RafPolimata como validaç�...` | AUDIT (sem ref rastreável) |
| docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md | 72 | `| C0 | Formulação | “Existe uma formulação matemática...` | AUDIT (sem ref rastreável) |
| docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md | 82 | `C0 = PASS...` | AUDIT (sem ref rastreável) |
| docs/RAFAELIA_PAPER_MARKET_7_VECTORS.md | 304 | `> Está provado que existem exatamente 42 atratores em todos...` | AUDIT (sem ref rastreável) |
| docs/ROTINA_OPERACIONAL_BENCHMARKS.md | 11 | `5. **TOKEN_VAZIO**: quando falta evidência, registrar `VOID...` | AUDIT (sem ref rastreável) |
| docs/ROTINA_OPERACIONAL_BENCHMARKS.md | 49 | `A rotina para somente quando os gates bloqueantes passam, o ...` | AUDIT (sem ref rastreável) |
| docs/arch/ANDROID_NDK.md | 188 | `This template is embedded as a string constant in RAF_042 an...` | AUDIT (sem ref rastreável) |
| docs/arch/BCM2835.md | 234 | `#define CM_PASSWD  0x5A000000u...` | AUDIT (sem ref rastreável) |
| docs/arch/BCM2835.md | 257 | `cm_base[CM_PWMDIV_OFFSET] = CM_PASSWD | (5u << 12);...` | AUDIT (sem ref rastreável) |
| docs/arch/BCM2835.md | 264 | `2. Kill CM clock: write `CM_PASSWD | CM_PWMCTL_KILL` to CM_P...` | AUDIT (sem ref rastreável) |
| docs/arch/BCM2835.md | 266 | `4. Set new divisor: write `CM_PASSWD | (DIVI << 12)` to CM_P...` | AUDIT (sem ref rastreável) |
| docs/arch/BCM2835.md | 267 | `5. Enable clock with source: write `CM_PASSWD | CM_PWMCTL_EN...` | AUDIT (sem ref rastreável) |

## Resumo

| PASS | AUDIT | FAIL |
|:----:|:-----:|:----:|
| 6 | 150 | 0 |

Claims AUDIT: precisam de `(ref: arquivo:linha)` para ser promovidos a PASS.
