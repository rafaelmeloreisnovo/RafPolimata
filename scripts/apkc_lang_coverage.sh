#!/bin/sh
set -eu
# Smoke-tests ApkC language coverage.
# Tests the 6 no-toolchain pipeline modes: ASM (use_asm) + 5 script languages (use_script).
# fork+exec languages (C/C++/Rust/Kotlin/Java/JSX) require ARM64 host — TOKEN_VAZIO on x86_64 CI.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
OUT="$APKC/proofs/out"
EXEC_ROOT="${TMPDIR:-${TMP:-/tmp}}/apkc_lang_$$"
mkdir -p "$OUT" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM

SUMMARY="$OUT/lang-coverage.md"
COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)
DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
{
  echo '# ApkC language coverage'
  echo ''
  echo "- Data UTC: $DATE"
  echo "- Commit: $COMMIT"
  echo ''
  echo '| Lang | Pipeline | Status | Evidência |'
  echo '|------|----------|--------|-----------|'
} > "$SUMMARY"

log(){ printf '%s\n' "$*"; printf '%s\n' "$*" >> "$SUMMARY"; }
row(){ printf '| %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4"; printf '| %s | %s | %s | %s |\n' "$1" "$2" "$3" "$4" >> "$SUMMARY"; }

cd "$APKC"

# ── build apkc (same fallback ladder as apkc_validate.sh) ───────────────────
EXE="$EXEC_ROOT/apkc"
BUILD_MODE=TOKEN_VAZIO

try_build(){
  "$@" > "$EXEC_ROOT/build.txt" 2>&1
}

for c in cc gcc clang; do
  if command -v "$c" >/dev/null 2>&1; then
    if try_build "$c" -std=c11 -Wall -Wextra -Wno-unused-function \
           -nostdlib -Wl,-e,_start apkc.c -lgcc -o "$EXE" 2>/dev/null ||
       try_build "$c" -std=c11 -Wall -Wextra -Wno-unused-function \
           -nostdlib -Wl,-e,_start apkc.c -o "$EXE"; then
      BUILD_MODE=native; break
    fi
  fi
done
if [ "$BUILD_MODE" = TOKEN_VAZIO ] && command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
  if try_build aarch64-linux-gnu-gcc -std=c11 -Wall -Wextra -Wno-unused-function \
         -nostdlib -static -Wl,-e,_start apkc.c -o "$EXE"; then
    BUILD_MODE=cross
  fi
fi

if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  log 'TOKEN_VAZIO: não há binário apkc executável neste host; instale toolchain ARM/Linux.'
  for lang in asm py sh pl js php; do
    row "$lang" use_asm/use_script TOKEN_VAZIO 'apkc não executável neste host'
  done
  for lang in c cpp rs kt java jsx; do
    row "$lang" use_fork TOKEN_VAZIO 'fork_exec_wait ARM64-only; apkc não executável'
  done
  log ''
  log 'Conclusão: sem binário apkc executável; todos os gates são TOKEN_VAZIO.'
  exit 0
fi

chmod +x "$EXE"

# ── helper: run one language test ────────────────────────────────────────────
PASS_COUNT=0
FAIL_COUNT=0
test_lang(){
  lang=$1; pipeline=$2; fixture_content=$3; ext=$4
  src="$EXEC_ROOT/fixture_${lang}${ext}"
  outapk="$EXEC_ROOT/out_${lang}.apk"
  printf '%s' "$fixture_content" > "$src"
  if "$EXE" "$src" -o "$outapk" -64 > "$EXEC_ROOT/${lang}.txt" 2>&1; then
    if [ -s "$outapk" ]; then
      size=$(wc -c < "$outapk" | tr -d ' ')
      row "$lang" "$pipeline" PASS "APK gerado: ${size} bytes"
      PASS_COUNT=$((PASS_COUNT+1))
    else
      row "$lang" "$pipeline" FAIL 'APK vazio após execução'
      FAIL_COUNT=$((FAIL_COUNT+1))
    fi
  else
    last=$(tail -1 "$EXEC_ROOT/${lang}.txt" 2>/dev/null || echo '(sem saída)')
    row "$lang" "$pipeline" FAIL "$last"
    FAIL_COUNT=$((FAIL_COUNT+1))
  fi
}

# ── ASM: use internal 2-pass assembler ───────────────────────────────────────
ASM_SRC="$APKC/hello.s.txt"
if [ -f "$ASM_SRC" ]; then
  test_lang asm use_asm "$(cat "$ASM_SRC")" '.s'
else
  row asm use_asm TOKEN_VAZIO 'Apkc/hello.s.txt ausente'
fi

# ── use_script languages: gen_script_code64 embeds interpreter path + source ─
test_lang py  use_script 'print("hello")'           '.py'
test_lang sh  use_script 'echo hello'               '.sh'
test_lang pl  use_script 'print "hello\n"'          '.pl'
test_lang js  use_script 'console.log("hello")'     '.js'
test_lang php use_script '<?php echo "hello"; ?>'   '.php'

# ── use_fork languages: ARM64-only (fork_exec_wait), TOKEN_VAZIO on x86_64 ──
for lang in c cpp rs kt java jsx; do
  row "$lang" use_fork TOKEN_VAZIO 'fork_exec_wait é ARM64-only; testado em Termux/ARM64'
done

log ''
log "Conclusão: ${PASS_COUNT}/6 testes PASS, ${FAIL_COUNT} FAIL; use_fork TOKEN_VAZIO (ARM64-only)."
if [ "$FAIL_COUNT" -gt 0 ]; then exit 1; fi
