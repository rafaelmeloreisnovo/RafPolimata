# RAFAELIA Fractal Core — Mandelbrot, arco projetivo e Fibonacci

**Estado:** `ACTIVE`  
**Proprietário lógico:** `low-level-methods-maintainer`  
**Repositório:** [`rafaelmeloreisnovo/RafPolimata`](https://github.com/rafaelmeloreisnovo/RafPolimata) — `fractal_core/README.md`

Núcleo freestanding, sem syscall, sem libc, sem `malloc`, sem heap, sem GC,
sem ponto flutuante e sem estado global mutável.

## Ponto matemático fixado

O maior bulbo circular do conjunto de Mandelbrot é o bulbo de período 2:

- centro: `c = -1 + 0i`;
- raio: `1/4`;
- ponto de contato com a cardioide principal: `c = -3/4 + 0i`.

Esse contato é a âncora exata usada pelo módulo.

## Infinito dentro do finito

A reta real estendida é representada em coordenadas projetivas `[n:d]` e
mapeada racionalmente para o círculo:

```text
x = (n² - d²) / (n² + d²)
y = 2nd / (n² + d²)
```

Quando `d = 0`, o parâmetro representa o infinito da reta. No círculo ele é o
ponto finito `(1,0)`. Ao escalar o círculo para o bulbo de período 2, esse ponto
vira exatamente `c = -3/4`, a junção do bulbo com a cardioide.

O módulo não afirma que uma máquina calcula o infinito. Ele preserva o infinito
como ponto de fechamento projetivo e executa aproximações finitas auditáveis.

## Dois braços Fibonacci

O estado mantém o par `(F_n,F_{n+1})`. Os braços são projetados como:

```text
superior:   [ F_{n+1} : F_n ]
inferior:   [-F_{n+1} : F_n ]
recíproco:  [ F_n     : F_{n+1} ]
```

A multiplicação de ambos os termos por uma mesma escala não altera o ponto
projetivo. Por isso, quando o par cresce, os dois termos são renormalizados em
conjunto. A recorrência pode continuar indefinidamente enquanto o estado físico
permanece finito.

## Mandelbrot

O núcleo usa:

```text
z(0) = 0
z(n+1) = z(n)² + c
```

Antes do laço de escape, aplica dois testes analíticos:

1. cardioide principal;
2. bulbo circular de período 2.

Somente pontos ainda não classificados pagam o custo iterativo. A fricção vira
sinal operacional, não desperdício oculto.

## Flags de fricção

| Flag | Papel |
|---|---|
| `RAF_FRICTION_PROJECTIVE_DIV` | divisão inteira determinística necessária |
| `RAF_FRICTION_FIB_RENORMALIZED` | renormalização preservando a razão |
| `RAF_FRICTION_MANDEL_ITERATED` | ponto exigiu recorrência de escape |
| `RAF_FRICTION_SATURATED` | proteção aritmética acionada |
| `RAF_FRICTION_ARCH_FALLBACK` | multiplicação C portátil selecionada |
| `RAF_FRICTION_PROJECTIVE_INF` | ponto projetivo `[n:0]` observado |

Os comentários no código usam IDs `RAF-F001` a `RAF-F006`, ligados a essas
flags. Warnings do compilador são capturados por módulo e são fatais.

## Separação C / Assembly

C contém:

- coordenadas projetivas;
- divisor inteiro próprio;
- recorrência Fibonacci;
- mapeamento do bulbo;
- teste analítico e iteração de Mandelbrot;
- estado e flags.

Assembly contém apenas a primitiva folha:

```text
q16_mul(a,b) = (a*b) >> 16
```

Existem adaptadores diretos para:

- AArch64/AAPCS64;
- ARMv7/AAPCS32;
- x86-64/SysV.

Não há despacho em runtime. `RAF_USE_ARCH_MUL` escolhe o caminho na compilação.

## Artefatos

```sh
./build/build.sh
```

Produz:

```text
out/libraf_fractal.a           biblioteca relocável
out/raf_fractal_core.elf       imagem ELF sem símbolos
out/raf_fractal_core.bin       imagem bruta
out/raf_fractal_core.debug.elf ELF de auditoria
out/symbols.txt                símbolos antes da limpeza
out/flags/*.txt                flags exatas por módulo
out/warnings/*.log             warnings por módulo
out/test_host.txt              receipt do harness
```

Caminho C portátil:

```sh
USE_ASM=0 OUT=out_c ./build/build.sh
```

Cross-compile explícito:

```sh
CC=aarch64-linux-gnu-gcc \
LD=aarch64-linux-gnu-ld \
AR=aarch64-linux-gnu-ar \
NM=aarch64-linux-gnu-nm \
OBJCOPY=aarch64-linux-gnu-objcopy \
SIZE=aarch64-linux-gnu-size \
TARGET_ARCH=aarch64 \
./build/build.sh
```

## Flags de limpeza

O build aplica por módulo:

```text
-ffreestanding
-fno-builtin
-fno-common
-fno-stack-protector
-fno-unwind-tables
-fno-asynchronous-unwind-tables
-ffunction-sections
-fdata-sections
-fvisibility=hidden
-fno-ident
-Wall -Wextra -Wpedantic
-Wconversion -Wsign-conversion -Wshadow -Wundef
-Wstrict-prototypes -Wmissing-prototypes -Werror
```

O link aplica:

```text
--gc-sections
--fatal-warnings
--build-id=none
-z noexecstack
```

O linker descarta `.comment`, `.note`, `.eh_frame` e metadados GNU. O build
falha se encontrar qualquer símbolo externo indefinido.

## Resultado local desta versão

Em x86-64:

```text
ASM leaf:       1140 bytes de texto, data=0, bss=0, undefined=0
C fallback:     1036 bytes de texto, data=0, bss=0, undefined=0
host tests:     PASS, 2000 passos
warnings:       0
```

A diferença mostra uma régua importante: Assembly só permanece quando a ISA ou
o controle binário justificam seu custo. No host testado, o compilador C gerou
uma imagem menor; portanto o Assembly é um adaptador opcional, não um dogma.

## Limite honesto

`max_iter` não prova pertencimento matemático ao Mandelbrot. Um ponto que não
escapa dentro do limite recebe `RAF_MANDEL_MAX_ITER`; a conclusão exata continua
aberta, salvo quando um teste analítico o classificou.
