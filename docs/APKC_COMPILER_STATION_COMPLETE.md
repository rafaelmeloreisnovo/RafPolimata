# APKc / RAF Compiler — Estação completa e endurecida

**Estado no código:** `IMPLEMENTED_HOTFIX`  
**Estado da evidência neste branch:** `CI_PENDING / claim_allowed=false`  
**Escopo:** compilação determinística de núcleo `u32`, C/C++ estrito, lowering portátil `RAF_KERNEL`, emissão nativa e selagem de `.so` Android.  
**Documento detalhado do HOTFIX:** [`APKC_HOTFIX_OPERATIONAL_EXCELLENCE.md`](APKC_HOTFIX_OPERATIONAL_EXCELLENCE.md).

## 1. Pipeline canônico

```text
fonte
→ detectar rota
→ validar contrato delimitado
→ rewrite ou lowering dependente da fonte
→ objeto freestanding
→ link estrito
→ auditoria ELF por perfil
→ promoção atômica do artefato
→ recibo com hashes e fronteira de claim
```

O compilador raiz também mantém a rota compacta:

```text
RAF_RETURN / return / exit com expressão constante única
→ IR_MOVIMM(u32)
→ emissão por arquitetura
→ .s + .hex + .bin + .ops schema 4
```

Não existe retorno fixo, NOP de sucesso ou reclassificação silenciosa.

## 2. Rotas existentes

| Rota | Contrato | Saída | O que não significa |
|---|---|---|---|
| `ROOT_U32_IR` | exatamente uma expressão constante | `.s/.hex/.bin/.ops` | compilador geral da linguagem |
| `STRICT_C_REWRITE` | C/C++ em superfície explicitamente emulada | `.so + receipt` | libc completa ou múltiplas translation units |
| `HOSTED_RAF_KERNEL` | uma anotação de kernel puro em 14 perfis | C estrito → `.so + receipt` | Python/JVM/Node/Go/Swift completos |
| `DIRECT_ASSEMBLY` | assembler aceito pelo target | `.so + receipt` | portabilidade automática entre ISAs |

Inventário legível por máquina:

```text
ci/contracts/apkc_compiler_station_v2.json
```

## 3. Emissão por arquitetura

O backend raiz codifica o valor real do IR:

- x86-64: `mov eax, imm32; ret`;
- ARM64: `movz` + `movk` quando necessário + `ret`;
- ARM32/Thumb-2: `movw` + `movt` quando necessário + `bx lr`;
- RV64: `lui/addi` ou `addi` + `jalr`.

A selagem Android tem targets explícitos para ARM64, ARM32, x86-64 e RV64. Os testes bloqueantes desta estação exercitam diretamente C ARM64, C ARM32, C++ ARM64 e `RAF_KERNEL` Python ARM64. Demais combinações permanecem implementadas, mas condicionais à evidência de target.

## 4. Superfície C assimilada

```text
memcpy memmove memset memcmp memchr
strlen strnlen strcmp strncmp strcpy strncpy strchr strrchr
atoi strtoul raf_write putchar puts
```

Propriedades:

- zero heap;
- zero GC;
- storage estático ou do chamador;
- `memmove` sem comparação relacional indefinida de ponteiros;
- `strtoul` com base, prefixo, `endptr`, sinal e overflow delimitados;
- exports C++ com `extern "C"`;
- assertions de largura inteira.

Headers assimiláveis:

```text
stddef.h stdint.h stdbool.h stdio.h stdlib.h string.h
```

Isso não libera todas as APIs desses headers. Chamadas não emuladas falham fechadas. Includes externos ou headers hospedados sem implementação não são apagados silenciosamente.

## 5. Contrato `RAF_KERNEL`

Perfis aceitos:

```text
rs kt java py sh pl js php jsx go rb swift groovy clj
```

Forma:

```text
RAF_KERNEL mix(a,b) = ((a ^ b) + 7) & 0xffffffff
```

Regras:

- exatamente uma anotação independente;
- até quatro argumentos `uint32_t`;
- nenhuma chamada ou acesso a estado;
- divisão/módulo somente por constante não nula;
- shift somente por constante `0..31`;
- resultado em semântica `uint32_modulo`.

O manifesto intermediário permanece `claim_allowed=false`. Apenas o `.so` que atravessou os gates recebe recibo `STRICT_ELF_PASS`.

## 6. Cadeia de custódia

### `.ops` schema 4

Campos assinados incluem arquitetura, marca, cores, linguagem, flags, hash da fonte, métricas Ω, contagens de artefatos, `ir_value`, versão do emissor, estado nativo, rollback e transação.

```text
COMMITTED   → saída coerente
ROLLED_BACK → sem .s/.hex/.bin antigo
```

O validador recomputa FNV-1a 64 com o offset canônico e rejeita alteração em qualquer campo assinado.

### `.so.receipt.json`

Registra SHA-256 da fonte/saída, target, compilador, gates, dependências do plano de construção e ausência de dependências externas no runtime final.

## 7. Perfis ELF

### Executável estrito

```text
ELF EXEC
sem PT_INTERP
sem PT_DYNAMIC
sem relocação residual
```

### Android shared object

```text
ELF DYN
PT_DYNAMIC e DT_SONAME permitidos para o loader
sem PT_INTERP
sem DT_NEEDED
sem símbolo indefinido
somente relocação relativa autorizada
```

Ambos bloqueiam build-id, pilha executável, segmento LOAD RWX, `RPATH`, `RUNPATH`, `TEXTREL` e seções de exceção/runtime.

## 8. Escrita e rollback

Artefatos são produzidos em temporários e promovidos por `rename`. Falha sobre o mesmo `out_base` elimina resultados anteriores e deixa somente o recibo de rollback.

```text
FILE_EXISTS ≠ PASS
OLD_FILE ≠ NEW_RESULT
ROLLBACK = remoção efetiva + recibo verificável
```

## 9. Comandos únicos

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

## 10. Gates bloqueantes

```text
G00 contrato/inventário canônico
G01 source bounded
G02 rewrite/lowering único
G03 hosted surface fail-closed
G04 libc selftest adversarial
G05 objeto estrito
G06 link sem símbolo indefinido
G07 perfil ELF e máquina
G08 sem PT_INTERP/DT_NEEDED/RWX/exec-stack/build-id
G09 mesma fonte → mesma saída
G10 valores distintos → bytes distintos
G11 assinatura .ops recomputada
G12 adulteração de manifesto rejeitada
G13 rollback remove artefatos antigos
G14 C++ exports sem mangling
G15 recibo SHA-256 e claim boundary
```

## 11. Limite preservado

A estação não reivindica:

- compilação geral de C/C++ ou de linguagens hospedadas;
- assinatura, instalação ou lançamento de APK;
- runtime Android observado em dispositivo;
- execução real de drivers GPU/DSP/NPU;
- equivalência de tempo, consumo ou side channel;
- execução x86-64/RV64 sem prova no target;
- verdade científica ou semântica derivada das métricas Ω.

Após CI verde, o estado promovível é:

```text
HOTFIX_VERIFIED_BY_CI
```

Isso continua diferente de:

```text
APK_RUNTIME_PROVEN
```

## R3

```text
F_ok   = compilador delimitado + transação + recibos + gates adversariais
F_gap  = dispositivo Android e combinações condicionais de target
F_next = empacotar a saída selada e produzir uma cadeia única assinatura→instalação→launch→logcat
```
