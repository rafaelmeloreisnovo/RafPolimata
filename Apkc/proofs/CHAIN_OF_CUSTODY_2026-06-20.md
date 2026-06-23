
## 2026-06-20T04:13:32Z

**pipeline**: proof_chain  
**script**: scripts/apkc_validate.sh  
**result**: PASS  

```
| F0 | PASS | Apkc/hello.s.txt presente |
| F1 | PASS | apkc compilado como objeto ARM64; linker executável ausente; log: Apkc/proofs/out/apkc-compile.txt |
| F2 | TOKEN_VAZIO | geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável |
| F3 | TOKEN_VAZIO | hello.apk ausente |
| F4 | TOKEN_VAZIO | hello.apk ausente |
| F5 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |

Conclusão: compilação/verificação básica arquivada; geração de APK exige binário executável do ApkC.
```

## 2026-06-20T04:13:44Z

**pipeline**: proof_chain  
**script**: scripts/apkc_validate.sh  
**result**: PASS  

```
| F0 | PASS | Apkc/hello.s.txt presente |
| F1 | PASS | apkc compilado como objeto ARM64; linker executável ausente; log: Apkc/proofs/out/apkc-compile.txt |
| F2 | TOKEN_VAZIO | geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável |
| F3 | TOKEN_VAZIO | hello.apk ausente |
| F4 | TOKEN_VAZIO | hello.apk ausente |
| F5 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |
| F6 | TOKEN_VAZIO | hello.apk ausente |

Conclusão: compilação/verificação básica arquivada; geração de APK exige binário executável do ApkC.
```
