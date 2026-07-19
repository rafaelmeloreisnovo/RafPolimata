# Pipeline profissional C freestanding — Termux/Android

## Escopo

Este corpo separa três provas que antes podiam ser confundidas:

1. **núcleo C freestanding:** objeto sem `libc`, heap ou símbolos indefinidos;
2. **cross-build Android:** ELF estático ARM32 e ARM64 produzido pelo NDK, sem execução falsa no runner x86;
3. **execução Termux:** o mesmo núcleo é compilado e executado no aparelho ARM real, usando somente `_start` e o syscall Linux/Android de saída.

A relação canônica é:

```text
fonte auditável
→ objeto freestanding
→ ELF por ABI
→ verificação estrutural
→ execução em Termux no aparelho
→ evidência registrada
```

`cross-build PASS` não significa `device runtime PASS`. A execução real só é promovida quando `termux_freestanding_build.sh --run` termina com código zero no Android correspondente.

## Núcleo Omega indexado

O estado usa uma única página de palavras de 32 bits:

```text
q[0..7]    estado e prova
q[8..15]   oito coeficientes direcionais Q8
q[16..1015] matriz 10×10×10
```

A escolha por índices preserva posição e reduz metadados estruturais em runtime. O código-fonte ainda possui identificadores porque auditoria, revisão e manutenção exigem linguagem verificável. Em artefatos de release:

- `-g0` remove informação de depuração;
- `-fvisibility=hidden` oculta símbolos por padrão;
- `llvm-strip --strip-all` remove a tabela de nomes do ELF final;
- nomes de variáveis locais não constituem overhead do binário otimizado.

Portanto, “sem nomear variável” é tratado corretamente como **ausência de nomes no artefato executável**, não como destruição da legibilidade da fonte.

## Fricção como operador

Os oito coeficientes `q[8..15]` estão no intervalo `0..255` e funcionam como atenuação Q8 por direção. Eles podem representar relações distintas de organização sem introduzir ponto flutuante, divisão de runtime ou biblioteca matemática.

```text
axis 0..7 → coefficient 0..255 → attenuation of the selected transition
```

Este termo é matemático/computacional. Não constitui, sozinho, afirmação sobre atrito físico, viscosidade ou dissipação material.

## Invariantes

| Gate | Prova |
|---|---|
| Sem heap | scanner bloqueia `malloc/calloc/realloc/free` |
| Sem libc | scanner bloqueia headers e chamadas hospedadas |
| Sem runtime implícito | `nm -u` deve retornar vazio |
| Tamanho controlado | objeto host limitado a 4096 bytes de texto |
| Determinismo | dois estados com os mesmos vetores terminam com o mesmo digest |
| Android estático | ELF `EXEC`, sem `DT_NEEDED` |
| ARM32/ARM64 | builds separados por ABI e entrypoint Assembly próprio |
| Símbolos removidos | release passa por `llvm-strip --strip-all` |
| Runtime real | somente o aparelho Termux pode promover o gate de execução |

## Uso local host

```sh
sh scripts/validate_omega_freestanding.sh
```

## Cross-build Android NDK

```sh
export ANDROID_NDK_HOME="$HOME/Android/Sdk/ndk/27.2.12479018"
sh scripts/android_freestanding_matrix.sh --build
sh scripts/android_freestanding_matrix.sh --verify
```

Saídas:

```text
build/freestanding-android/armeabi-v7a/omega_core
build/freestanding-android/arm64-v8a/omega_core
build/freestanding-android/manifest.sha256
```

## Termux Android

Dependência mínima:

```sh
pkg install clang
```

Planejar sem compilar:

```sh
sh scripts/termux_freestanding_build.sh --plan
```

Compilar:

```sh
sh scripts/termux_freestanding_build.sh --build
```

Compilar e executar no aparelho:

```sh
sh scripts/termux_freestanding_build.sh --run
```

O programa não usa `main`, `printf`, linker dinâmico ou `libc`. O entrypoint Assembly cria o estado na pilha, chama o núcleo C, valida o digest e encerra pelo syscall da arquitetura.

## Estados epistemológicos

```text
SOURCE_AUDITED      = scanner + compilação estrita
HOST_OBJECT_PASS    = objeto sem undefined e teste determinístico
ANDROID_CROSS_PASS  = ELF estruturalmente válido para a ABI
TERMUX_DEVICE_PASS  = execução real no aparelho com exit 0
TOKEN_VAZIO         = etapa não executada ou prova ausente
```

Nenhum estado superior é inferido automaticamente de um estado inferior.
