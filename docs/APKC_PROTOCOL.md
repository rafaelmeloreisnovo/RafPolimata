# Protocolo de validacao ApkC

| Fase | Gate | Estado atual | Evidencia |
|---|---|---|---|
| F0 | source exists | PASS | Apkc/hello.s.txt |
| F1 | compile apkc | TOKEN_VAZIO | Apkc/proofs/out/apkc-compile.txt |
| F2 | generate hello.apk | PASS | Apkc/proofs/out/apkc-generate.txt, SHA-256 do APK |
| F3 | unzip parses | PASS | Apkc/proofs/out/unzip.txt |
| F4 | AXML parses | PASS | Apkc/proofs/out/aapt-xmltree.txt |
| F5 | DEX SHA-1 matches | PASS | Apkc/proofs/out/dex-sha1.txt |
| F6 | ELF readelf parses | PASS/SKIP | readelf-arm32.txt PASS; readelf-arm64.txt SKIP |
| F7 | APK signs | PASS | Apkc/proofs/out/apksigner-verify.txt |
| F8 | APK installs/package visible | PASS limitado | Apkc/proofs/out/adb-install.txt |
| F9 | NativeActivity runs | TOKEN_VAZIO | Apkc/proofs/out/logcat-nativeactivity.txt |
| F10 | proof archived | PASS | Apkc/proofs/CHAIN_OF_CUSTODY_2026-06-14.md + Apkc/proofs/out/* |

## Politica de verdade

- PASS exige comando executado e artefato verificavel.
- FAIL exige saida capturada para diagnostico.
- TOKEN_VAZIO registra ausencia de ferramenta, device, comando, stdout bruto ou pre-condicao.
- SKIP so e usado quando a etapa nao se aplica ao artefato encontrado.
- NOT_RUN indica que a etapa ainda nao foi executada.
- PASS limitado registra evidencia positiva parcial sem transformar lacuna em conclusao plena.

## Estado critico preservado

- Ha prova de APK assinado v1/v2/v3 e package visibility.
- Ainda nao ha prova de runtime NativeActivity sem falha.
- Ainda nao ha transcript completo source-to-binary para o executavel apkc usado.
