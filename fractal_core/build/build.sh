#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=${OUT:-"$ROOT/out"}
CC=${CC:-cc}
LD=${LD:-ld}
AR=${AR:-ar}
NM=${NM:-nm}
OBJCOPY=${OBJCOPY:-objcopy}
SIZE=${SIZE:-size}
TARGET_ARCH=${TARGET_ARCH:-host}
USE_ASM=${USE_ASM:-1}
OPT=${OPT:--Os}

rm -rf "$OUT"
mkdir -p "$OUT/obj" "$OUT/warnings" "$OUT/flags"

COMMON_FLAGS="$OPT -std=c11 -ffreestanding -fno-builtin -fno-common \
-fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables \
-ffunction-sections -fdata-sections -fvisibility=hidden -fno-ident \
-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow \
-Wundef -Wstrict-prototypes -Wmissing-prototypes -Werror \
-I$ROOT/include"

PROJECTIVE_FLAGS="$COMMON_FLAGS -fno-tree-vectorize -fno-unroll-loops"
FIB_FLAGS="$COMMON_FLAGS -fno-unroll-loops"
MANDEL_FLAGS="$COMMON_FLAGS -fno-unroll-loops"
CORE_FLAGS="$COMMON_FLAGS"

if [ "$USE_ASM" = 1 ]; then
    COMMON_DEFINE="-DRAF_USE_ARCH_MUL=1"
else
    COMMON_DEFINE="-DRAF_USE_ARCH_MUL=0"
fi

compile_module() {
    name=$1
    src=$2
    flags=$3
    printf '%s\n' "$flags $COMMON_DEFINE" > "$OUT/flags/$name.txt"
    if ! $CC $flags $COMMON_DEFINE -c "$src" -o "$OUT/obj/$name.o" \
        2>"$OUT/warnings/$name.log"; then
        cat "$OUT/warnings/$name.log" >&2
        exit 1
    fi
    if [ -s "$OUT/warnings/$name.log" ]; then
        cat "$OUT/warnings/$name.log" >&2
        exit 1
    fi
}

compile_module projective "$ROOT/src/raf_projective.c" "$PROJECTIVE_FLAGS"
compile_module fibonacci  "$ROOT/src/raf_fibonacci_arc.c" "$FIB_FLAGS"
compile_module mandelbrot "$ROOT/src/raf_mandelbrot.c" "$MANDEL_FLAGS"
compile_module core       "$ROOT/src/raf_fractal_core.c" "$CORE_FLAGS"

ASM_OBJ=""
if [ "$USE_ASM" = 1 ]; then
    case "$TARGET_ARCH" in
        host)
            case "$(uname -m)" in
                x86_64) ASM_SRC="$ROOT/arch/x86_64/raf_q16_mul.S" ;;
                aarch64) ASM_SRC="$ROOT/arch/aarch64/raf_q16_mul.S" ;;
                armv7l|armv8l) ASM_SRC="$ROOT/arch/armv7/raf_q16_mul.S" ;;
                *) echo "unsupported host ISA; set USE_ASM=0" >&2; exit 2 ;;
            esac
            ;;
        x86_64) ASM_SRC="$ROOT/arch/x86_64/raf_q16_mul.S" ;;
        aarch64) ASM_SRC="$ROOT/arch/aarch64/raf_q16_mul.S" ;;
        armv7) ASM_SRC="$ROOT/arch/armv7/raf_q16_mul.S" ;;
        *) echo "unknown TARGET_ARCH=$TARGET_ARCH" >&2; exit 2 ;;
    esac
    $CC -c "$ASM_SRC" -o "$OUT/obj/arch_mul.o" 2>"$OUT/warnings/arch_mul.log"
    test ! -s "$OUT/warnings/arch_mul.log"
    ASM_OBJ="$OUT/obj/arch_mul.o"
fi

$AR rcsD "$OUT/libraf_fractal.a" \
    "$OUT/obj/projective.o" "$OUT/obj/fibonacci.o" \
    "$OUT/obj/mandelbrot.o" "$OUT/obj/core.o" $ASM_OBJ

$LD --gc-sections --fatal-warnings --build-id=none -z noexecstack \
    -T "$ROOT/linker/raf_fractal.ld" \
    -o "$OUT/raf_fractal_core.debug.elf" \
    "$OUT/obj/projective.o" "$OUT/obj/fibonacci.o" \
    "$OUT/obj/mandelbrot.o" "$OUT/obj/core.o" $ASM_OBJ

if [ -n "$($NM -u "$OUT/raf_fractal_core.debug.elf")" ]; then
    echo "undefined symbols detected:" >&2
    $NM -u "$OUT/raf_fractal_core.debug.elf" >&2
    exit 1
fi

$NM --defined-only --size-sort "$OUT/raf_fractal_core.debug.elf" > "$OUT/symbols.txt"
$OBJCOPY --strip-all "$OUT/raf_fractal_core.debug.elf" "$OUT/raf_fractal_core.elf"
$OBJCOPY -O binary "$OUT/raf_fractal_core.elf" "$OUT/raf_fractal_core.bin"
$SIZE "$OUT/raf_fractal_core.elf" > "$OUT/size.txt"

# Host-only proof harness; libc is used by the test, never by the core image.
if [ "$TARGET_ARCH" = host ]; then
    $CC -std=c11 -O2 -Wall -Wextra -Werror -I"$ROOT/include" \
        -DRAF_USE_ARCH_MUL="$USE_ASM" \
        "$ROOT/tests/test_host.c" \
        "$ROOT/src/raf_projective.c" "$ROOT/src/raf_fibonacci_arc.c" \
        "$ROOT/src/raf_mandelbrot.c" "$ROOT/src/raf_fractal_core.c" \
        $ASM_OBJ -o "$OUT/test_host"
    "$OUT/test_host" > "$OUT/test_host.txt"
fi

printf 'core ELF: '; cat "$OUT/size.txt"
printf 'raw bytes: '; wc -c < "$OUT/raf_fractal_core.bin"
printf 'undefined: 0\n'
