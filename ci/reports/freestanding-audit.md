# Freestanding audit — ApkC

- Data UTC: 2026-08-14T23:02:21Z
- Commit: 542047d

| Gate | Status | Observação |
|---|---|---|
| heap_calls | PASS | padrão ausente |
| libc_includes | FAIL | padrão proibido encontrado |
- /home/user/RafPolimata/Apkc/hardening_integration_test.c:7:#include <stdint.h>
- /home/user/RafPolimata/Apkc/hardening_integration_test.c:9:#include <stdio.h>
- /home/user/RafPolimata/Apkc/hardening_integration_test.c:10:#include <string.h>

Resultado: FAIL — violação freestanding encontrada.
