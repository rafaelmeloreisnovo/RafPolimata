# RafBBS tests

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `quality-assurance`  
**Repositório:** [`rafaelmeloreisnovo/RafPolimata`](https://github.com/rafaelmeloreisnovo/RafPolimata) — `tools/rafbbs/tests/README.md`

- `rafbbs_failsafe_test.c`: watchdog, rollback e SHA256 conhecido.
- `rafbbs_freestanding_core_test.c`: compila o core sem host com `-ffreestanding -fno-builtin`.
- `rafbbs_zero_dependency_test.c`: known vectors CRC32/SHA-256 + bare-metal/watchdog sob `-nostdinc`, sem símbolos externos no objeto.
- `rafbbs_time_core_test.c`: falsifica aritmética monotônica autoral, ordem reversa e estado inválido; compila sob `-nostdinc` e também executa como smoke semântico no host CI.
- `rafbbs_filepicker_core_test.c`: falsifica catálogo estático, seleção válida e preservação do estado sob escolhas inválidas; zero hosted headers.
- `rafbbs_theme_core_test.c`: falsifica mapeamento status→ANSI e default vazio; zero hosted headers e zero símbolos externos.
- `rafbbs_baremetal_test.c`: saída byte-a-byte, manifesto binário, failover de hash e flags de arquitetura.
- `rafbbs_watchdog_negative_test.c`: garante que watchdog expirado é detectado.
- `rafbbs_baremetal_overflow_test.c`: valida saturação do buffer fixo e contador `dropped`.
- `rafbbs_manifest_bin_core_test.c`: valida bytes little-endian canônicos, roundtrip e rejeição de tamanho truncado sem headers hospedados.
- `rafbbs_manifest_bin_test.c`: valida persistência host sobre o codec autoral, sem desserializar `struct` nativa diretamente.

- rafbbs_authorial_probe.c: caller-owned probe that exercises every declared pure module without hosted headers or external runtime symbols.
