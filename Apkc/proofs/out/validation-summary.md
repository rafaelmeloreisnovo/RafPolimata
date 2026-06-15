# ApkC validation summary

- Data da prova original UTC: 2026-06-14T10:56:26Z
- Data de consolidacao: 2026-06-15
- Repositorio: rafaelmeloreisnovo/RafPolimata
- Politica: cadeia de custodia por arquivo, hash e transcript.

| Gate | Status | Evidencia/observacao |
|---|---|---|
| F0 | PASS | Apkc/hello.s.txt esta versionado no repositorio. |
| F1 | TOKEN_VAZIO | Falta transcript source-to-binary do executavel apkc; binario local foi enviado, mas nao prova build do commit atual. |
| F2 | PASS | hello.apk gerado; SHA-256 a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024; geracao teve 39 avisos unknown ARM32 mnemonic. |
| F3 | PASS | unzip lista 3 entradas no unsigned e 6 no signed. |
| F4 | PASS | aapt parseia manifest com package com.rafael.teste, NativeActivity, lib hello. |
| F5 | PASS | SHA-1 interno do classes.dex confere: 9ea7c00884bffbdbbab055ddc3ad6565050fc4e4. |
| F6-arm32 | PASS | libhello.so e ELF32 ARM EABI5 shared object, soft-float ABI. |
| F6-arm64 | SKIP | APK enviado nao contem lib/arm64-v8a/*.so. |
| F6-negative | FAIL esperado | readelf out/hello.apk falha porque APK e container ZIP, nao ELF. |
| F7 | PASS | apksigner reporta v1/v2/v3 true; signer CN=ApkC Debug, O=Rafael, C=BR; v4 false; SourceStamp false. |
| F8 | PASS limitado | prova enviada contem package:com.rafael.teste; stdout completo de adb install -r permanece ausente. |
| F9 | TOKEN_VAZIO | Sem logcat/launch NativeActivity. |

Conclusao: a prova avancou de NOT_RUN/TOKEN_VAZIO para APK gerado, parseado, assinado e visivel como pacote instalado. A lacuna principal agora e runtime NativeActivity + reprodutibilidade source-to-binary.
