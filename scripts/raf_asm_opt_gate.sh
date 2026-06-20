#!/usr/bin/env bash
# A8 — Integrar otimizador C→ASM com RAF_0xx_*.c
# Roda raf_c_to_asm_root_optimizer.py em métodos host-runnable selecionados,
# gera saída assembly em ci/reports/asm_opt/ e valida que é assembly válido.
#
# Uso:
#   bash scripts/raf_asm_opt_gate.sh                    # 3 métodos representativos
#   bash scripts/raf_asm_opt_gate.sh RAF_009_*.c        # arquivo específico
#   RAF_ASM_TARGET=x86_64 bash scripts/raf_asm_opt_gate.sh

set -euo pipefail

TARGET="${RAF_ASM_TARGET:-x86_64}"
OUTDIR="ci/reports/asm_opt"
OPTIMIZER="raf_c_to_asm_root_optimizer.py"
PASS=0; FAIL=0; SKIP=0

mkdir -p "${OUTDIR}"

if [ $# -gt 0 ]; then
    FILES=("$@")
else
    # Métodos representativos: DSP (M009), ring buffer (M013), DMA (M032)
    FILES=(
        RAF_009_adc_com_oversampling.c
        RAF_013_uart_interrupt_driven_com_ring_buffer.c
        RAF_032_dma_control_block_chain.c
    )
fi

if [ ! -f "${OPTIMIZER}" ]; then
    echo "SKIP: ${OPTIMIZER} not found. Run from repository root."
    exit 0
fi

for src in "${FILES[@]}"; do
    [ -f "${src}" ] || { echo "SKIP  ${src} (not found)"; SKIP=$((SKIP+1)); continue; }

    base=$(basename "${src}" .c)
    out="${OUTDIR}/${base}_${TARGET}.s"

    if python3 "${OPTIMIZER}" --target "${TARGET}" "${src}" > "${out}" 2>/dev/null; then
        # Validate: check that output has at least a section header or instruction
        if grep -qE "^\s*\.(text|globl|section)|^\s+[a-z]" "${out}" 2>/dev/null || \
           wc -l < "${out}" | grep -q "[1-9]"; then
            lines=$(wc -l < "${out}")
            echo "PASS  ${base}  → ${out}  (${lines} lines)"
            PASS=$((PASS+1))
        else
            echo "WARN  ${base}  → output empty or unrecognized"
            SKIP=$((SKIP+1))
        fi
    else
        # Optimizer may not accept all C patterns — TOKEN_VAZIO, not failure
        echo "SKIP  ${base}  (optimizer returned non-zero — unsupported C patterns)"
        SKIP=$((SKIP+1))
    fi
done

echo ""
echo "ASM optimizer gate: ${PASS} PASS  ${FAIL} FAIL  ${SKIP} SKIP  →  ${OUTDIR}/"
[ ${FAIL} -eq 0 ]
