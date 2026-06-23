#!/usr/bin/env bash
# S11 — Medir antes de otimizar
# Compila todos os RAF_0xx_*.c e registra métricas de baseline:
#   - tempo de compilação (nanosegundos via date +%s%N)
#   - tamanho do segmento .text no objeto resultante
#
# Estas métricas capturam o estado ANTES de qualquer otimização de código,
# permitindo comparação posterior (antes vs depois).
#
# Saída: ci/reports/baseline_measurements.txt
#
# Uso:
#   bash scripts/raf_baseline_measure.sh              # todos os RAF_0xx_*.c
#   bash scripts/raf_baseline_measure.sh RAF_009_*.c  # arquivo específico
#   RAF_OPT=-O0 bash scripts/raf_baseline_measure.sh  # medir com -O0 como baseline sem otimização

set -euo pipefail

OPT="${RAF_OPT:--O2}"
OUTFILE="ci/reports/baseline_measurements.txt"
TMPOBJ=$(mktemp -d)
trap 'rm -rf "${TMPOBJ}"' EXIT

mkdir -p ci/reports

if [ $# -gt 0 ]; then
    SOURCES=("$@")
else
    SOURCES=($(ls RAF_0[0-9][0-9]_*.c 2>/dev/null | sort || true))
fi

if [ ${#SOURCES[@]} -eq 0 ]; then
    echo "No RAF_0xx_*.c files found. Run from repository root."
    exit 1
fi

{
    echo "# RAF Baseline Measurements — Compile Time + Code Size"
    echo "# Purpose: capture state BEFORE optimization passes (S11)"
    echo "# Generated: $(date -u '+%Y-%m-%dT%H:%M:%SZ')"
    echo "# Host: $(uname -m) $(uname -s)"
    echo "# Compiler: $(gcc --version | head -1)"
    echo "# CFLAGS: -std=c11 ${OPT} -Wall -I."
    echo "#"
    echo "# Format: status | method | compile_ns | text_bytes"
    echo "#"
} > "${OUTFILE}"

PASS=0; FAIL=0

for src in "${SOURCES[@]}"; do
    base=$(basename "${src}" .c)
    obj="${TMPOBJ}/${base}.o"

    t0=$(date +%s%N)
    if gcc -std=c11 ${OPT} -Wall -I. -c "${src}" -o "${obj}" 2>/dev/null; then
        t1=$(date +%s%N)
        compile_ns=$((t1 - t0))

        text_hex=$(objdump -h "${obj}" 2>/dev/null \
                   | awk '/\.text/{print $3; exit}')
        text_bytes=$((16#${text_hex:-0}))

        echo "PASS | ${base} | ${compile_ns} | ${text_bytes}" >> "${OUTFILE}"
        printf "PASS  %-55s  compile=%6dns  .text=%4dB\n" \
               "${base}" "${compile_ns}" "${text_bytes}"
        PASS=$((PASS + 1))
    else
        echo "FAIL | ${base} | - | -" >> "${OUTFILE}"
        printf "FAIL  %s\n" "${base}"
        FAIL=$((FAIL + 1))
    fi
done

{
    echo "#"
    echo "# Summary: PASS=${PASS} FAIL=${FAIL}"
    echo "# Compare against a post-optimization run to measure improvement."
} >> "${OUTFILE}"

echo ""
echo "Baseline: ${PASS} PASS  ${FAIL} FAIL  →  ${OUTFILE}"
[ ${FAIL} -eq 0 ]
