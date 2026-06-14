# Checklist de validação — hello.apk

> Escopo: validar `hello.apk` gerado a partir de `Apkc/hello.s.txt`, sem afirmar instalação ou runtime sem prova real.

| Gate | Checklist | Status | Evidência |
|---|---:|---|---|
| F0 source exists | `Apkc/hello.s.txt` existe | NOT_RUN | `Apkc/proofs/out/validation-summary.md` |
| F1 compile apkc | `apkc.c` compila | NOT_RUN | `Apkc/proofs/out/validation-summary.md` |
| F2 generate hello.apk | `hello.apk` gerado | NOT_RUN | `Apkc/proofs/out/hello.apk` |
| F3 unzip parses | `unzip -l` lê o APK | NOT_RUN | `Apkc/proofs/out/unzip.txt` |
| F4 AXML parses | `aapt dump xmltree` lê manifest | TOKEN_VAZIO | `Apkc/proofs/out/aapt-xmltree.txt` |
| F5 DEX SHA-1 matches | checksum interno DEX confere | NOT_RUN | `Apkc/proofs/out/dex-sha1.txt` |
| F6 ELF readelf parses | `.so` arm64/arm32 lido por `readelf` | NOT_RUN | `Apkc/proofs/out/readelf-*.txt` |
| F7 APK signs | APK assinado por debug keystore local | TOKEN_VAZIO | `Apkc/proofs/out/apksigner-verify.txt` |
| F8 APK installs | `adb install -r` com device real | TOKEN_VAZIO | `Apkc/proofs/out/adb-install.txt` |
| F9 NativeActivity runs | logcat sem crash relevante | TOKEN_VAZIO | `Apkc/proofs/out/logcat-nativeactivity.txt` |

## Regras

- Todo `TOKEN_VAZIO` deve incluir a ferramenta ou pré-condição ausente.
- Todo `PASS` deve apontar para artefato verificável.
- Não declarar APK instalado, assinado ou executando sem os arquivos de evidência correspondentes.
