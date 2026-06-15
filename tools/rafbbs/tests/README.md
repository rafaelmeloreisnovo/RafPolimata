# RafBBS tests

- `rafbbs_failsafe_test.c`: watchdog, rollback e SHA256 conhecido.
- `rafbbs_freestanding_core_test.c`: compila o core sem host com `-ffreestanding -fno-builtin`.
- `rafbbs_baremetal_test.c`: saída byte-a-byte, manifesto binário, failover de hash e flags de arquitetura.
- `rafbbs_watchdog_negative_test.c`: garante que watchdog expirado é detectado.
- `rafbbs_baremetal_overflow_test.c`: valida saturação do buffer fixo e contador `dropped`.
- `rafbbs_manifest_bin_test.c`: grava e relê fixture de manifesto binário compacto.
