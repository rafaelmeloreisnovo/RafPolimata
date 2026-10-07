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
- `rafbbs_log_core_test.c`: falsifica linha exata, tempo inválido, minutos >99 e overflow; o divmod 64-bit autoral evita helper externo em alvos 32-bit.
- `rafbbs_format_core_test.c`: falsifica texto prefixado, i32 mínimo/máximo, hex32 fixo, tamanho de string e overflow; sem stdarg/vsnprintf/div helper externo.
- `rafbbs_git_core_test.c`: falsifica HEAD simbólico/detached, SHA-1 OID, loose ref, packed-ref match/mismatch, hexadecimal inválido e truncamento; zero execução de `git` e zero filesystem no core.
- `rafbbs_runlog_core_test.c`: falsifica bytes exatos do cabeçalho/artefatos/gaps, CRC32 hexadecimal fixo, TOKEN_VAZIO (CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY) para gaps ausentes e overflow de buffer; zero FILE/fprintf/libc no core.
- `rafbbs_pipeline_core_test.c`: falsifica igualdade exata/case-sensitive, flags, lookup por índice e not-found; nenhum callback/handler no core.
- `rafbbs_cli_core_test.c`: falsifica HELP/LIST/RUN/INVALID, argumento de RUN e comparação case-sensitive; zero terminal/libc no core.
- `rafbbs_tui_core_test.c`: falsifica QUIT/LIST/FILES, rotas 1/2/3 e default/tecla desconhecida preservando ids exatos; zero stdio/terminal no core.
- `rafbbs_context_core_test.c`: falsifica seed exato, composição de caminhos, watchdog/zero-state e truncamento sem memset/snprintf/libc.
- `rafbbs_command_core_test.c`: falsifica ausência de execução, rc=0, falha opcional e falha obrigatória sem shell/provider.
- `rafbbs_baremetal_test.c`: saída byte-a-byte, manifesto binário, failover de hash e flags de arquitetura.
- `rafbbs_watchdog_negative_test.c`: garante que watchdog expirado é detectado.
- `rafbbs_baremetal_overflow_test.c`: valida saturação do buffer fixo e contador `dropped`.
- `rafbbs_manifest_bin_core_test.c`: valida bytes little-endian canônicos, roundtrip e rejeição de tamanho truncado sem headers hospedados.
- `rafbbs_manifest_bin_test.c`: valida persistência host sobre o codec autoral, sem desserializar `struct` nativa diretamente.

- rafbbs_authorial_probe.c: caller-owned probe that exercises every declared pure module without hosted headers or external runtime symbols.
