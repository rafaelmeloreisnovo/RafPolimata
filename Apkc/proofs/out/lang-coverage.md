# ApkC language coverage

- Data UTC: 2026-07-12T07:08:50Z
- Commit: 6f9e104

| Lang | Pipeline | Status | Evidência |
|------|----------|--------|-----------|
TOKEN_VAZIO: não há binário apkc executável neste host; instale toolchain ARM/Linux.
| asm | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| py | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| sh | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| pl | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| js | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| php | use_asm/use_script | TOKEN_VAZIO | apkc não executável neste host |
| c | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |
| cpp | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |
| rs | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |
| kt | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |
| java | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |
| jsx | use_fork | TOKEN_VAZIO | fork_exec_wait ARM64-only; apkc não executável |

Conclusão: sem binário apkc executável; todos os gates são TOKEN_VAZIO.
