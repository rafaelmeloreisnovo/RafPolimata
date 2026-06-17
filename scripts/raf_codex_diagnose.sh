#!/usr/bin/env bash
# S38 — Modo Codex para corrigir compilação
# Diagnostica erros de compilação em todos os RAF_0xx_*.c e produz
# saída estruturada adequada para input de agente de IA.
#
# Uso:
#   bash scripts/raf_codex_diagnose.sh            # todos os RAF_0xx_*.c
#   bash scripts/raf_codex_diagnose.sh RAF_036_*.c  # arquivo específico
#   bash scripts/raf_codex_diagnose.sh --json     # saída em JSON por arquivo

set -euo pipefail

JSON=0
[[ "${1:-}" == "--json" ]] && JSON=1 && shift

if [ $# -gt 0 ]; then
    FILES=("$@")
else
    FILES=($(ls RAF_0[0-9][0-9]_*.c 2>/dev/null | sort || true))
fi

if [ ${#FILES[@]} -eq 0 ]; then
    echo "No RAF_0xx_*.c files found. Run from repository root."
    exit 0
fi

CLEAN=0; WARN=0; ERROR=0

if [ "${JSON}" -eq 1 ]; then
    echo "["
    FIRST=1
fi

for src in "${FILES[@]}"; do
    output=$(gcc -std=c11 -O2 -Wall -Wextra -I. -c "${src}" -o /dev/null 2>&1 || true)
    errors=$(echo "${output}" | grep -c ": error:" || true)
    warnings=$(echo "${output}" | grep -c ": warning:" || true)

    if [ "${JSON}" -eq 1 ]; then
        [ "${FIRST}" -eq 0 ] && echo ","
        FIRST=0
        echo -n "  {\"file\": \"${src}\", \"errors\": ${errors}, \"warnings\": ${warnings}"
        if [ "${errors}" -gt 0 ] || [ "${warnings}" -gt 0 ]; then
            diag=$(echo "${output}" | grep -E ": (error|warning):" | \
                   sed 's/"/\\"/g' | awk '{printf "%s\\n", $0}')
            echo -n ", \"diagnostics\": \"${diag}\""
        fi
        echo -n "}"
    else
        if [ "${errors}" -eq 0 ] && [ "${warnings}" -eq 0 ]; then
            echo "OK     ${src}"
            CLEAN=$((CLEAN + 1))
        elif [ "${errors}" -eq 0 ]; then
            echo "WARN   ${src}  (${warnings} warnings)"
            echo "${output}" | grep ": warning:" | sed 's/^/         /'
            WARN=$((WARN + 1))
        else
            echo "ERROR  ${src}  (${errors} errors, ${warnings} warnings)"
            echo "${output}" | grep -E ": (error|warning):" | sed 's/^/         /'
            ERROR=$((ERROR + 1))
        fi
    fi
done

if [ "${JSON}" -eq 1 ]; then
    echo ""
    echo "]"
else
    echo ""
    echo "Diagnosis: ${CLEAN} clean  ${WARN} warnings-only  ${ERROR} errors"
    if [ "${ERROR}" -gt 0 ]; then
        echo "See docs/CODEX_FIX_PROTOCOL.md for categorized fix patterns."
        exit 1
    fi
fi
