# APKc / RAF Compiler — Estação concluída

Estado: `IMPLEMENTED / TESTED_LOCAL / CI_GATED`  
Escopo: compilação determinística de núcleo `u32`, assimilação de C freestanding, lowering portátil entre linguagens e emissão estrita ARM64/ARM32.  
Política: ausência de suporte falha fechada; nenhum valor fixo é emitido como se fosse compilação real.

## 1. O que foi encerrado

A antiga pré-compilação emitia sempre `return 42`, independentemente da fonte. Essa âncora foi removida do caminho operacional. O pipeline agora é:

```text
fonte
→ detecção de linguagem
→ lowering dependente da fonte
→ IR_MOVIMM(valor real da expressão)
→ emissão por arquitetura
→ bytes nativos
→ manifesto .ops
→ gates de reprodutibilidade e divergência
```

O núcleo interno reconhece um contrato limitado e explícito de retorno `u32`:

```c
RAF_RETURN((0x100 + 7) ^ 3)
return 6 * 7;
```

Operadores aceitos: `+ - * / % << >> & ^ | ~` e parênteses. Expressão variável, chamada de função, divisão por zero ou sintaxe fora do contrato falha com rollback; não há fallback para 42, NOP ou sucesso textual.

## 2. Emissão real por arquitetura

`raf_asm_emit.c` decodifica o valor do IR e produz bytes diferentes para fontes diferentes:

- x86-64: `mov eax, imm32; ret`;
- ARM64: `movz` + `movk` quando necessário + `ret`;
- ARM32/Thumb-2: `movw` + `movt` quando necessário + `bx lr`;
- RV64: `lui/addi` ou `addi` + `jalr`.

O gate compara duas compilações da mesma fonte e exige igualdade, depois compara fontes `42` e `1337` e exige bytes diferentes.

## 3. C sem libc final

`Apkc/raf_libc_emu.h` fornece implementações internas, bounded e sem heap para:

```text
memcpy memmove memset memcmp
strlen strnlen strcmp strncmp strchr strrchr
atoi strtoul putchar puts
```

`malloc`, `calloc`, `realloc` e `free` falham por construção. `printf`, arquivos hospedados, threads, dynamic loading e saltos de runtime são rejeitados pelo rewriter.

`scripts/raf_c_rewrite.py`:

1. remove headers hospedados assimiláveis;
2. injeta `raf_libc_emu.h`;
3. identifica chamadas emuladas;
4. rejeita runtime proibido;
5. grava hashes de entrada/saída e manifesto de transformação.

A ferramenta externa pode existir no plano de construção. O `.so` final é ligado com `-nostdlib`, `-nostartfiles`, `-nodefaultlibs`, `--no-undefined`, sem `PT_INTERP` e sem símbolo indefinido.

## 4. Linguagens hospedadas sem carregar seus runtimes

Para Kotlin, Java, Python, Shell, Perl, JavaScript, PHP, JSX, Go, Ruby, Swift, Groovy, Clojure e Rust assimilado, o contrato portátil é uma anotação única:

```text
RAF_KERNEL mix(a,b) = ((a ^ b) + 7) & 0xffffffff
```

`scripts/raf_kernel_lower.py` valida a expressão, gera C estrito e encaminha para o mesmo backend ARM64/ARM32. A anotação extrai um kernel puro; ela não afirma compilar toda a semântica da linguagem original.

Sem `RAF_KERNEL`, a rota estrita falha. O runtime de Python, JVM, Node, Go, Swift ou Clojure nunca é ocultado dentro do claim freestanding.

## 5. Entrada operacional única

```bash
make compile \
  RAF_LANG=c \
  RAF_ARCH=arm32 \
  SRC=tests/fixtures/strict_kernel.c \
  OUT=build/strict/libmain.so

make compiler-selftest
make language-contract
```

O target `compile-plan` continua disponível somente para auditoria. O target `compile` executa de fato.

## 6. Gates fechados

```text
G0 source exists
G1 rewrite/lowering valid
G2 strict object compiles
G3 shared ELF links with no undefined symbols
G4 no PT_INTERP
G5 ARM64 and ARM32 class/machine validated
G6 same source → same bytes/manifest
G7 different source value → different bytes
G8 invalid expression/runtime → nonzero rollback
```

## 7. Limite preservado

A estação do compilador está concluída no escopo acima. Permanecem como gates externos, não como lacunas do compilador:

- assinatura APK;
- instalação e lançamento em Android real;
- `ANativeActivity_onCreate` observado em `logcat`;
- drivers e loaders GPU/DSP/NPU;
- compilação geral e irrestrita de todas as construções de C/C++/Rust ou linguagens hospedadas.

Esses itens exigem dispositivo, toolchain ou semântica adicional e continuam separados para impedir promoção falsa.

## R3

```text
F_ok   = lowering dependente da fonte + libc assimilada + ARM64/ARM32 + CI bloqueante
F_gap  = prova Android no dispositivo e semântica geral fora do subset puro
F_next = usar o compilador concluído para gerar o primeiro APK assinado e capturar runtime no mesmo run
```
