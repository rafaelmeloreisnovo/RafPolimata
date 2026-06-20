# ApkC validation summary

- Data UTC: 2026-06-20T04:13:44Z
- Commit: 93c8230
- Exec root: /tmp/apkc_validate_12583

| Gate | Status | Evidência/observação |
|---|---|---|
| F0 | PASS | Apkc/hello.s.txt presente |
| F1 | PASS | apkc compilado como objeto ARM64; linker executável ausente; log: Apkc/proofs/out/apkc-compile.txt |
| F2 | TOKEN_VAZIO | geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável |
| F3 | TOKEN_VAZIO | hello.apk ausente |
| F4 | TOKEN_VAZIO | hello.apk ausente |
| F5 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |

Conclusão: compilação/verificação básica arquivada; geração de APK exige binário executável do ApkC.
