# Gaps ApkC — estado canônico após HOTFIX operacional

> Corte: 2026-07-27. Regra: documento não promove estado acima do artefato e do gate executado. Neste branch, `claim_allowed=false` até conclusão dos workflows bloqueantes.

| Frente | Estado atual | Evidência / limite |
|---|---|---|
| Lowering do núcleo `raf_compile` | **IMPLEMENTED_HOTFIX / CI_PENDING** | Fonte controla `ir_value`; exatamente uma expressão válida; múltiplos retornos, operadores malformados e comentários que escondem tokens falham fechados. |
| Emissão x86-64/ARM64/ARM32/RV64 | **IMPLEMENTED** | Imediato real codificado por arquitetura. Host atual, ARM64 e ARM32 entram nos gates; execução x86-64/RV64 no target permanece condicional. |
| Recibo `.ops` | **SCHEMA 4 / HOTFIXED / CI_PENDING** | FNV-1a 64 canônico; produtor e validador assinam os mesmos campos; inclui `ir_value`, emissor e estado transacional. |
| Rollback | **IMPLEMENTED / BLOCKING TEST** | Falha remove `.s/.hex/.bin` anteriores do mesmo `out_base` e deixa somente recibo `ROLLED_BACK`. |
| Compatibilidade C sem libc final | **IMPLEMENTED_HOTFIX / TESTED_BY_SUITE** | `memmove` sem UB relacional; novas rotas `memchr/strcpy/strncpy`; `strtoul` delimitado; heap proibido. |
| Rewriter C/C++ | **FAIL-CLOSED** | Só remove headers cuja superfície está emulada. Header hospedado ou include local sem resolução falha antes do objeto. |
| C estrito ARM64 | **IMPLEMENTED / BLOCKING TEST** | Android ELF64 AArch64 selado, sem `DT_NEEDED`, `PT_INTERP`, símbolo indefinido, RWX ou pilha executável. |
| C estrito ARM32 | **IMPLEMENTED / BLOCKING TEST** | Android ELF32 ARM com o mesmo contrato e máquina validada. |
| C++ estrito ARM64 | **IMPLEMENTED / BLOCKING TEST** | `RAF_EXPORT` usa `extern "C"`; entrypoints e kernel não podem ser mangled. |
| Linguagens hospedadas → kernel puro | **14 ROTAS IMPLEMENTADAS / BLOCKING TEST** | `RAF_KERNEL` baixa para C estrito. Não equivale à semântica completa da linguagem. |
| Recibo `.so.receipt.json` | **IMPLEMENTED / BLOCKING TEST** | SHA-256 da fonte/saída, toolchain, gates, dependências de build e claims explicitamente limitados. |
| Auditoria ELF | **DOIS PERFIS IMPLEMENTADOS** | `exec` proíbe `PT_DYNAMIC`; `android-so` permite metadata do loader e proíbe dependências externas. |
| Inventário canônico | **IMPLEMENTED** | `ci/contracts/apkc_compiler_station_v2.json` + validador bloqueante. |
| Build reproduzível de `apkc.c` empacotador | **GATE EXISTENTE / NÃO CONFUNDIR COM ESTAÇÃO RAIZ** | O compilador/selador estrito está separado do empacotador APK monolítico. |
| Geração de `hello.apk` no run atual | **TOKEN_VAZIO** | Exige empacotar os `.so` selados e produzir APK no mesmo run. |
| Parser ZIP do APK atual | **TOKEN_VAZIO** | Regenerar `unzip.txt` do APK canônico recém-produzido. |
| Parser AXML do APK atual | **TOKEN_VAZIO** | Regenerar `aapt-xmltree.txt` no mesmo run. |
| ELF ARM32/ARM64 dentro do APK | **TOKEN_VAZIO** | `.so` selado isolado não prova inserção e extração do APK. |
| DEX mínimo estrutural | **IMPLEMENTED** | Não prova classe Java/Kotlin funcional nem runtime. |
| Assinatura APK | **REFERENCE HISTÓRICA** | Deve ser regenerada sobre o APK do run atual; chave release permanece fora do repositório. |
| Instalação e runtime NativeActivity | **TOKEN_VAZIO** | Capturar instalação, launch, `dlopen`, `ANativeActivity_onCreate` e `logcat` sem fatal. |
| GPU/DSP/NPU | **DEVICE_KERNEL / RUNTIME TOKEN_VAZIO** | Asset/kernel não prova driver, loader nem execução física. |
| Reprodutibilidade APK→runtime | **TOKEN_VAZIO** | Um run único deve registrar commit, ambiente, ELF, APK, assinatura, instalação e runtime. |

## Invariantes

```text
HOTFIX_IMPLEMENTED ≠ HOTFIX_VERIFIED_BY_CI
HOTFIX_VERIFIED_BY_CI ≠ APK_RUNTIME_PROVEN
BUILD_PLANE_DEPENDENCY ≠ FINAL_RUNTIME_DEPENDENCY
PT_DYNAMIC_ANDROID_METADATA ≠ DT_NEEDED_EXTERNAL_RUNTIME
KERNEL_LOWERED ≠ FULL_LANGUAGE_IMPLEMENTED
FILE_EXISTS ≠ PASS
ROLLED_BACK → NO_STALE_EXECUTABLE_ARTIFACT
TOKEN_VAZIO ≠ ZERO
```
