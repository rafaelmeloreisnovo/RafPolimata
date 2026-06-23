# ApkC — flags, comandos e limites atuais

Este documento descreve o estado atual do ApkC: como compilar, como gerar APK, quais opções existem na CLI e quais limites existem por ele ser um gerador freestanding/minimalista.

## Natureza do ApkC

`Apkc/apkc.c` é descrito no próprio arquivo como:

```c
/* apkc.c — freestanding APK compiler, no libc, no heap, no abstractions */
```

Ele inclui diretamente as camadas internas:

```c
#include "sys.h"
#include "mem.h"
#include "arch_arm64.h"
#include "arch_arm32.h"
#include "fmt_zip.h"
#include "fmt_dex.h"
#include "fmt_axml.h"
#include "fmt_elf.h"
```

Ou seja: a intenção estrutural é gerar APK sem depender de Gradle/Android Studio, usando buffers próprios, formatos próprios e syscalls próprias.

## Compilação no Termux/Android ARM32

### Comando que funcionou na prova real

```sh
cd "$HOME/RafPolimata-main/Apkc"
mkdir -p out

cc -std=c11 -Oz -Wno-unused-function \
  -nostartfiles -Wl,-e,_start \
  apkc.c -o out/apkc
```

### Por que `-nostartfiles` funcionou

- `-nostartfiles` remove o startup padrão/crt, permitindo usar o `_start` próprio do ApkC.
- Mas ainda deixa o linker resolver libc/builtins necessários no Termux, como `strlen` e `__aeabi_*`.

### Por que `-nostdlib` falhou no Termux ARM32

`-nostdlib` remove tudo: startup, libc e builtins. No teste real, isso gerou símbolos indefinidos:

```text
strlen
__aeabi_llsr
__aeabi_uidivmod
__aeabi_uldivmod
```

Portanto, para o teste simples no Termux, usar:

```text
-nostartfiles
```

em vez de:

```text
-nostdlib
```

## Compilação mais dura / freestanding puro

Para manter o ideal freestanding total, o caminho correto futuro é eliminar dependências residuais de libc/builtins ou prover substitutos internos.

Alvo futuro:

```sh
cc -std=c11 -Oz -ffreestanding -fno-builtin \
  -nostdlib -Wl,-e,_start \
  apkc.c -o out/apkc
```

Mas esse modo só deve virar claim quando resolver internamente:

- `strlen` residual ou qualquer builtin emitido pelo compilador;
- divisão/modulo 32/64 em ARM32 (`__aeabi_*`);
- shifts 64-bit que o compilador baixa para helpers;
- qualquer chamada implícita gerada por otimização.

## Entrada e saída do ApkC

Formato geral:

```sh
out/apkc [options] source.s
```

Exemplo mínimo ARM32:

```sh
out/apkc hello.s.txt \
  -o out/hello.apk \
  -p com.rafael.teste \
  -l RafaelTeste \
  -n hello \
  -32
```

Exemplo ARM64:

```sh
out/apkc hello.s.txt \
  -o out/hello-arm64.apk \
  -p com.rafael.teste64 \
  -l RafaelTeste64 \
  -n hello \
  -64
```

Exemplo ambos ABIs:

```sh
out/apkc hello.s.txt \
  -o out/hello-both.apk \
  -p com.rafael.teste.both \
  -l RafaelBoth \
  -n hello \
  -both
```

## Flags da CLI do ApkC

| Flag | Valor | Função |
|---|---|---|
| `-o` | arquivo | caminho do APK de saída |
| `-p` | package | nome do pacote Android |
| `-l` | label | label do app no Manifest |
| `-n` | nome | nome da biblioteca nativa sem `lib` e sem `.so` |
| `-m` | sdk | `minSdkVersion` |
| `-t` | sdk | `targetSdkVersion` |
| `-64` | nenhum | gera somente `lib/arm64-v8a/lib<nome>.so` |
| `-32` | nenhum | gera somente `lib/armeabi-v7a/lib<nome>.so` |
| `-both` | nenhum | gera ARM64 e ARM32; é o default interno |

Defaults internos atuais:

```text
outpath = out.apk
pkg = com.example.app
label = App
libname = main
min_sdk = 21
target_sdk = 33
do64 = 1
do32 = 1
```

## O que o APK gerado contém

O `build_apk` monta:

- `AndroidManifest.xml` em AXML;
- `classes.dex` mínimo;
- `lib/arm64-v8a/lib<name>.so`, se `-64` ou `-both`;
- `lib/armeabi-v7a/lib<name>.so`, se `-32` ou `-both`;
- ZIP/APK sem compressão avançada.

## Linguagem de entrada: não é C completo

O arquivo de entrada `hello.s.txt` é assembly simplificado, não C completo.

O ApkC atual não compila C arbitrário para APK. Ele lê um arquivo `.s`/texto assembly simples e emite código ARM32/ARM64 via encoders próprios.

Portanto:

```text
C completo -> NÃO
subset assembly -> SIM
```

Se quiser aceitar C no futuro, precisa de um frontend C real, IR, lowered ops e backend. O repositório tem outros arquivos `raf_*` relacionados a compilação/otimização, mas o ApkC provado aqui usa entrada assembly simples.

## Tokens e sintaxe suportada pelo lexer

O lexer reconhece:

- identificadores com letras, `_`, `.`, `@`;
- inteiros decimais;
- inteiros hexadecimais `0x...`;
- strings entre aspas;
- `,`, `[`, `]`, `#`, `:`, `+`, `-`, `!`;
- comentários `;` e `//`.

## Diretivas especiais

| Diretiva | Função |
|---|---|
| `.sym1` | marca offset usado como símbolo exportado `ANativeActivity_onCreate` |
| `.sym2` | marca offset usado como símbolo exportado `android_main` |
| `.word` | emite literal bruto de 32 bits |

## Instruções ARM64 atualmente reconhecidas

Lista observada no parser atual:

```text
nop
ret
ret xN
brk
svc
blr
br
movz
movk
movn
mov
add
sub
and
orr
eor
cmp
csel
ldr
str
adr
adrp
b
bl
beq
bne
blt
bgt
ble
bge
cbz
cbnz
stp
ldp
.word
```

Observações:

- `mov rd, #imm64` expande para cadeia `movz/movk`.
- `mov rd, rn` vira forma equivalente baseada em `orr` com zero-register.
- Branch com label existe via patch interno.
- Condicionais aceitas no formato curto `beq`, `bne`, etc.; não está documentado como suporte pleno a `b.eq`.

## Instruções ARM32 atualmente reconhecidas

Lista parcial/observada no parser atual:

```text
nop
bx
swi
svc
push
pop
movw
movt
mov
add
sub
cmp
.word
```

Também há suporte a outras formas no trecho restante do parser ARM32, mas o claim seguro deve ser restrito ao que já foi usado/testado em prova real.

## O que é seguro dizer agora

```text
ApkC gera APK Android mínimo a partir de assembly simplificado, com AXML, DEX mínimo, ELF ARM32/ARM64 e ZIP/APK próprios.
```

Após a prova de 2026-06-14, também é seguro dizer:

```text
No Termux/Android ARM32, o ApkC compilado com -nostartfiles gerou APK ARM32, assinado e instalado com sucesso observado.
```

## O que ainda não é seguro dizer

- Que compila C completo.
- Que substitui NDK/Gradle em produção.
- Que suporta todo ARMv7/AArch64.
- Que o APK roda lógica útil complexa além do mínimo NativeActivity.
- Que é totalmente `-nostdlib` no Termux ARM32 sem helpers internos.
- Que já é compatível com todos Androids.

## Próximos comandos úteis

### Gerar prova mínima novamente

```sh
cd "$HOME/RafPolimata-main/Apkc"
sh ../testa_apkc_simples.sh
```

### Verificar AXML

```sh
aapt dump xmltree out/hello.apk AndroidManifest.xml
```

### Verificar ELF

```sh
unzip -p out/hello.apk lib/armeabi-v7a/libhello.so > out/libhello.so
readelf -h out/libhello.so
readelf -s out/libhello.so | head -80
```

### Verificar DEX SHA-1

```sh
python3 - <<'PY'
import zipfile, hashlib
apk="out/hello.apk"
dex=zipfile.ZipFile(apk).read("classes.dex")
header=dex[12:32]
calc=hashlib.sha1(dex[32:]).digest()
print("header:", header.hex())
print("calc:  ", calc.hex())
print("PASS" if header == calc else "FAIL")
PY
```

### Assinar

```sh
apksigner sign \
  --ks out/debug.keystore \
  --ks-key-alias apkc-debug \
  --ks-pass pass:android \
  --key-pass pass:android \
  --min-sdk-version 21 \
  --out out/hello-signed.apk \
  out/hello.apk
```

### Verificar assinatura

```sh
apksigner verify --verbose --print-certs out/hello-signed.apk
```

### Confirmar instalação

```sh
cmd package list packages | grep com.rafael.teste
```

---

## Matriz de Suporte Arquitetura × Linguagem (B2 — 2026-06-20)

Tabela derivada diretamente de `Apkc/lang_profile.h` (_lang_table, LP_COUNT=17).

| Lang | ID | Pipeline | ARM64 | ARM32 | Ferramenta externa | Saída |
|------|:--:|----------|:-----:|:-----:|-------------------|-------|
| asm | LP_ASM=0 | ASM_INTERNAL | ✓ | ✓ | nenhuma | lib/arm64-v8a/ + lib/armeabi-v7a/ |
| c | LP_C=1 | NATIVE_SO | ✓ | — | clang aarch64-linux-android | lib/arm64-v8a/ |
| cpp | LP_CPP=2 | NATIVE_SO | ✓ | — | clang++ aarch64-linux-android | lib/arm64-v8a/ |
| rs | LP_RS=3 | NATIVE_SO | ✓ | — | rustc aarch64-linux-android | lib/arm64-v8a/ |
| kt | LP_KT=4 | DEX (d8) | ✓ | ✓ | kotlinc + d8 | classes.dex |
| java | LP_JAVA=5 | DEX (d8) | ✓ | ✓ | javac + d8 | classes.dex |
| py | LP_PY=6 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/python3 (runtime) | lib/arm64-v8a/ |
| sh | LP_SH=7 | SCRIPT_BOOTSTRAP | ✓ | — | /bin/sh (runtime) | lib/arm64-v8a/ |
| pl | LP_PL=8 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/perl (runtime) | lib/arm64-v8a/ |
| js | LP_JS=9 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/node (runtime) | lib/arm64-v8a/ |
| php | LP_PHP=10 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/php (runtime) | lib/arm64-v8a/ |
| jsx | LP_JSX=11 | JSX_BOOTSTRAP | ✓ | — | npx/babel + node (runtime) | lib/arm64-v8a/ |
| go | LP_GO=12 | NATIVE_SO | ✓ | — | go build -buildmode=c-shared | lib/arm64-v8a/ |
| rb | LP_RB=13 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/ruby (runtime) | lib/arm64-v8a/ |
| swift | LP_SWIFT=14 | NATIVE_SO | ✓ | — | swiftc -emit-library | lib/arm64-v8a/ |
| groovy | LP_GROOVY=15 | DEX (d8) | ✓ | ✓ | groovyc + d8 | classes.dex |
| clj | LP_CLJ=16 | SCRIPT_BOOTSTRAP | ✓ | — | /usr/bin/clojure (runtime) | lib/arm64-v8a/ |

### Legenda

- **ASM_INTERNAL**: montador ARM de 2 passes embutido no ApkC (nenhuma ferramenta externa necessária)
- **NATIVE_SO**: fork+exec do compilador nativo → ELF64 .so
- **DEX**: fork+exec compilador JVM → d8 → classes.dex (JVM runtime independente de ABI nativa)
- **SCRIPT_BOOTSTRAP**: gen_script_code64() gera ELF64 .so que executa o intérprete passando o fonte inline
- **JSX_BOOTSTRAP**: Babel pré-processa JSX → JS → gen_script_code64() com Node.js

### Nota sobre ARM32

Somente `asm` (ASM_INTERNAL) gera `lib/armeabi-v7a/`. Idiomas DEX (kt/java/groovy) são ABI-neutros por rodarem na JVM. Todos os outros são `arm64_only=1` na tabela.
