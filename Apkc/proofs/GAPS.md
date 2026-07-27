# Gaps ApkC — estado canônico após fechamento da estação do compilador

> Corte: 2026-07-26. Regra: documento não promove estado acima do artefato existente.

| Frente | Estado atual | Evidência / limite |
|---|---|---|
| Lowering do núcleo `raf_compile` | **IMPLEMENTED / TESTED** | A fonte controla o valor IR; removido o emissor fixo `42`. Expressões constantes inválidas falham fechadas. |
| Emissão x86-64/ARM64/ARM32/RV64 | **IMPLEMENTED / TESTED** | Imediato real é codificado por arquitetura; mesma fonte deve repetir bytes e fontes diferentes devem divergir. |
| Compatibilidade C sem libc final | **IMPLEMENTED / TESTED** | `raf_libc_emu.h` + rewriter; memória/string/conversão/saída mínima internas; heap e runtime hospedado bloqueados. |
| C estrito ARM64 | **IMPLEMENTED / TESTED_LOCAL / CI_GATED** | ELF64 AArch64, `-nostdlib`, `--no-undefined`, sem `PT_INTERP`. |
| C estrito ARM32 | **IMPLEMENTED / TESTED_LOCAL / CI_GATED** | ELF32 ARM, mesmo contrato, fallback não permitido. |
| Linguagens hospedadas → kernel puro | **IMPLEMENTED / TESTED** | `RAF_KERNEL` é baixado para C estrito. Não equivale a compilar a linguagem inteira. |
| Build reproduzível de `apkc.c` | **GATE CORRIGIDO** | CI usa `clang -target aarch64-linux-gnu -fsyntax-only` e falha bloqueante. |
| Geração de `hello.apk` no run atual | **TOKEN_VAZIO** | Requer binário APKc executável no ambiente Android/Termux e empacotamento no mesmo run. |
| Parser ZIP do APK atual | **TOKEN_VAZIO** | Regenerar `unzip.txt` a partir do APK produzido no mesmo run. |
| Parser AXML do APK atual | **TOKEN_VAZIO** | Regenerar `aapt-xmltree.txt` no mesmo run. |
| ELF ARM32/ARM64 dentro do APK | **TOKEN_VAZIO** | Os `.so` estritos existem como rota; ainda precisam ser inseridos e extraídos do APK canônico. |
| DEX mínimo estrutural | **IMPLEMENTED** | Continua distinto de classe Java/Kotlin funcional. |
| Assinatura APK | **REFERENCE HISTÓRICA** | Regenerar sobre o APK do run atual; chave release fora do repositório. |
| Instalação e runtime NativeActivity | **TOKEN_VAZIO** | Capturar `adb install`, launch, `dlopen`, `ANativeActivity_onCreate` e `logcat` sem fatal. |
| GPU/DSP/NPU | **DEVICE_KERNEL / RUNTIME TOKEN_VAZIO** | Kernel/asset não prova driver, loader nem execução física. |
| Reprodutibilidade completa APK→runtime | **TOKEN_VAZIO** | Um run deve registrar commit, ambiente, comandos, ELF, APK, assinatura, instalação e runtime. |

## Invariante

```text
COMPILER_STATION_PASS ≠ APK_RUNTIME_PROVEN
KERNEL_LOWERED ≠ FULL_LANGUAGE_IMPLEMENTED
FILE_EXISTS ≠ PASS
TOKEN_VAZIO ≠ ZERO
```
