# ApkC language coverage

- Data UTC: 2026-06-17T22:10:49Z
- Commit: 650e199

| Lang | Pipeline | Status | Evidência |
|------|----------|--------|-----------|
| asm | use_asm | PASS | APK gerado: 3626 bytes |
| py | use_script | PASS | APK gerado: 3706 bytes |
| sh | use_script | PASS | APK gerado: 3690 bytes |
| pl | use_script | PASS | APK gerado: 3706 bytes |
| js | use_script | PASS | APK gerado: 3706 bytes |
| php | use_script | PASS | APK gerado: 3706 bytes |
| c | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |
| cpp | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |
| rs | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |
| kt | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |
| java | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |
| jsx | use_fork | TOKEN_VAZIO | fork_exec_wait é ARM64-only; testado em Termux/ARM64 |

Conclusão: 6/6 testes PASS, 0 FAIL (modo=qemu); use_fork TOKEN_VAZIO (ARM64-only).
