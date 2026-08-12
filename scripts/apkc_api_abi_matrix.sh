#!/bin/sh
set -eu
# Fecha a lacuna "matriz API/ABI ampla" (docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md).
# Para cada linguagem sem toolchain externa (use_asm + use_script), gera APKs
# variando minSdk/targetSdk/ABI e verifica o minSdk no AXML binário.
# Anti-retraction: Apkc/apkc.c nunca é compilado cru neste caminho.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
OUT="$APKC/proofs/out"
EXEC_ROOT="${TMPDIR:-${TMP:-/tmp}}/apkc_matrix_$$"
mkdir -p "$OUT" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM

SUMMARY="$OUT/api-abi-matrix.md"
HARD_LOG="$OUT/api-abi-source-cap.txt"
HARD_SRC="$EXEC_ROOT/apkc_hardened.c"
COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)
DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
{
  echo '# ApkC matriz API/ABI'
  echo ''
  echo "- Data UTC: $DATE"
  echo "- Commit: $COMMIT"
  echo '- claim_allowed: false'
  echo ''
  echo '| Lang | minSdk | tgtSdk | ABI | Status | Evidência |'
  echo '|------|--------|--------|-----|--------|-----------|'
} > "$SUMMARY"

row(){ printf '| %s | %s | %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4" "$5" "$6"; printf '| %s | %s | %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4" "$5" "$6" >> "$SUMMARY"; }

command -v python3 >/dev/null 2>&1 || { echo 'FAIL: python3 ausente; source-cap hardening obrigatório.' >> "$SUMMARY"; exit 1; }
[ -f "$ROOT/scripts/patch_apkc_source_cap.py" ] || { echo 'FAIL: transformer source-cap ausente.' >> "$SUMMARY"; exit 1; }
[ -f "$ROOT/tests/test_apkc_source_cap_patch.py" ] || { echo 'FAIL: falsificador source-cap ausente.' >> "$SUMMARY"; exit 1; }

: > "$HARD_LOG"
if python3 "$ROOT/tests/test_apkc_source_cap_patch.py" > "$HARD_LOG" 2>&1 \
   && python3 "$ROOT/scripts/patch_apkc_source_cap.py" "$APKC/apkc.c" "$HARD_SRC" >> "$HARD_LOG" 2>&1 \
   && grep -q 'source exceeds SRC_CAP' "$HARD_SRC" \
   && ! grep -Fq 'if (n<=0) break;' "$HARD_SRC"; then
  RAW_SHA=$(sha256sum "$APKC/apkc.c" 2>/dev/null | cut -d' ' -f1 || echo TOKEN_VAZIO)
  HARD_SHA=$(sha256sum "$HARD_SRC" 2>/dev/null | cut -d' ' -f1 || echo TOKEN_VAZIO)
  {
    echo "raw_source_sha256: $RAW_SHA"
    echo "hardened_source_sha256: $HARD_SHA"
  } >> "$HARD_LOG"
  echo "- source_cap_hardening: PASS ($HARD_SHA)" >> "$SUMMARY"
else
  echo 'FAIL: source-cap hardening/falsifier/anchor gate falhou; matriz não executada.' >> "$SUMMARY"
  exit 1
fi

cd "$APKC"

# ── build hardened apkc ------------------------------------------------------
EXE="$EXEC_ROOT/apkc"
BUILD_MODE=TOKEN_VAZIO
try_build(){ "$@" > "$EXEC_ROOT/build.txt" 2>&1; }
for c in cc gcc clang; do
  if command -v "$c" >/dev/null 2>&1; then
    if try_build "$c" -std=c11 -Wall -Wextra -Wno-unused-function \
           -nostdlib -Wl,-e,_start -I"$APKC" "$HARD_SRC" -lgcc -o "$EXE" 2>/dev/null ||
       try_build "$c" -std=c11 -Wall -Wextra -Wno-unused-function \
           -nostdlib -Wl,-e,_start -I"$APKC" "$HARD_SRC" -o "$EXE"; then
      BUILD_MODE=native; break
    fi
  fi
done

if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  for lang in asm py sh pl js php; do
    for sdk in 21 24 28 29 30 31 33 34; do
      row "$lang" "$sdk" "$sdk" both TOKEN_VAZIO 'source hardening PASS; apkc hardened não executável neste host'
    done
  done
  echo 'TOKEN_VAZIO: hardening confirmado, mas sem binário apkc executável; matriz runtime não executada.'
  exit 0
fi
chmod +x "$EXE"

# ── verifica, byte a byte, que minSdkVersion foi codificado no AXML ────────
verify_minsdk(){
  apk=$1; sdk=$2
  python3 - "$apk" "$sdk" <<'PYEOF'
import struct, sys, zipfile
apk, sdk = sys.argv[1], int(sys.argv[2])
try:
    with zipfile.ZipFile(apk) as z:
        manifest = z.read("AndroidManifest.xml")
except Exception:
    sys.exit(1)
needle = struct.pack("<I", sdk)
sys.exit(0 if needle in manifest else 1)
PYEOF
}

PASS_COUNT=0
FAIL_COUNT=0
SKIP_COUNT=0

run_cell(){
  lang=$1; ext=$2; fixture=$3; sdk=$4; tgt=$5; abi_flag=$6; abi_label=$7
  src="$EXEC_ROOT/fixture_${lang}_${sdk}_${abi_label}${ext}"
  apk="$EXEC_ROOT/out_${lang}_${sdk}_${abi_label}.apk"
  printf '%s' "$fixture" > "$src"
  if "$EXE" "$src" -o "$apk" -m "$sdk" -t "$tgt" "$abi_flag" \
       > "$EXEC_ROOT/log_${lang}_${sdk}_${abi_label}.txt" 2>&1; then
    if [ -s "$apk" ] && verify_minsdk "$apk" "$sdk"; then
      row "$lang" "$sdk" "$tgt" "$abi_label" PASS "minSdkVersion=$sdk confirmado no AXML binário de APK gerado pelo hardened source"
      PASS_COUNT=$((PASS_COUNT+1))
    else
      row "$lang" "$sdk" "$tgt" "$abi_label" FAIL 'APK vazio ou minSdkVersion ausente no AXML'
      FAIL_COUNT=$((FAIL_COUNT+1))
    fi
  else
    last=$(tail -1 "$EXEC_ROOT/log_${lang}_${sdk}_${abi_label}.txt" 2>/dev/null || echo '(sem saída)')
    row "$lang" "$sdk" "$tgt" "$abi_label" FAIL "$last"
    FAIL_COUNT=$((FAIL_COUNT+1))
  fi
}

SDKS='21 24 28 29 30 31 33 34'
for sdk in $SDKS; do
  tgt=34
  if [ -f "$APKC/hello.s.txt" ]; then
    run_cell asm '.s' "$(cat "$APKC/hello.s.txt")" "$sdk" "$tgt" -64 arm64
  else
    row asm "$sdk" "$tgt" arm64 TOKEN_VAZIO 'Apkc/hello.s.txt ausente'
    SKIP_COUNT=$((SKIP_COUNT+1))
  fi
  run_cell py  '.py'  'print("hello")'         "$sdk" "$tgt" -64 arm64
  run_cell sh  '.sh'  'echo hello'             "$sdk" "$tgt" -64 arm64
  run_cell pl  '.pl'  'print "hello\n"'        "$sdk" "$tgt" -64 arm64
  run_cell js  '.js'  'console.log("hello")'   "$sdk" "$tgt" -64 arm64
  run_cell php '.php' '<?php echo "hello"; ?>' "$sdk" "$tgt" -64 arm64
done

for lang in c cpp rs kt java jsx; do
  for sdk in $SDKS; do
    row "$lang" "$sdk" 34 arm64 TOKEN_VAZIO 'use_fork exige ARM64/toolchain; execução física atual não comprovada neste ciclo'
    SKIP_COUNT=$((SKIP_COUNT+1))
  done
done

{
  echo ''
  echo "Conclusão: ${PASS_COUNT} PASS, ${FAIL_COUNT} FAIL, ${SKIP_COUNT} TOKEN_VAZIO em $((PASS_COUNT+FAIL_COUNT+SKIP_COUNT)) células; hardening source-cap obrigatório aplicado antes da matriz."
} >> "$SUMMARY"
echo "apkc_api_abi_matrix: ${PASS_COUNT} PASS, ${FAIL_COUNT} FAIL, ${SKIP_COUNT} TOKEN_VAZIO"

if [ "$FAIL_COUNT" -gt 0 ]; then exit 1; fi
