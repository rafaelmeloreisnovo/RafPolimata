#!/bin/sh
set -eu
# Uso: bash scripts/ci_freestanding_audit.sh
# Audita invariantes freestanding do núcleo ApkC: sem heap explícito e sem includes libc proibidos.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
REPORT="$ROOT/ci/reports/freestanding-audit.md"
mkdir -p "$ROOT/ci/reports"
TMP=$(mktemp)
TMP_FILTERED=$(mktemp)
trap 'rm -f "$TMP" "$TMP_FILTERED"' EXIT HUP INT TERM

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

  # GitHub-hosted runners do not guarantee ripgrep. Keep the audit portable:
  # prefer rg when available, otherwise use grep over the same first-level
  # ApkC C/header files. A missing search tool must never be interpreted as
  # "pattern absent".
  : > "$TMP"
  if command -v rg >/dev/null 2>&1; then
    rg -n --glob 'Apkc/*.[ch]' "$pattern" "$ROOT" > "$TMP" || true
  elif command -v grep >/dev/null 2>&1; then
    grep -En "$pattern" "$ROOT"/Apkc/*.[ch] > "$TMP" 2>/dev/null || true
  else
    echo "| $name | FAIL | nenhum mecanismo de busca disponível (rg/grep) |" >> "$REPORT"
    return 1
  fi

  # raf_libc_emu.h intentionally defines heap names as fail-closed macros.
  # They are enforcement sentinels, not allocations. Ignore only those
  # preprocessor definitions; any real invocation remains a failure.
  if [ "$name" = "heap_calls" ]; then
    grep -Ev '#[[:space:]]*define[[:space:]]+(malloc|calloc|realloc|free)[[:space:]]*\(' "$TMP" > "$TMP_FILTERED" || true
    cp "$TMP_FILTERED" "$TMP"
  fi

  if [ -s "$TMP" ]; then
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
