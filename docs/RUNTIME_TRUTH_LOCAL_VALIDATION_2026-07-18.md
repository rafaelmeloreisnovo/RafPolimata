# Runtime Truth — validação local — 2026-07-18

## Escopo

Este corte materializa correções da auditoria RafGitTools ↔ RafPolimata sem usar
GitHub Actions. A conta está momentaneamente sem crédito de Actions; por isso:

```text
workflow não executado
≠ PASS
≠ FAIL de código
```

## Mudanças implementadas

### Compilador raiz

- `--native` é reconhecido e encaminhado ao frontend;
- flags podem aparecer imediatamente após o source, sem serem confundidas com `out_base`;
- `.bin` passa a ser escrito quando solicitado;
- `.ops` registra `native_requested` e `native_written`;
- fontes acima de `RAF_SOURCE_CAP` são rejeitadas, não truncadas;
- FNV-1a 64 usa offset basis canônico;
- extensão desconhecida retorna `RAF_LANG_UNKNOWN` e falha com rollback `-6`;
- quantidade de CPUs online deixa de ser fixada em 1 no Linux;
- device node GPU/DSP/NPU gera somente flag de presença, não capacidade validada.

### Conversation Indexer

Foi implementado o primeiro fragmento congelado de `segment.v1`:

- header serializado explicitamente em 64 bytes;
- little-endian independente de padding do compilador;
- magic `RAFSEG1\0`;
- versão major `1`;
- CRC32C do header com campo de CRC zerado durante o cálculo;
- reserved bytes obrigatoriamente zero;
- offsets mínimos verificados;
- corrupção rejeitada.

## Prova realmente executada nesta atividade

O codec isolado de `segment.v1` foi compilado fora do GitHub Actions com:

```text
cc -std=c11 -Wall -Wextra -Werror -pedantic
```

Resultados observados:

```text
CRC32C("123456789") = 0xe3069283  PASS
encode → decode round-trip           PASS
alteração de byte → CRC rejeitado    PASS
```

Essa prova cobre o codec do header. Ela não prova o build integral do repositório,
Android, ARM32, ARM64, ApkC runtime ou extração completa de conversas.

## Gate local criado

```sh
bash scripts/validate_runtime_truth_local.sh
```

O script foi adicionado, mas o resultado integral deve ser executado em checkout
local/Termux e anexado posteriormente. Ele compila o núcleo, executa `segment.v1`,
verifica `.bin/.ops`, rejeita extensões desconhecidas e fontes oversized, e valida
`ECOSYSTEM_RUNTIME_STATE.json`.

## Estados preservados

| Corpo | Estado após este corte |
|---|---|
| header `segment.v1` | `VERIFIED` no teste isolado descrito acima |
| compilador raiz modificado | `DECLARED_BY_AUTHOR`; gate integral local pendente |
| aceleradores | node presence `PARTIAL`; runtime compute `TOKEN_VAZIO` |
| ApkC NativeActivity | `DEVICE_REQUIRED / TOKEN_VAZIO` |
| GitHub Actions | `OUT_OF_SCOPE_NO_CREDIT` |
| ponte RafGitTools → Termux | `TOKEN_VAZIO` |

## Próxima prova permitida

A próxima promoção só pode ocorrer com:

1. comando local exato;
2. versão de compilador;
3. stdout e stderr;
4. commit da branch;
5. hashes dos artefatos;
6. para Android: ABI, device-info, install, launch e logcat.

Nenhum documento ou PR fechado substitui esses elementos.
