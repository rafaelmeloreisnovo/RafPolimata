# Protocolo de validação ApkC

| Fase | Gate | Status inicial | Evidência |
|---|---|---|---|
| F0 | source exists | NOT_RUN | `Apkc/hello.s.txt` |
| F1 | compile apkc | NOT_RUN | `Apkc/proofs/out/apkc-compile.txt` |
| F2 | generate hello.apk | NOT_RUN | `Apkc/proofs/out/hello.apk` |
| F3 | unzip parses | NOT_RUN | `Apkc/proofs/out/unzip.txt` |
| F4 | AXML parses | TOKEN_VAZIO | `Apkc/proofs/out/aapt-xmltree.txt` |
| F5 | DEX SHA-1 matches | NOT_RUN | `Apkc/proofs/out/dex-sha1.txt` |
| F6 | ELF readelf parses | NOT_RUN | `Apkc/proofs/out/readelf-arm64.txt`, `readelf-arm32.txt` |
| F7 | APK signs | TOKEN_VAZIO | `Apkc/proofs/out/apksigner-verify.txt` |
| F8 | APK installs | TOKEN_VAZIO | `Apkc/proofs/out/adb-install.txt` |
| F9 | NativeActivity runs | TOKEN_VAZIO | `Apkc/proofs/out/logcat-nativeactivity.txt` |
| F10 | proof archived | NOT_RUN | commit + artifacts CI |

## Política de verdade

- `PASS` exige comando executado e artefato verificável.
- `FAIL` exige saída capturada para diagnóstico.
- `TOKEN_VAZIO` registra ausência de ferramenta, device ou pré-condição.
- `SKIP` só é usado quando a etapa não se aplica ao artefato encontrado.
- `NOT_RUN` indica que a etapa ainda não foi executada.
