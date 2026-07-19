# Gaps ApkC — estado canônico reconciliado

> Corte: 2026-07-19. Regra: documento não promove estado acima do artefato
> existente. Provas antigas continuam preservadas em `Apkc/proofs/runs/`, mas os
> arquivos canônicos de `Apkc/proofs/out/` governam o estado atual.

| Gap | Estado atual | Evidência / próxima ação |
|---|---|---|
| Build reproduzível de `apkc.c` | **CONTRADICTION → GATE CORRIGIDO** | O transcript canônico anterior contém erros de compilação. `tools/raf_source_to_binary_proof.sh` v2 agora falha fechado, cria run isolado e só promove PASS quando AArch64+ARM32 constroem, identificam e reproduzem. Executar novamente. |
| Geração de `hello.apk` | **TOKEN_VAZIO** | `Apkc/proofs/out/apkc-generate.txt` declara ausência de binário executável. Rodar em ARM64/Termux após o source→binary PASS. |
| Mnemônicos ARM32 desconhecidos | **PARTIAL / STRICT GATE** | O compilador bloqueia APK por padrão quando encontra instrução desconhecida; ampliar cobertura conforme corpus real. |
| Parser ZIP | **TOKEN_VAZIO neste corte** | Regerar `unzip.txt` no mesmo run e commit do APK. |
| Parser AXML | **TOKEN_VAZIO neste corte** | Regerar `aapt-xmltree.txt` no mesmo run e commit do APK. |
| ELF ARM32 dentro do APK | **TOKEN_VAZIO** | `readelf-arm32.txt` diz que `hello.apk` está ausente. |
| ELF ARM64 dentro do APK | **TOKEN_VAZIO** | `readelf-arm64.txt` diz que `hello.apk` está ausente. |
| Gerador ELF estrutural | **IMPLEMENTED / RUNTIME PENDING** | `Apkc/fmt_elf.h` contém geradores ELF32/ELF64; falta validar o `.so` realmente empacotado e carregado pelo Android. |
| DEX mínimo estrutural | **IMPLEMENTED** | `Apkc/fmt_dex.h` gera DEX 035 mínimo de 140 bytes com SHA-1 e Adler-32. Isso não prova classe Java/Kotlin funcional. |
| DEX SHA-1 do APK atual | **TOKEN_VAZIO** | `dex-sha1.txt` declara `hello.apk` ausente. Regerar no mesmo run. |
| Java/Kotlin/Groovy → JAR → D8 | **IMPLEMENTED / RUNTIME PENDING** | Java/Groovy agora usam wrappers de JAR; `execve` resolve ferramentas Termux/Android. Falta executar toolchains e validar `dexdump`/runtime. |
| Assinatura APK | **REFERENCE HISTÓRICA** | Há transcript anterior, mas deve ser regenerado sobre o APK do mesmo run atual. Release key permanece fora do repositório. |
| Package instalado/visível | **REFERENCE HISTÓRICA** | Não equivale a runtime. Capturar `adb install -r` integral no mesmo run. |
| Runtime NativeActivity | **TOKEN_VAZIO** | Capturar lançamento, `dlopen`, `ANativeActivity_onCreate` e logcat sem fatal em aparelho autorizado. |
| Navegador web ASM + TLS 1.2/1.3 + X.509 | **TOKEN_VAZIO / NÃO LOCALIZADO** | A árvore contém `raf_shell` como navegador de arquivos TUI, não prova cliente HTTP/TLS. Identificar arquivo/repositório de origem ou implementar camada separada com validação de certificado. |
| Mapa de arquivos soltos | **IMPLEMENTADO NESTA FRENTE** | `scripts/apkc_first_part_gate.py` gera inventário por hash, categoria, referência e rota documental. |
| Reprodutibilidade completa | **TOKEN_VAZIO** | Um único run deve registrar commit, ambiente, comandos, binário, APK, ELF32/64, DEX, assinatura, instalação e runtime. |

## Invariante

```text
IMPLEMENTED ≠ EXECUTED ≠ RUNTIME_PROVEN
FILE_EXISTS ≠ PASS
HISTORICAL_REFERENCE ≠ CURRENT_CANONICAL_EVIDENCE
```
