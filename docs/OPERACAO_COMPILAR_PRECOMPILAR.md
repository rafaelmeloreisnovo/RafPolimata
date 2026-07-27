# Operação técnica: compilar e pré-compilar com baixa fricção

> **Entrada canônica:** `raf_compile_file()` para o núcleo interno e `make compile` para o backend estrito C/RAF_KERNEL.

## Ciclo 1 — lowering determinístico dependente da fonte

1. Detectar arquitetura, linguagem, otimização e recursos de CPU.
2. Calcular `src_hash` por FNV-1a 64-bit, sem heap.
3. Localizar `RAF_RETURN(...)`, `return ...` ou `exit ...` fora de comentários e strings.
4. Avaliar o subset inteiro constante com precedência formal.
5. Emitir `IR_MOVIMM(valor_u32)` + `IR_RET`.
6. Falhar quando a fonte contém variável, chamada, divisão por zero ou sintaxe não suportada.

Não existe mais fallback operacional para `return 42`. O valor 42 permanece apenas como fixture de teste.

## Ciclo 2 — emissão e materialização

1. Emitir assembly textual específico da arquitetura.
2. Emitir bytes nativos para x86-64, ARM64, ARM32/Thumb-2 ou RV64.
3. Gerar `.hex`, `.bin` opcional e manifesto `.ops`.
4. Comparar duas execuções idênticas.
5. Exigir divergência entre fontes semanticamente diferentes.

## Ciclo 3 — assimilação C e linguagens hospedadas

A rota estrita de produção é:

```text
C/C++
→ raf_c_rewrite.py
→ raf_libc_emu.h
→ clang/lld freestanding
→ ELF sem símbolo indefinido e sem PT_INTERP
```

Para linguagens que normalmente exigem VM, GC ou interpretador:

```text
fonte hospedada
→ RAF_KERNEL nome(args)=expressão
→ raf_kernel_lower.py
→ C estrito
→ mesmo backend freestanding
```

Isso assimila kernels puros sem transportar Python, JVM, Node, Go runtime, Swift runtime ou Clojure para o artefato final.

## Biblioteca C internalizada

Implementados sem heap:

- memória: `memcpy`, `memmove`, `memset`, `memcmp`;
- strings: `strlen`, `strnlen`, `strcmp`, `strncmp`, `strchr`, `strrchr`;
- conversão: `atoi`, `strtoul`;
- saída mínima: `putchar`, `puts`, `raf_write` por syscall.

Heap, `printf`, FILE, threads, dynamic loading e execução de processo falham fechados no rewriter estrito.

## Comandos

```bash
make compiler-selftest
make language-contract

make compile RAF_LANG=c RAF_ARCH=arm64 \
  SRC=tests/fixtures/strict_kernel.c \
  OUT=build/strict/libmain.so
```

## Critérios operacionais

- sem heap/GC no artefato final;
- buffers e limites explícitos;
- mesma fonte produz os mesmos bytes;
- fontes 42 e 1337 não podem produzir bytes idênticos;
- falha gera retorno não zero e manifesto de rollback;
- ARM64 e ARM32 são construídos e verificados por `readelf`;
- ausência de evidência nunca é transformada em PASS.

## Separação de estados

```text
COMPILER_STATION_PASS
≠ APK_SIGNED
≠ INSTALLED_ON_DEVICE
≠ ANDROID_RUNTIME_PROVEN
```

A estação do compilador está fechada. Assinatura, instalação, `logcat` e backends de dispositivo permanecem gates próprios.
