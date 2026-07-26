#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC_BIN="${CC:-cc}"
NM_BIN="${NM:-nm}"
REPORT="$ROOT/ci/reports/language-freestanding-contract.md"
OBJ="$(mktemp)"
BIN="$(mktemp)"
trap 'rm -f "$OBJ" "$BIN"' EXIT

STRICT_FILES=(
  "$ROOT/Apkc/lang_freestanding_policy.h"
  "$ROOT/rafaelia/raf_lane16_patch.h"
  "$ROOT/RAF_063_language_completion_freestanding.c"
)

mkdir -p "$ROOT/ci/reports"

fail() {
  printf 'M063 audit: FAIL — %s\n' "$*" >&2
  exit 1
}

for file in "${STRICT_FILES[@]}"; do
  [[ -s "$file" ]] || fail "arquivo ausente ou vazio: $file"
done

if grep -En '\b(malloc|calloc|realloc|free)[[:space:]]*\(' "${STRICT_FILES[@]}"; then
  fail 'heap explícito encontrado no núcleo estrito'
fi

if grep -En '^#[[:space:]]*include[[:space:]]*[<"](stdio|stdlib|string|unistd|pthread|dlfcn|execinfo|setjmp)\.h[>"]' "${STRICT_FILES[@]}"; then
  fail 'header hosted proibido encontrado no núcleo estrito'
fi

if grep -En '\b(new|delete|throw|catch|dynamic_cast|typeid)[[:space:]]*(\(|\[|[A-Za-z_])' "${STRICT_FILES[@]}"; then
  fail 'runtime/abstração C++ proibida encontrada no núcleo estrito'
fi

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-ident \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -fno-optimize-sibling-calls -fvisibility=hidden \
  -ffunction-sections -fdata-sections \
  -c "$ROOT/RAF_063_language_completion_freestanding.c" -o "$OBJ"

if "$NM_BIN" -u "$OBJ" | grep -q '[^[:space:]]'; then
  "$NM_BIN" -u "$OBJ" >&2 || true
  fail 'símbolo indefinido encontrado no objeto estrito'
fi

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic \
  -DRAFAELIA_M063_SELFTEST_MAIN \
  "$ROOT/RAF_063_language_completion_freestanding.c" -o "$BIN"
"$BIN"

TSV="$ROOT/ci/contracts/rafaelia_language_completion_v1.tsv"
[[ -s "$TSV" ]] || fail 'matriz TSV ausente'

awk -F '\t' '
  NR == 1 {
    if ($1 != "id" || $2 != "language" || $5 != "strict_route") exit 10
    next
  }
  {
    if ($1 != NR - 2) exit 11
    if ($2 == "" || seen[$2]++) exit 12
    rows++
  }
  END { if (rows != 23) exit 13 }
' "$TSV" || fail 'matriz TSV inválida, duplicada ou incompleta'

grep -Eq '^#define[[:space:]]+LP_COUNT[[:space:]]+23([[:space:]]|$)' \
  "$ROOT/Apkc/lang_profile.h" || fail 'LP_COUNT de lang_profile.h não é 23'

for name in asm c cpp rs kt java py sh pl js php jsx go rb swift groovy clj glsl cl hlsl wgsl dsp tflite; do
  grep -Fq "\"$name\"" "$ROOT/Apkc/lang_freestanding_policy.h" \
    || fail "linguagem sem política M063: $name"
done

{
  echo '# M063 — Language completion freestanding audit'
  echo
  echo "- Compiler: $CC_BIN"
  echo '- Strict source files: 3'
  echo '- Language policies: 23'
  echo '- Heap calls: PASS'
  echo '- Hosted headers: PASS'
  echo '- C++ runtime tokens: PASS'
  echo '- Strict compile warnings: PASS'
  echo '- Undefined symbols in strict object: PASS'
  echo '- Selective bit patch selftest: PASS'
  echo '- Static lane-16 dependency selftest: PASS'
  echo '- Policy/TSV coverage: PASS'
  echo
  echo 'Resultado: PASS_LOCAL. Final ELF/DEX/device binaries still require their own symbol, relocation and runtime gates.'
} > "$REPORT"

cat "$REPORT"
