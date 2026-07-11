# Estado Atual e Valor Conservador — RafPolimata

> **Entrada canônica:** `docs/AGENTES.md` §3 (estados canônicos — TOKEN_VAZIO, PENDING, AUDIT)
> e §5 (pipeline operacional VOID → VALIDATED). Este documento avalia o estado atual do RafPolimata
> e valor conservador por estágio — sem inflar claim e sem apagar lacunas.

Data: 2026-06-15  
Status: `AUDITABLE_BUSINESS_TECH_STATE`  
Escopo: avaliação conservadora do ativo técnico, sem inflar claim e sem apagar lacunas.

---

## 1. Tese honesta

O RafPolimata não deve ser descrito como produto pronto. A frase correta hoje é:

```text
RafPolimata é um núcleo técnico autoral em C para geração de APKs Android mínimos,
com assembler ARM64/ARM32, matriz multi-linguagem, componentes de runtime/validação
e sementes matemáticas, mas ainda sem prova Android full-chain reproduzível.
```

A frase proibida por enquanto é:

```text
Produto Android completo, validado em ARM64 real e pronto para mercado.
```

Motivo:

```text
código existe != execução provada
APK existe != runtime provado
CI x86_64 != Android real
semente matemática != novidade acadêmica
TOKEN_VAZIO != fracasso; é lacuna honesta
```

---

## 2. O que existe

| Camada | Estado | Evidência esperada/observada |
|---|---|---|
| `Apkc/apkc.c` | `PRESENT` | assembler/parser/build APK, CLI `-64/-32/-both`, geração de libs por ABI |
| `Apkc/arch_arm64.h` | `PRESENT_STRONG` | encoders ARM64, incluindo LDRSW, LDR literal, LD2/3/4, ST2/3/4, MRS/MSR |
| `Apkc/arch_arm32.h` | `PRESENT_WITH_GAP` | ARM32 existe, mas cobertura de mnemonics ainda requer catálogo formal |
| `Apkc/lang_profile.h` | `PRESENT` | 12 linguagens declaradas |
| `raf_compile.h` | `PRESENT` | `RAF_LANG_COUNT=12`, `RAF_CAP_MATRIX[12][5]`, `raf_cap_query()` |
| `rafaelia/verbovivo.c` | `PRESENT` | T7/Fiber-H/SVG, trajetória em stack, sem `malloc` no caminho ajustado |
| `Benchmark/` | `PRESENT` | auxiliares de arena/FSM/hash/CRC32C/Q16/T7 |
| `RAF_001..RAF_056` | `PRESENT_AS_SKELETONS` | firmware AVR/ARM bare-metal; MMIO skeleton intencional em host |
| Sementes matemáticas | `PRESENT_AS_HYPOTHESES` | `√3/2`, `F*`, `T^7`, `D2`, `n_crítico(L)` |

---

## 3. O que ainda não existe como prova forte

| Lacuna | Estado | Como fechar |
|---|---|---|
| Transcript de compilação do `apkc` | `TOKEN_VAZIO` | `01_compile_apkc.txt` no mesmo commit/run |
| ARM64 ELF real dentro do APK | `TOKEN_VAZIO` até `readelf` real | `05_readelf_arm64.txt` com `ELF64` + `AArch64` |
| Runtime NativeActivity | `TOKEN_VAZIO` | `install + launch + logcat` sem crash/fatal |
| Reprodutibilidade source→APK | `TOKEN_VAZIO` | `manifest.json` com commit, env, sha, logs |
| 39 mnemonics ARM32 | `PENDING_CATALOG` | lista formal: implementado / unsupported / trap |
| 12 linguagens provadas | `PENDING_MATRIX_RUN` | rodada por linguagem com status por host/ABI |
| novidade acadêmica das sementes | `PENDING_LITERATURE_REVIEW` | busca formal + referências + contraexemplos |
| produto comercial | `PENDING_PRODUCTIZATION` | docs, release, demo, usuário, suporte, packaging |

---

## 4. Valor conservador por estágio

Esta tabela é avaliação conservadora, não promessa de venda.

| Estágio | Condição | Valor/valuation conservador |
|---|---|---:|
| Código bruto privado | repo técnico sem prova full-chain | `US$0 – US$10k` realizável |
| Ativo técnico autoral | C/ASM/APK/matriz/docs/provas parciais | `US$15k – US$75k` valor técnico |
| MVP técnico demonstrável | run full-chain com ARM64/logcat/hash | `US$75k – US$250k` valor técnico demonstrável |
| Pre-seed story | demo reproduzível + pitch + roadmap + riscos claros | `US$250k – US$1M` valuation narrativo possível |
| Produto devtool inicial | usuários reais, release, docs, suporte, casos de uso | `US$1M – US$5M` valuation inicial possível |
| Empresa com receita | receita recorrente e retenção | múltiplos de receita; hoje `TOKEN_VAZIO` |

Regra:

```text
O valor sobe menos por mais texto e sobe mais por menos TOKEN_VAZIO.
```

---

## 5. Multiplicadores de valor

| Multiplicador | Impacto | Motivo |
|---|---|---|
| `proofs/run-arm64-full-chain/out/manifest.json` completo | Alto | transforma narrativa em cadeia auditável |
| `readelf` ARM64 real | Alto | prova relevância Android moderno |
| `logcat` sem crash | Alto | prova runtime, não só empacotamento |
| matriz 12 linguagens com runs | Médio/alto | valida promessa multi-linguagem |
| release com artefatos pequenos e hashes | Médio | facilita auditoria externa |
| demo curta gravável | Médio/alto | reduz fricção de entendimento |
| revisão bibliográfica das sementes | Médio | protege contra claim inflado |
| usuários externos | Muito alto | muda de ativo técnico para produto |

---

## 6. Frase de pitch conservadora

```text
RafPolimata é um compilador/runtime experimental para Android que gera APKs mínimos
a partir de fontes multi-linguagem, com assembler ARM32/ARM64 próprio, cadeia de
integridade e integração com um núcleo geométrico T7/Fiber-H. Hoje é um MVP técnico
autoral com provas parciais; o próximo marco é fechar uma rodada ARM64 full-chain
com compile log, APK hash, readelf, install, launch e logcat no mesmo commit.
```

---

## 7. Checklist de promoção para `MVP_TECH_PROVED`

```text
[ ] 00_env.txt contém commit, branch, device, toolchain
[ ] 01_compile_apkc.txt contém comando e stdout/stderr completos
[ ] 01_apkc.sha256 existe
[ ] 02_generate_apk.txt contém geração sem erro bloqueante
[ ] 02_apk.sha256 existe
[ ] 03_unzip_list.txt mostra AndroidManifest.xml e classes.dex
[ ] 03_unzip_list.txt mostra lib/arm64-v8a/lib*.so
[ ] 05_readelf_arm64.txt mostra ELF64/AArch64
[ ] 08_install.txt ou 08_adb_install.txt mostra instalação bem-sucedida
[ ] 09_launch.txt mostra tentativa de launch
[ ] 10_logcat_nativeactivity.txt não mostra FATAL EXCEPTION/crash relevante
[ ] manifest.json amarra todos os artefatos ao mesmo commit/run
```

Quando todos forem satisfeitos:

```text
status: MVP_TECH_PROVED
claim permitido: gerou, instalou e lançou APK mínimo Android com ARM64 rastreado
claim ainda proibido: produto comercial completo ou sistema científico validado universalmente
```

---

## 8. Próximas ações no repositório

| Ordem | Ação | Arquivo |
|---|---|---|
| 1 | Rodar captura full-chain | `scripts/capture_android_proof_chain.sh` |
| 2 | Commitar logs pequenos | `proofs/run-arm64-full-chain/out/*.txt` |
| 3 | Revisar `status.tsv` e promover somente gates com evidência | `proofs/run-arm64-full-chain/out/status.tsv` |
| 4 | Catálogo ARM32 mnemonics | `docs/ARM32_MNEMONIC_COVERAGE.md` |
| 5 | Matriz 12 linguagens | `proofs/lang-matrix/` |
| 6 | Release técnico | `CHANGELOG.md` + GitHub Release |

---

## 9. Retroalimentação

```text
F_ok:
- existe núcleo técnico autoral;
- existe mapa de gaps;
- existe protocolo de captura full-chain;
- valor conservador pode ser defendido sem inflar.

F_gap:
- sem run ARM64 real, valor fica preso no estágio de ativo técnico;
- sem logcat, runtime continua TOKEN_VAZIO;
- sem usuário/receita, valuation comercial continua narrativo.

F_next:
- executar a captura full-chain em ARM64 real;
- commitar logs pequenos;
- transformar a rodada em release demonstrável.
```

ΣΩΔΦBITRAF · prova antes do claim.
