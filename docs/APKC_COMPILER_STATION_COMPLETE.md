# APKc / RAF Compiler — Estação completa e endurecida

**Estado no código:** `IMPLEMENTED_HOTFIX`  
**Estado da evidência neste branch:** `CI_PENDING / claim_allowed=false`  
**Escopo:** núcleo `u32`, C/C++ estrito, lowering `RAF_KERNEL`, emissão nativa, transação e selagem de `.so` Android.  
**HOTFIX detalhado:** [`APKC_HOTFIX_OPERATIONAL_EXCELLENCE.md`](APKC_HOTFIX_OPERATIONAL_EXCELLENCE.md).

## Pipeline canônico

```text
fonte
→ detectar rota
→ validar contrato delimitado
→ rewrite/lowering dependente da fonte
→ objeto freestanding
→ link estrito
→ auditoria ELF por perfil
→ promoção atômica do artefato e recibo
```

O compilador raiz mantém a rota compacta:

```text
exatamente um RAF_RETURN / return / exit constante
→ IR_MOVIMM(u32)
→ emissão por arquitetura
→ .s + .hex + .bin + .ops schema 4
```

Não existe retorno fixo, NOP de sucesso ou reclassificação silenciosa.

## Rotas existentes

| Rota | Contrato | Saída | Limite |
|---|---|---|---|
| `ROOT_U32_IR` | uma expressão constante | `.s/.hex/.bin/.ops` | não é parser geral |
| `STRICT_C_REWRITE` | C/C++ na superfície explícita | `.so + receipt` | uma translation unit, sem includes externos |
| `HOSTED_RAF_KERNEL` | uma anotação pura em 14 perfis | C estrito → `.so + receipt` | não compila a linguagem completa |
| `DIRECT_ASSEMBLY` | assembler do target | `.so + receipt` | depende da ISA/toolchain alvo |

Inventário de máquina:

```text
ci/contracts/apkc_compiler_station_v2.json
```

## Emissão

O backend raiz codifica o valor real:

- x86-64: `mov eax, imm32; ret`;
- ARM64: `movz/movk/ret`;
- ARM32 Thumb-2: `movw/movt/bx lr`;
- RV64: `lui/addi/jalr`.

Os gates exercitam C ARM64, C ARM32, C++ ARM64, `RAF_KERNEL` Python ARM64, arquitetura host e semântica local da emulação. Outras combinações permanecem implementadas, mas condicionais à prova no target.

## Superfície C assimilada

```text
memcpy memmove memset memcmp memchr
strlen strnlen strcmp strncmp strncpy strchr strrchr
atoi strtoul raf_write putchar puts
```

Propriedades:

- zero heap e GC;
- storage estático ou do chamador;
- `memmove` sem comparação relacional indefinida de ponteiros;
- `atoi/strtoul` com overflow determinístico;
- `strncpy` exige capacidade explícita;
- `strcpy`, `strcat` e `gets` são proibidos;
- exports C++ usam `extern "C"`;
- larguras inteiras são verificadas na compilação.

Headers assimiláveis:

```text
stddef.h stdint.h stdbool.h stdio.h stdlib.h string.h
```

A presença do header não libera toda a API. Função não emulada, header hospedado ou include local sem resolução falha fechado.

## `RAF_KERNEL`

Perfis:

```text
rs kt java py sh pl js php jsx go rb swift groovy clj
```

Forma:

```text
RAF_KERNEL mix(a,b) = ((a ^ b) + 7) & 0xffffffff
```

Regras:

- uma anotação independente;
- até quatro `uint32_t`;
- nenhuma chamada ou estado;
- divisão/módulo por constante não nula;
- shift constante `0..31`;
- semântica `uint32_modulo`.

Rewrite e lowering continuam `claim_allowed=false`; somente o ELF selado que atravessou todos os gates recebe recibo `STRICT_ELF_PASS`.

## Cadeia de custódia

### `.ops` schema 4

Assina arquitetura, marca, cores, linguagem, flags, hash da fonte, métricas Ω, IR/ASM/BIN, `ir_value`, emissor, estado nativo, rollback e transação com FNV-1a 64 canônico.

```text
COMMITTED   → artefatos atuais e coerentes
ROLLED_BACK → nenhum .s/.hex/.bin anterior
```

### `.so.receipt.json`

Registra SHA-256 da fonte/saída, target, compilador, gates, dependências de build e zero dependência externa no runtime final. `.so` e recibo são tratados como uma única transação: se o recibo não fechar, a saída não permanece promovida.

## Perfis ELF

### `exec`

```text
ELF EXEC
sem PT_INTERP
sem PT_DYNAMIC
sem relocação residual
```

### `android-so`

```text
ELF DYN
PT_DYNAMIC + DT_SONAME permitidos
sem PT_INTERP
sem DT_NEEDED
sem símbolo indefinido
somente relocação relativa autorizada
```

Ambos bloqueiam build-id, pilha executável, LOAD RWX, `RPATH`, `RUNPATH`, `TEXTREL` e seções de exceção/runtime.

## Transação

```text
.tmp → flush/close → auditoria → rename → COMMITTED
erro → remover temporários e saídas antigas → ROLLED_BACK
```

```text
FILE_EXISTS ≠ PASS
OLD_FILE ≠ NEW_RESULT
OUTPUT_WITHOUT_RECEIPT ≠ COMMITTED
```

## Comandos

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

## Gates bloqueantes

```text
G00 inventário canônico
G01 source bounded
G02 contrato único
G03 superfície hosted fail-closed
G04 libc adversarial
G05 objeto estrito
G06 link sem undefined
G07 perfil ELF e máquina
G08 sem PT_INTERP/DT_NEEDED/RWX/exec-stack/build-id
G09 mesma fonte → mesma saída
G10 valores diferentes → bytes diferentes
G11 assinatura .ops recomposta
G12 adulteração rejeitada
G13 rollback remove stale
G14 C++ sem mangling
G15 .so + receipt transacionais
```

## Não reivindicado

- compilador geral de C/C++ ou linguagens hospedadas;
- assinatura, instalação e launch de APK;
- NativeActivity observada em dispositivo;
- runtime GPU/DSP/NPU;
- equivalência de timing, energia ou side channel;
- execução x86-64/RV64 sem target;
- verdade científica ou semântica derivada de Ω.

Após CI verde:

```text
HOTFIX_VERIFIED_BY_CI ≠ APK_RUNTIME_PROVEN
```

## R3

```text
F_ok   = compilador delimitado + transação + recibos + gates adversariais
F_gap  = dispositivo Android e targets condicionais
F_next = empacotar a saída selada numa prova única assinatura→instalação→launch→logcat
```
