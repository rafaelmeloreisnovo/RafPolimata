#!/usr/bin/env bash
# S08 — Build MCU separado do NDK
# Compila todos os RAF_0xx_*.c para ATmega328P usando avr-gcc.
# Requer: avr-gcc, avr-objdump (pacote gcc-avr / avr-gcc no sistema)
#
# Uso:
#   bash Benchmark/build_mcu.sh                  # compila todos os M001-M020 para AVR
#   bash Benchmark/build_mcu.sh RAF_003_*.c      # compila arquivo específico
#
# Output: build_mcu/<file>.o (compile-only), no executable/firmware or physical evidence.

set -euo pipefail

MCU=atmega328p
F_CPU=16000000UL
CFLAGS="-mmcu=${MCU} -DF_CPU=${F_CPU} -DRAFAELIA_FORCE_AVR_DEMO=1 -Os -std=c11 -Wall -Wextra"
OUTDIR="build_mcu"
PASS=0
FAIL=0
SKIP=0

mkdir -p "${OUTDIR}"

# Target files: argument list or all AVR-relevant methods (M001-M020)
if [ $# -gt 0 ]; then
    FILES=("$@")
else
    FILES=($(ls RAF_0{01,02,03,04,05,06,07,08,09,10,11,12,13,14,15,16,17,18,19,20}_*.c 2>/dev/null || true))
fi

if [ ${#FILES[@]} -eq 0 ]; then
    echo "No MCU files found (RAF_001-020_*.c). Run from repository root."
    exit 1
fi

AVR_GCC=$(command -v avr-gcc 2>/dev/null || true)
if [ -z "${AVR_GCC}" ]; then
    echo "SKIP: avr-gcc not found — install gcc-avr package to build MCU targets"
    exit 2  # NOT_RUN_NO_COMPILER, never success
fi

if ! command -v avr-objdump >/dev/null 2>&1; then
    echo "NOT_RUN_NO_OBJDUMP: missing AVR disassembler"
    exit 2
fi

for src in "${FILES[@]}"; do
    base=$(basename "${src}" .c)
    obj="${OUTDIR}/${base}.o"

    if ${AVR_GCC} ${CFLAGS} -I. -c "${src}" -o "${obj}" 2>/dev/null; then
        size=$(avr-objdump -h "${obj}" 2>/dev/null \
               | awk '/\.text/{print $3}' | head -1)
        size_dec=$((16#${size:-0}))
        echo "COMPILE_ONLY  ${base}  .text=${size_dec}B (NOT_LINKED_NOT_FLASHED)"
        PASS=$((PASS + 1))
    else
        echo "FAIL  ${base}"
        FAIL=$((FAIL + 1))
    fi
done

echo ""
echo "MCU compile-only: ${PASS} COMPILE_OK  ${FAIL} FAIL  ${SKIP} SKIP; ELF/HEX/device=NOT_RUN"
[ ${FAIL} -eq 0 ]
