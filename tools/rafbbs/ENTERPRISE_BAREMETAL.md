# RafBBS Enterprise/Bare-Metal Profile

Este perfil descreve os 10 passos operacionais pedidos para reduzir fricção, evitar heap/GC e preservar prova honesta.

1. **Separação host/baremetal**: `rafbbs_host.h` concentra execução POSIX; `rafbbs_baremetal.h` concentra saída, manifesto binário, failover de hash e flags de arquitetura.
2. **Saída byte-a-byte**: `RafBaremetalOut` usa buffer fixo e contador de bytes descartados; não há `malloc`.
3. **SHA256/CRC32 failover**: SHA256 assina evidência quando existe; CRC32 é fallback; ausência total vira `TOKEN_VAZIO`.
4. **File picker sem runtime scan**: entradas conhecidas ficam em tabela estática; varredura de diretório pode ser gerada em build-time.
5. **Full-chain proof honesto**: `proof_chain` chama a captura real no host e permanece `AUDIT/PASS_LIMITED` sem dispositivo/logcat.
6. **Watchdog preventivo**: cada comando recebe checkpoint e tick antes da execução; expiração vira `FAIL`.
7. **Rollback paliativo**: checkpoints guardam passo/status/hash em ring fixo para restauração ou auditoria.
8. **Manifesto compacto**: `RafBinManifest` guarda status, arquitetura, hashes e lacunas em forma fixa para bare-metal.
9. **Flags por arquitetura**: `generic`, `arm32`, `arm32_neon`, `arm64` e `x86_64` têm perfil de cache/SIMD/watchdog.
10. **Testes failsafe/failover**: o harness compila host e freestanding, testa watchdog negativo, rollback, hash e ausência de heap explícito.

## Flags recomendadas

- Host auditável: `-std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L`
- Core freestanding: `-std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin`
- ARM64 futuro: adicionar flags específicas do toolchain sem remover `-ffreestanding` no core.

## Regra de verdade

`TOKEN_VAZIO` não é falha moral e não é sucesso técnico. É ausência de evidência. O RafBBS só promove para `PASS` quando a prova existe.

## Próximo ciclo implementado

1. O manifesto binário agora tem gravador host (`rafbbs_manifest_bin.h`).
2. A saída bare-metal tem porta de flush por callback byte-a-byte.
3. `raf_arch_flags()` usa tabela constante indexada, reduzindo condicionais.
4. O build tem alvo `commandless` para compilar o operador com `RAFBBS_FREESTANDING_MODE`.
5. O harness testa fixture binária do manifesto.
6. O harness testa overflow de buffer bare-metal e contador `dropped`.
7. O manifesto TXT registra `hash_state` e caminho do manifesto binário.
8. O log TXT registra `hash_state` e manifesto binário associado.
9. O failover de hash permanece `SHA256_OK`, `CRC32_OK` ou `TOKEN_VAZIO`.
10. Os artefatos gerados continuam ignorados/revertidos para não poluir prova fonte.
