# Checklist de validacao — hello.apk

Escopo: validar hello.apk e hello-signed.apk no fluxo ApkC, sem afirmar runtime sem logcat.

| Gate | Checklist | Status | Evidencia |
|---|---:|---|---|
| F0 source exists | Apkc/hello.s.txt existe | PASS | arquivo versionado |
| F1 compile apkc | apkc.c compila ate o executavel usado | TOKEN_VAZIO | Apkc/proofs/out/apkc-compile.txt |
| F2 generate hello.apk | hello.apk gerado | PASS | Apkc/proofs/out/apkc-generate.txt; SHA-256 a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024 |
| F3 unzip parses | unzip le APK unsigned/signed | PASS | Apkc/proofs/out/unzip.txt |
| F4 AXML parses | aapt le manifest | PASS | Apkc/proofs/out/aapt-xmltree.txt |
| F5 DEX SHA-1 matches | checksum interno DEX confere | PASS | Apkc/proofs/out/dex-sha1.txt |
| F6 ELF readelf parses | .so ARM32 lido por readelf | PASS | Apkc/proofs/out/readelf-arm32.txt |
| F6 arm64 present | .so ARM64 presente | SKIP | Apkc/proofs/out/readelf-arm64.txt |
| F7 APK signs | APK assinado por debug keystore local | PASS | Apkc/proofs/out/apksigner-verify.txt; SHA-256 063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28 |
| F8 package visible | pacote aparece instalado | PASS limitado | Apkc/proofs/out/adb-install.txt |
| F9 NativeActivity runs | logcat sem falha relevante | TOKEN_VAZIO | Apkc/proofs/out/logcat-nativeactivity.txt |

## Regras

- Todo TOKEN_VAZIO deve incluir ferramenta, comando ou pre-condicao ausente.
- Todo PASS deve apontar para artefato verificavel.
- PASS limitado nao vira prova plena de runtime.
- Nao declarar execucao NativeActivity sem logcat/launch capturado.
