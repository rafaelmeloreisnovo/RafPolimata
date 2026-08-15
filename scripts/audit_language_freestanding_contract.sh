#!/usr/bin/env bash
set -Eeuo pipefail

export LC_ALL=C
export TZ=UTC
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC_BIN="${CC:-cc}"
NM_BIN="${NM:-nm}"
REPORT="$ROOT/ci/reports/language-freestanding-contract.md"
OBJ="$(mktemp)"
CPU_OBJ="$(mktemp)"
BIN="$(mktemp)"
trap 'rm -f "$OBJ" "$CPU_OBJ" "$BIN"' EXIT

STRICT_FILES=(
  "$ROOT/Apkc/lang_freestanding_policy.h"
  "$ROOT/rafaelia/raf_lane16_patch.h"
  "$ROOT/RAF_063_language_completion_freestanding.c"
)
COMPILER_STATION_FILES=(
  "$ROOT/Apkc/raf_libc_emu.h"
  "$ROOT/raf_frontend.c"
  "$ROOT/raf_precomp.c"
  "$ROOT/raf_asm_emit.c"
  "$ROOT/scripts/raf_c_rewrite.py"
  "$ROOT/scripts/raf_kernel_lower.py"
  "$ROOT/scripts/apkc_strict_native_build.sh"
  "$ROOT/scripts/audit_strict_elf.sh"
  "$ROOT/scripts/validate_ops_manifest.py"
  "$ROOT/scripts/compare_ops_manifest.py"
  "$ROOT/scripts/test_compiler_station.sh"
  "$ROOT/scripts/validate_compiler_station_contract.py"
  "$ROOT/ci/contracts/apkc_compiler_station_v2.json"
)

mkdir -p "$ROOT/ci/reports"

fail() {
  printf 'M063 audit: FAIL — %s\n' "$*" >&2
  exit 1
}

for file in "${STRICT_FILES[@]}" "${COMPILER_STATION_FILES[@]}"; do
  [[ -s "$file" ]] || fail "arquivo ausente ou vazio: $file"
done

if grep -En '\b(malloc|calloc|realloc|free)[[:space:]]*\(' "${STRICT_FILES[@]}"; then
  fail 'heap explícito encontrado no núcleo M063 estrito'
fi
# Keep the hosted-header regex semantically identical while avoiding a literal
# Markdown-like `](` sequence in this shell source. The documentation governance
# scanner treats Markdown links as relations; this split prevents the shell regex
# itself from being misclassified as a broken local document reference.
HOSTED_HEADER_RE='^#[[:space:]]*include[[:space:]]*[<"]'
HOSTED_HEADER_RE+='(stdio|stdlib|string|unistd|pthread|dlfcn|execinfo|setjmp)\.h[>"]'
if grep -En "$HOSTED_HEADER_RE" "${STRICT_FILES[@]}"; then
  fail 'header hosted proibido encontrado no núcleo M063 estrito'
fi
if grep -En '\b(new|delete|throw|catch|dynamic_cast|typeid)[[:space:]]*(\(|\[|[A-Za-z_])' "${STRICT_FILES[@]}"; then
  fail 'runtime/abstração C++ proibida encontrada no núcleo M063 estrito'
fi

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-ident \
  -fno-unwind-tables -fno-asynchronous-unwind-tables \
  -fno-optimize-sibling-calls -fvisibility=hidden \
  -ffunction-sections -fdata-sections \
  -c "$ROOT/RAF_063_language_completion_freestanding.c" -o "$OBJ"

if "$NM_BIN" -u "$OBJ" | grep -q '[^[:space:]]'; then
  "$NM_BIN" -u "$OBJ" >&2 || true
  fail 'símbolo indefinido encontrado no objeto M063 estrito'
fi

"$CC_BIN" -std=c11 -O2 -Wall -Wextra -Werror -Wshadow -Wconversion -Wpedantic \
  -DRAFAELIA_M063_SELFTEST_MAIN \
  "$ROOT/RAF_063_language_completion_freestanding.c" -o "$BIN"
"$BIN"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror -c "$ROOT/raf_cpu.c" -o "$CPU_OBJ"
python3 "$ROOT/tests/test_capability_matrix.py"
python3 "$ROOT/scripts/raf_library_assimilation_audit.py" --selftest
python3 "$ROOT/scripts/raf_strict_compile_plan.py" --selftest
python3 "$ROOT/scripts/validate_compiler_station_contract.py"
bash "$ROOT/scripts/test_compiler_station.sh"

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
grep -Eq '^#define[[:space:]]+RAF_RECOGNIZED_LANG_COUNT[[:space:]]+23([[:space:]]|$)' \
  "$ROOT/raf_compile.h" || fail 'raf_compile.h não declara 23 perfis reconhecidos'

for name in asm c cpp rs kt java py sh pl js php jsx go rb swift groovy clj glsl cl hlsl wgsl dsp tflite; do
  grep -Fq "\"$name\"" "$ROOT/Apkc/lang_freestanding_policy.h" \
    || fail "linguagem sem política M063: $name"
done

{
  echo '# M063 + APKc — auditoria freestanding e HOTFIX operacional'
  echo
  echo "- Compiler host: $CC_BIN"
  echo '- Perfis reconhecidos: 23 + UNKNOWN'
  echo '- Rotas canônicas da estação: 4'
  echo '- Hosted RAF_KERNEL routes: 14'
  echo '- Heap no núcleo M063: PASS'
  echo '- Headers hosted no núcleo M063: PASS'
  echo '- Objeto M063 sem símbolos indefinidos: PASS'
  echo '- Selective bit patch/lane-16: PASS'
  echo '- Matriz de arquitetura e linguagem: PASS'
  echo '- Auditoria de biblioteca estrangeira: PASS'
  echo '- Inventário canônico APKc v2: PASS'
  echo '- Emulação C/C++ adversarial: PASS'
  echo '- Rewrite de headers fail-closed: PASS'
  echo '- RAF_KERNEL lowering delimitado: PASS'
  echo '- FNV-1a .ops schema 4: PASS'
  echo '- Recibo adulterado rejeitado: PASS'
  echo '- Rollback sem artefato stale: PASS'
  echo '- ARM64 + ARM32 + C++ export + hosted kernel: PASS'
  echo '- Perfil ELF Android sem dependência externa: PASS'
  echo '- Reprodutibilidade byte a byte: PASS'
  echo
  echo 'Resultado local: HOTFIX_PASS_LOCAL.'
  echo 'Promoção global: depende do status verde dos workflows do commit/PR.'
  echo 'Não promovido: APK_SIGNATURE / APK_INSTALL / ANDROID_RUNTIME / DEVICE_ACCELERATOR_RUNTIME.'
} > "$REPORT"

cat "$REPORT"
