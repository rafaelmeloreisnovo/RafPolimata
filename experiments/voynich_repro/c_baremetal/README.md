# Voynich bare-metal audit layer

Esta camada preserva dois programas distintos:

- `voynich_angular_v5.c`: detector heurístico em PGM e estimador angular;
- `voy_core_v2.c`: tokenização, famílias, transições, CRC32 e matriz PGM.

## Estado

```text
x86_64 build/runtime                 = PASS
AArch64 cross-build                  = PASS
AArch64 physical runtime             = TOKEN_VAZIO_RUNTIME_RECEIPT_PENDING
angle kernel synthetic vectors       = PASS
angle detector end-to-end synthetic  = FAIL
claim_allowed                        = false
```

## x86_64

```sh
gcc -O2 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
  -nostdlib -ffreestanding -fno-builtin -static \
  -o voy_angular_v5_x86 \
  start_angular_x86_64.s sys_x86_64_fixed.s voynich_angular_v5.c

./voy_angular_v5_x86 --test
```

```sh
gcc -O2 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
  -nostdlib -ffreestanding -fno-builtin -static \
  -o voy_barecore_x86 \
  start_core_x86_64.s sys_x86_64_fixed.s voy_core_v2.c

./voy_barecore_x86 entrada.txt saida 0x3F
```

## AArch64 / Termux

```sh
clang -O2 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
  -nostdlib -ffreestanding -fno-builtin -static -fuse-ld=lld \
  -Wl,-e,_start -o voy_angular_v5_aarch64 \
  start_angular_aarch64.s sys_aarch64_fixed.s voynich_angular_v5.c
```

```sh
clang -O2 -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow \
  -nostdlib -ffreestanding -fno-builtin -static -fuse-ld=lld \
  -Wl,-e,_start -o voy_barecore_aarch64 \
  start_core_aarch64.s sys_aarch64_fixed.s voy_core_v2.c
```

## Gate científico

O teste de direção usa oito bins e o crítico aproximado de qui-quadrado para `df=7`, `p=0,05`:

```text
Chi2*100 > 1407
```

A aproximação assintótica somente é promovida quando `n>=40`, para manter contagem esperada mínima de cinco por bin. Mesmo nesse caso:

```text
direcionalidade detectada != prova de hipertexto medieval
```

O detector completo ainda não recupera todos os ângulos sintéticos e permanece bloqueado até ground truth anotado, precisão/recall, MAE angular pré-registrado e controles negativos.
