# ApkC validation summary

- Data UTC: 2026-06-14T08:56:34Z
- Commit: 8aef89a

| Gate | Status | Evidência/observação |
|---|---|---|
| F0 | PASS | Apkc/hello.s.txt presente |
| F1 | PASS | apkc compilado como objeto ARM64; linker cross ausente para executável; log: Apkc/proofs/out/apkc-compile.txt |
| F2 | TOKEN_VAZIO | geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável |
| F3 | TOKEN_VAZIO | hello.apk ausente |
| F4 | TOKEN_VAZIO | hello.apk ausente |
| F5 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |

Conclusão: compilação/verificação básica arquivada; geração de APK exige binário executável do ApkC.
