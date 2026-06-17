#!/usr/bin/env bash
# S21 — Teste de tamanho binário
# Compila cada RAF_0xx_*.c como objeto standalone e reporta tamanho .text em bytes.
# Falha se qualquer arquivo ultrapassar o limite configurável (padrão: 4096 bytes de .text).
# Saída: ci/reports/binary_size.md
set -euo pipefail

LIMIT_TEXT_BYTES=${RAF_SIZE_LIMIT:-4096}
OUT_DIR="${RAF_REPORT_DIR:-ci/reports}"
REPORT="$OUT_DIR/binary_size.md"
mkdir -p "$OUT_DIR"

{
  echo "# Relatório de tamanho binário — S21"
  echo ""
  echo "Data: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "Limite .text por arquivo: ${LIMIT_TEXT_BYTES} bytes"
  echo ""
  echo "| Arquivo | .text (bytes) | .rodata (bytes) | Total .o (bytes) | Estado |"
  echo "|---------|:---:|:---:|:---:|:---:|"
} > "$REPORT"

fail=0

for src in RAF_0*.c; do
  obj=$(mktemp /tmp/raf_size_XXXXXX.o)
  if ! gcc -std=c11 -O2 -Wall -Wextra \
       -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE \
       -c "$src" -o "$obj" 2>/dev/null; then
    echo "| $src | COMPILE_FAIL | — | — | FAIL |" >> "$REPORT"
    fail=1
    continue
  fi

  # Extract section sizes; use 0 if section absent
  text_bytes=$(objdump -h "$obj" 2>/dev/null \
    | awk '/\.text/ {printf "%d", strtonum("0x"$3)}' || echo 0)
  rodata_bytes=$(objdump -h "$obj" 2>/dev/null \
    | awk '/\.rodata/ {printf "%d", strtonum("0x"$3)}' || echo 0)
  total_bytes=$(stat -c%s "$obj" 2>/dev/null || echo 0)

  text_bytes=${text_bytes:-0}
  rodata_bytes=${rodata_bytes:-0}
  total_bytes=${total_bytes:-0}

  if [ "$text_bytes" -le "$LIMIT_TEXT_BYTES" ]; then
    state="PASS"
  else
    state="FAIL (>${LIMIT_TEXT_BYTES})"
    fail=1
  fi

  echo "| $src | $text_bytes | $rodata_bytes | $total_bytes | $state |" >> "$REPORT"
  rm -f "$obj"
done

echo "" >> "$REPORT"
if [ "$fail" -eq 0 ]; then
  echo "**Resultado: PASS — todos os arquivos dentro do limite.**" >> "$REPORT"
else
  echo "**Resultado: FAIL — um ou mais arquivos ultrapassaram o limite.**" >> "$REPORT"
fi

cat "$REPORT"
exit "$fail"
