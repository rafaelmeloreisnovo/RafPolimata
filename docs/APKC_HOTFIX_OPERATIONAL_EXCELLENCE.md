# APKc / RAF Compiler — HOTFIX de excelência operacional

**Estado desta revisão:** `IMPLEMENTED_HOTFIX / BLOCKING_TESTS_REQUIRED / claim_allowed=false`  
**Escopo:** compilador raiz `u32`, assimilação C/C++, lowering `RAF_KERNEL`, emissão nativa, recibos, rollback e auditoria ELF.  
**Fronteira:** não promove assinatura, instalação ou execução de APK em dispositivo.

## 1. Defeitos que exigiram o HOTFIX

| Severidade | Defeito localizado | Risco |
|---|---|---|
| P0 | offset FNV-1a incorreto no validador `.ops` | assinatura operacional impossível de recomputar corretamente |
| P0 | produtor e validador assinavam campos diferentes | cadeia de custódia inconsistente |
| P0 | falha podia preservar `.bin/.s/.hex` anteriores | artefato stale parecer resultado novo |
| P0 | `.so` podia ser promovido antes da criação do recibo | binário órfão sem fechamento de custódia |
| P1 | `memmove` comparava ponteiros não relacionados | comportamento indefinido em C |
| P1 | headers sem emulação eram removidos silenciosamente | perda semântica sem diagnóstico |
| P1 | manifests intermediários usavam `claim_allowed=true` | claim antes do ELF final |
| P1 | C++ podia aplicar name mangling nos exports | loader Android não localizar entrypoints |
| P1 | EXEC e Android `.so` usavam a mesma política ELF | contradição sobre `PT_DYNAMIC` |
| P1 | parser podia selecionar a primeira ocorrência | lowering ambíguo |
| P1 | `strcpy` e overflow de `atoi` não tinham política segura | overflow de buffer ou aritmética indefinida |

## 2. Correções

### 2.1 `.ops` schema 4

```text
arquitetura + marca + cores + linguagem + otimização + features + flags
+ tamanho/hash da fonte + métricas Ω
+ IR/ASM/BIN + ir_value + emitter_schema
+ native_requested/native_written
+ rollback_code + transaction_state
→ FNV-1a 64 canônico
```

```text
COMMITTED   → rollback_code=0 e artefatos coerentes
ROLLED_BACK → rollback_code!=0 e nenhum executável antigo preservado
```

Qualquer alteração em campo assinado invalida `ops_signature`.

### 2.2 Transação da saída raiz

```text
.tmp → flush/close → rename → COMMITTED
erro → apagar temporários + apagar .s/.hex/.bin antigos → escrever .ops ROLLED_BACK
```

`FILE_EXISTS ≠ PASS` virou comportamento, não apenas documentação.

### 2.3 Transação do `.so + receipt`

A saída selada e seu recibo formam uma unidade:

```text
ELF temporário
→ auditoria
→ receipt temporário
→ promover os dois
```

Falha antes do fechamento remove a saída nova. Não existe estado válido `OUTPUT_WITHOUT_RECEIPT`.

### 2.4 Emulação C/C++

Superfície permitida:

```text
memcpy memmove memset memcmp memchr
strlen strnlen strcmp strncmp strncpy strchr strrchr
atoi strtoul raf_write putchar puts
```

Políticas:

- `memmove` usa ordenação por `uintptr_t` nos targets de endereço plano;
- `atoi` satura em `INT_MIN/INT_MAX`;
- `strtoul` valida base, prefixo, `endptr`, sinal e overflow;
- `strncpy` exige capacidade explícita;
- `strcpy`, `strcat` e `gets` são proibidos;
- `RAF_EXPORT` usa `extern "C"` em C++;
- larguras inteiras têm assertions;
- heap permanece proibido.

### 2.5 Rewriter fail-closed

Headers assimiláveis:

```text
stddef.h stdint.h stdbool.h stdio.h stdlib.h string.h
```

A presença do header não libera toda a API. Header não emulado, include local sem resolução ou função fora da superfície falha antes do objeto.

```json
{
  "stage": "SOURCE_REWRITE_ONLY",
  "claim_allowed": false,
  "promotion_gate": "STRICT_ELF_AUDIT_AND_REPRODUCIBILITY"
}
```

### 2.6 `RAF_KERNEL`

- exatamente uma anotação em linha independente;
- até quatro argumentos `uint32_t`;
- operações aritméticas/bitwise delimitadas;
- divisão e módulo apenas por constante não nula;
- shift constante `0..31`;
- zero chamadas, estado ou alocação;
- resultado `uint32_modulo`;
- manifesto intermediário `claim_allowed=false`.

### 2.7 Perfis ELF

```text
exec       → ELF EXEC, sem PT_DYNAMIC
android-so → ELF DYN, PT_DYNAMIC + DT_SONAME permitidos
```

Ambos bloqueiam:

- `PT_INTERP`;
- `DT_NEEDED`;
- símbolos indefinidos;
- build-id;
- LOAD RWX;
- pilha executável;
- `RPATH/RUNPATH/TEXTREL`;
- máquina diferente do target.

O perfil Android admite somente relocação relativa autorizada.

### 2.8 Recibo SHA-256

Cada saída aprovada produz:

```text
libmain.so
libmain.so.receipt.json
```

O recibo registra hashes, arquitetura, target, compilador, dependências de build, zero dependência externa de runtime, gates e claims não promovidos.

## 3. Matriz real

| Rota | Entrada | Resultado | Limite |
|---|---|---|---|
| `ROOT_U32_IR` | uma expressão constante | `.s/.hex/.bin/.ops` | não é parser geral |
| `STRICT_C_REWRITE` | C/C++ no subset | `.so + receipt` | uma translation unit |
| `HOSTED_RAF_KERNEL` | anotação em 14 perfis | C estrito → `.so + receipt` | kernel puro |
| `DIRECT_ASSEMBLY` | ASM do target | `.so + receipt` | depende da ISA |

Inventário:

```text
ci/contracts/apkc_compiler_station_v2.json
scripts/validate_compiler_station_contract.py
```

## 4. Gates adversariais

```text
✓ memmove com sobreposição nos dois sentidos
✓ atoi com saturação positiva/negativa
✓ strtoul com zero, prefixo, sinal e base inválida
✓ strcpy proibido; cópia exige capacidade
✓ C ARM64 reproduzível byte a byte
✓ C ARM32
✓ C++ ARM64 sem name mangling
✓ Python RAF_KERNEL → ARM64
✓ header sem emulação falha antes do objeto
✓ múltiplos return falham
✓ operador ++ malformado falha
✓ comentário não esconde token posterior
✓ rollback elimina artefatos antigos
✓ alteração de ir_value invalida `.ops`
✓ `.so` e recibo fecham juntos
```

## 5. Comandos

```bash
make compiler-contract
make compiler-selftest
make language-contract
make hotfix-audit

make compile \
  RAF_LANG=c \
  RAF_ARCH=arm32 \
  SRC=tests/fixtures/strict_kernel.c \
  OUT=build/strict/libmain.so
```

## 6. Verdade operacional

```text
IMPLEMENTADO:
  lowering u32 dependente da fonte
  x86-64 / ARM64 / ARM32 Thumb-2 / RV64
  rewrite C/C++ fail-closed
  14 rotas RAF_KERNEL
  emulação sem heap e sem strcpy
  .ops schema 4 transacional
  .so Android sem DT_NEEDED
  recibos SHA-256 transacionais
  inventário e testes adversariais

AINDA EXIGE EVIDÊNCIA:
  workflows verdes deste commit/PR
  assinatura e instalação do APK
  launch e logcat no Android
  NativeActivity observada
  GPU/DSP/NPU físico
  x86-64/RV64 quando não executados no target
  semântica geral fora dos contratos
```

## 7. Promoção

```text
antes do CI: IMPLEMENTED_HOTFIX / claim_allowed=false
após todos os gates: HOTFIX_VERIFIED_BY_CI
```

A promoção vale somente para a estação. `HOTFIX_VERIFIED_BY_CI ≠ APK_RUNTIME_PROVEN`.

## R3

```text
F_ok   = transação + FNV reparado + superfície segura + ELF perfilado
F_gap  = CI do branch, dispositivo e targets condicionais
F_next = empacotar a saída selada e capturar assinatura→instalação→launch
```
