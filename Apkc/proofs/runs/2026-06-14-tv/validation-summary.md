# ApkC validation summary

- Data UTC: 2026-06-14T10:15:02Z
- Commit: TOKEN_VAZIO

| Gate | Status | Evidencia |
|---|---|---|
| F0 | PASS | Apkc/hello.s.txt presente |
| F1 | PASS | apkc compilado como objeto ARM64; linker cross ausente para executavel |
| F2 | TOKEN_VAZIO | geracao de hello.apk nao executada |
| F3 | TOKEN_VAZIO | hello.apk ausente |
| F4 | TOKEN_VAZIO | hello.apk ausente |
| F5 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |

Conclusao: verificacao basica arquivada; geracao de APK exige binario executavel do ApkC.
