#!/bin/sh
set -eu
# Uso: bash scripts/ci_freestanding_audit.sh
# Audita a rota ARM/AArch64 freestanding efetivamente ativa. Rotas hosted
# x86/x86_64 podem declarar libc sob guards próprios; isso não relaxa o target.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
REPORT="$ROOT/ci/reports/freestanding-audit.md"
mkdir -p "$ROOT/ci/reports"
TMP_PRE=$(mktemp)
TMP_ERR=$(mktemp)
TMP_HITS=$(mktemp)
trap 'rm -f "$TMP_PRE" "$TMP_ERR" "$TMP_HITS"' EXIT HUP INT TERM

CLANG_BIN=${CLANG:-clang}

{
  echo '# Freestanding audit — ApkC target projection'
  echo
  echo "- Data UTC: $(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  echo "- Commit: $(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo '- Escopo: AArch64 freestanding ativo; branches hosted x86 são fora do target.'
  echo
  echo '| Gate | Status | Observação |'
  echo '|---|---|---|'
} > "$REPORT"

fail_gate(){
  name=$1
  note=$2
  echo "| $name | FAIL | $note |" >> "$REPORT"
  echo >> "$REPORT"
  echo 'Resultado: FAIL — contrato freestanding do target violado.' >> "$REPORT"
  cat "$REPORT" >&2
  exit 1
}

if ! command -v "$CLANG_BIN" >/dev/null 2>&1; then
  fail_gate target_toolchain "clang indisponível: $CLANG_BIN"
fi

# 1) Projeta apenas o código ativo para AArch64. -nostdinc é deliberado:
# qualquer include hosted/libc que escapar dos guards do target torna o gate
# imediatamente inválido em vez de ser mascarado por uma busca textual.
if ! "$CLANG_BIN" -target aarch64-linux-gnu -E -P -nostdinc -ffreestanding \
     -I "$ROOT/Apkc" "$ROOT/Apkc/apkc.c" > "$TMP_PRE" 2> "$TMP_ERR"; then
  sed 's/^/- /' "$TMP_ERR" >> "$REPORT"
  fail_gate target_preprocess 'projeção AArch64 com -nostdinc falhou'
fi
echo '| target_preprocess | PASS | AArch64 ativo preprocessa com -nostdinc; includes hosted ficaram fora do target |' >> "$REPORT"

# 2) Valida sintaxe do mesmo target com builtins/stdlib desligados.
if ! "$CLANG_BIN" -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
     -ffreestanding -fno-builtin -I "$ROOT/Apkc" "$ROOT/Apkc/apkc.c" \
     > /dev/null 2> "$TMP_ERR"; then
  sed 's/^/- /' "$TMP_ERR" >> "$REPORT"
  fail_gate target_syntax 'sintaxe AArch64 freestanding falhou'
fi
echo '| target_syntax | PASS | clang -fsyntax-only -nostdlib -nostdinc -ffreestanding |' >> "$REPORT"

# 3) Audita heap no código já projetado. Macros/branches hosted removidos pelo
# preprocessor não geram falso positivo; chamada ativa de heap continua FAIL.
if grep -En '\b(malloc|calloc|realloc|free)[[:space:]]*\(' "$TMP_PRE" > "$TMP_HITS"; then
  echo '| heap_calls_active_target | FAIL | chamada de heap ativa no target AArch64 |' >> "$REPORT"
  sed 's/^/- /' "$TMP_HITS" >> "$REPORT"
  echo >> "$REPORT"
  echo 'Resultado: FAIL — heap ativo encontrado no target freestanding.' >> "$REPORT"
  cat "$REPORT" >&2
  exit 1
fi
echo '| heap_calls_active_target | PASS | nenhuma chamada de heap no código AArch64 projetado |' >> "$REPORT"

echo >> "$REPORT"
echo 'Resultado: PASS — rota AArch64 ativa permanece freestanding; hosted x86 não altera este claim.' >> "$REPORT"
cat "$REPORT"
