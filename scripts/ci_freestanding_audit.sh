#!/bin/sh
set -eu
# Uso: bash scripts/ci_freestanding_audit.sh
# Audita invariantes freestanding do núcleo ApkC: sem heap explícito e sem includes libc proibidos.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
REPORT="$ROOT/ci/reports/freestanding-audit.md"
mkdir -p "$ROOT/ci/reports"
TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT HUP INT TERM

{
  echo '# Freestanding audit — ApkC'
  echo
  echo "- Data UTC: $(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  echo "- Commit: $(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo
  echo '| Gate | Status | Observação |'
  echo '|---|---|---|'
} > "$REPORT"

scan_pattern(){
  name=$1
  pattern=$2
  if rg -n --glob 'Apkc/*.[ch]' "$pattern" "$ROOT" > "$TMP"; then
    echo "| $name | FAIL | padrão proibido encontrado |" >> "$REPORT"
    sed 's/^/- /' "$TMP" >> "$REPORT"
    return 1
  fi
  echo "| $name | PASS | padrão ausente |" >> "$REPORT"
  return 0
}

FAIL=0
scan_pattern heap_calls '\b(malloc|calloc|realloc|free)\s*\(' || FAIL=1
scan_pattern libc_includes '^#\s*include\s*[<"](stdlib|stdio|string|stdint|unistd|fcntl)\.h[>"]' || FAIL=1

if [ "$FAIL" -ne 0 ]; then
  echo >> "$REPORT"
  echo 'Resultado: FAIL — violação freestanding encontrada.' >> "$REPORT"
  exit 1
fi

echo >> "$REPORT"
echo 'Resultado: PASS — núcleo ApkC sem heap explícito/import libc proibido nos padrões auditados.' >> "$REPORT"
