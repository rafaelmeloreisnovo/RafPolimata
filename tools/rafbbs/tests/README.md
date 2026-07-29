# RafBBS Tests

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `quality-assurance`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · tools`](../../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)  
**Documento pai:** [`tools/rafbbs/README.md`](../README.md)

## Propósito

Testes operacionais do núcleo freestanding e do adaptador POSIX do RafBBS. Todos os testes devem compilar com `-ffreestanding -fno-builtin` onde indicado e não podem depender de heap nem de libc.

## Arquivos de teste

| Arquivo | Cobertura | Estado |
|---|---|---|
| `rafbbs_failsafe_test.c` | watchdog, rollback e SHA256 conhecido | `ACTIVE` |
| `rafbbs_freestanding_core_test.c` | compilação do core sem host (`-ffreestanding -fno-builtin`) | `ACTIVE` |
| `rafbbs_baremetal_test.c` | saída byte-a-byte, manifesto binário, failover de hash, flags de arquitetura | `ACTIVE` |
| `rafbbs_watchdog_negative_test.c` | garante que watchdog expirado é detectado | `ACTIVE` |
| `rafbbs_baremetal_overflow_test.c` | saturação do buffer fixo e contador `dropped` | `ACTIVE` |
| `rafbbs_manifest_bin_test.c` | escrita e releitura de fixture de manifesto binário compacto | `ACTIVE` |

## Como executar

```sh
sh tools/rafbbs/rafbbs_test.sh
```

O script cobre build, help, listagem, file picker, TUI, watchdog, rollback, SHA256 conhecido e compilação do núcleo freestanding.

## Invariantes

- Nenhum teste usa `malloc`, `calloc` ou `free`
- `rafbbs_freestanding_core_test.c` compila sem libc
- Ausência de ferramenta → `SKIP` ou `TOKEN_VAZIO`, nunca `PASS` falso
- Testes em host x86 marcam rotinas ARM como `SKIP`, não como `FAIL`
