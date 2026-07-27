# APKc / RAF Compiler — HOTFIX de excelência operacional

**Estado desta revisão:** `IMPLEMENTED_HOTFIX / BLOCKING_TESTS_REQUIRED / claim_allowed=false`  
**Escopo:** compilador raiz `u32`, assimilação C/C++, lowering `RAF_KERNEL`, emissão nativa, recibos, rollback e auditoria ELF.  
**Fronteira:** não promove assinatura, instalação ou execução de APK em dispositivo.

## 1. Por que o HOTFIX foi necessário

A estação anterior eliminou o retorno fixo `42` e criou rotas executáveis, mas a auditoria posterior encontrou defeitos que impediam chamar a cadeia de **excelência operacional**:

| Severidade | Defeito localizado | Risco |
|---|---|---|
| P0 | offset FNV-1a incorreto no validador `.ops` | recibo legítimo não podia ser recomputado corretamente |
| P0 | produtor e validador assinavam conjuntos diferentes de campos | cadeia de custódia inconsistente |
| P0 | falha sobre o mesmo `out_base` podia deixar `.bin/.s/.hex` anteriores | artefato antigo poderia parecer resultado da tentativa nova |
| P1 | `memmove` ordenava ponteiros não relacionados diretamente | comportamento indefinido em C |
| P1 | headers sem emulação eram removidos silenciosamente | compilação poderia perder semântica sem diagnóstico claro |
| P1 | manifests intermediários usavam `claim_allowed=true` | promoção epistemológica antes do ELF final |
| P1 | C++ exportava símbolos sujeitos a name mangling | loader Android poderia não localizar os entrypoints |
| P1 | auditoria ELF tratava EXEC e Android `.so` como o mesmo perfil | política contraditória para `PT_DYNAMIC` |
| P1 | parser podia escolher a primeira entre múltiplas ocorrências | lowering ambíguo em vez de falha fechada |

## 2. Correções aplicadas

### 2.1 Recibo operacional `.ops` schema 4

O produtor e o validador usam agora o mesmo contrato assinado:

```text
arquitetura + marca + cores + linguagem + otimização + features + flags
+ tamanho/hash da fonte
+ métricas Ω
+ contagens IR/ASM/BIN
+ ir_value + emitter_schema
+ native_requested/native_written
+ rollback_code + transaction_state
→ FNV-1a 64 canônico
```

Estados permitidos:

```text
COMMITTED   → rollback_code=0 e artefatos coerentes
ROLLED_BACK → rollback_code!=0 e nenhum binário antigo preservado
```

Qualquer alteração em campo assinado invalida `ops_signature`.

### 2.2 Escrita transacional

A compilação escreve primeiro em arquivos temporários e somente depois promove por `rename`:

```text
.tmp → flush/close → rename atômico → COMMITTED
```

Em erro:

```text
remover temporários
remover .s/.hex/.bin anteriores do mesmo out_base
escrever apenas .ops com ROLLED_BACK
```

Assim, `FILE_EXISTS ≠ PASS` deixa de ser apenas regra documental e vira comportamento executável.

### 2.3 Emulação C/C++ endurecida

`Apkc/raf_libc_emu.h` contém uma superfície explícita, sem heap:

```text
memcpy memmove memset memcmp memchr
strlen strnlen strcmp strncmp strcpy strncpy strchr strrchr
atoi strtoul raf_write putchar puts
```

Correções relevantes:

- `memmove` usa ordem de endereços por `uintptr_t`, evitando comparação relacional indefinida entre ponteiros;
- `strtoul` valida base, prefixo, `endptr`, sinal e overflow;
- larguras inteiras recebem assertions de compilação;
- `RAF_EXPORT` usa `extern "C"` em C++;
- heap continua proibido por construção.

### 2.4 Rewriter fail-closed

Somente estes headers podem ser assimilados:

```text
stddef.h stdint.h stdbool.h stdio.h stdlib.h string.h
```

A presença do header não libera toda sua API. Chamadas fora da superfície emulada continuam bloqueadas. Headers como `ctype.h`, `errno.h`, `time.h`, `unistd.h` e includes locais não resolvidos são rejeitados — nunca removidos silenciosamente.

O manifesto de rewrite é apenas evidência intermediária:

```json
{
  "stage": "SOURCE_REWRITE_ONLY",
  "claim_allowed": false,
  "promotion_gate": "STRICT_ELF_AUDIT_AND_REPRODUCIBILITY"
}
```

### 2.5 Lowering `RAF_KERNEL` delimitado

O marcador precisa existir exatamente uma vez e em linha de anotação independente. Não é reconhecido dentro de string comum.

Semântica:

- até quatro argumentos `uint32_t`;
- soma, subtração, multiplicação e operações bitwise;
- divisão/módulo somente por constante não nula;
- shift somente por constante entre 0 e 31;
- zero chamadas, atributos, índices, estado ou alocação;
- resultado `uint32_modulo`.

O manifesto de lowering também permanece `claim_allowed=false` até o ELF ser selado.

### 2.6 Dois perfis ELF

```text
exec       → ELF EXEC, sem PT_DYNAMIC
android-so → ELF DYN, PT_DYNAMIC + DT_SONAME permitidos
```

Em ambos:

- sem `PT_INTERP`;
- sem `DT_NEEDED`;
- sem símbolo indefinido;
- sem build-id;
- sem segmento LOAD RWX;
- sem pilha executável;
- sem `RPATH`, `RUNPATH` ou `TEXTREL`;
- máquina precisa coincidir com o target.

O perfil Android aceita somente relocação relativa autorizada quando necessária ao loader.

### 2.7 Recibo SHA-256 do `.so`

Cada compilação estrita aprovada produz:

```text
libmain.so
libmain.so.receipt.json
```

O recibo registra:

- hashes SHA-256 da fonte e saída;
- target e arquitetura;
- versão do compilador;
- dependências do plano de build;
- zero dependência externa no runtime final;
- resultado de cada gate;
- claims permitidos e não reivindicados.

## 3. Matriz real de rotas

| Rota | Entrada | Resultado | Limite |
|---|---|---|---|
| `ROOT_U32_IR` | uma expressão constante | `.s/.hex/.bin/.ops` | não é parser geral da linguagem |
| `STRICT_C_REWRITE` | C/C++ no subset explícito | `.so` Android selado | tradução unitária sem includes externos |
| `HOSTED_RAF_KERNEL` | anotação em 14 perfis | C estrito → `.so` | kernel puro, não linguagem completa |
| `DIRECT_ASSEMBLY` | ASM do target | `.so` Android selado | validade depende da ISA/toolchain alvo |

O inventário canônico está em:

```text
ci/contracts/apkc_compiler_station_v2.json
```

E é validado por:

```text
scripts/validate_compiler_station_contract.py
```

## 4. Gates adversariais adicionados

```text
✓ memmove com sobreposição nos dois sentidos
✓ strtoul: zero, prefixo hexadecimal, sinal e base inválida
✓ C ARM64 repetido produz byte a byte o mesmo .so
✓ C ARM32
✓ C++ ARM64 com exports não mangled
✓ Python RAF_KERNEL → ARM64
✓ header sem emulação falha antes do objeto
✓ múltiplos return falham
✓ operador ++ malformado falha
✓ comentário de bloco não esconde token posterior
✓ falha sobre saída antiga elimina .s/.hex/.bin
✓ alteração de ir_value invalida assinatura .ops
✓ recibo JSON contém somente claims aprovadas pelos gates
```

## 5. Comandos canônicos

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

## 6. Verdade operacional após o HOTFIX

```text
IMPLEMENTADO:
  lowering u32 dependente da fonte
  emissão x86-64 / ARM64 / ARM32 Thumb-2 / RV64
  rewrite C/C++ delimitado
  14 rotas RAF_KERNEL
  emulação C sem heap
  .ops schema 4 transacional
  .so Android sem DT_NEEDED
  recibos SHA-256
  testes adversariais e inventário canônico

AINDA EXIGE EVIDÊNCIA EXTERNA:
  assinatura do APK
  instalação no Android
  lançamento e logcat
  ANativeActivity_onCreate observado no dispositivo
  loaders GPU/DSP/NPU
  execução x86-64/RV64 quando não houver target disponível
  semântica geral das linguagens fora dos contratos delimitados
```

## 7. Promoção de estado

Antes do CI:

```text
IMPLEMENTED_HOTFIX / claim_allowed=false
```

Somente após todos os jobs bloqueantes:

```text
HOTFIX_VERIFIED_BY_CI / claim_allowed=true
```

A promoção vale apenas para a estação descrita aqui. Não converte automaticamente o projeto em APK runtime-proven.

## R3

```text
F_ok   = cadeia transacional + assinatura reparada + ELF perfilado + testes adversariais
F_gap  = prova de dispositivo e rotas condicionais sem target executado
F_next = usar a saída selada no empacotador APK e capturar uma única prova de assinatura→instalação→launch
```
