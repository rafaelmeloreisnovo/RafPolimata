# Matriz de validação ApkC

Estado atualizado por cadeia de custodia documental de 2026-06-14T10:56:26Z. Regra: nao preencher lacunas por suposicao.

| Ferramenta | Fase | Fonte | Estado aferido | Saida |
|---|---|---|---|---|
| git | source exists | Apkc/hello.s.txt | PASS | arquivo versionado |
| compilador C | build do apkc | transcript de build | TOKEN_VAZIO | Apkc/proofs/out/apkc-compile.txt |
| apkc | generate APK | uploaded generate.log | PASS com GAP | Apkc/proofs/out/apkc-generate.txt |
| unzip | ZIP parse | unzip -l hello.apk e hello-signed.apk | PASS | Apkc/proofs/out/unzip.txt |
| aapt | AXML parse | aapt dump xmltree | PASS | Apkc/proofs/out/aapt-xmltree.txt |
| Python | DEX SHA-1 | classes.dex | PASS | Apkc/proofs/out/dex-sha1.txt |
| readelf | ELF header arm32 | readelf em libhello.so | PASS | Apkc/proofs/out/readelf-arm32.txt |
| readelf | ELF header arm64 | procurar lib/arm64-v8a/*.so | SKIP | Apkc/proofs/out/readelf-arm64.txt |
| readelf | controle negativo | readelf out/hello.apk | FAIL esperado | Apkc/proofs/out/readelf-apk-invalid.txt |
| apksigner | APK signing verify | APK_PROOF_INSTALL_OPEN.txt | PASS | Apkc/proofs/out/apksigner-verify.txt |
| adb/package manager | package visibility | package com.rafael.teste visivel | PASS limitado | Apkc/proofs/out/adb-install.txt |
| logcat | runtime evidence | NativeActivity/logcat | TOKEN_VAZIO | Apkc/proofs/out/logcat-nativeactivity.txt |

## Observacoes de verdade

- PASS limitado significa evidencia positiva parcial, sem virar prova plena de runtime.
- O transcript de geracao registrou 39 linhas `apkc: unknown ARM32 mnemonic`; isso abre gap de cobertura do assembler ARM32.
- readelf aplicado diretamente ao APK e controle negativo: APK e ZIP/container, nao ELF.
