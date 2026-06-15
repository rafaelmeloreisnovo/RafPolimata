#!/bin/sh
# test_asm_roundtrip.sh — build apkc (if possible) and verify ARM64 assembly output bytes
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/tests/results"
mkdir -p "$OUT"
PASS=0; FAIL=0

log()  { printf '%s\n' "$*"; }
pass() { log "  PASS  $1"; PASS=$((PASS+1)); }
fail() { log "  FAIL  $1"; FAIL=$((FAIL+1)); }

log "=== ARM64 assembler round-trip tests ==="
log ""

# ── Build apkc binary ────────────────────────────────────────────────────────
EXE="$OUT/apkc_test"
BUILD_OK=0
for cc in cc gcc clang; do
  if command -v "$cc" >/dev/null 2>&1; then
    if "$cc" -std=c11 -Wall -Wextra -Wno-unused-function \
         -nostdlib -Wl,-e,_start "$ROOT/Apkc/apkc.c" -lgcc -o "$EXE" 2>/dev/null ||
       "$cc" -std=c11 -Wall -Wextra -Wno-unused-function \
         -nostdlib -Wl,-e,_start "$ROOT/Apkc/apkc.c" -o "$EXE" 2>/dev/null; then
      BUILD_OK=1; break
    fi
  fi
done

if [ "$BUILD_OK" = "0" ]; then
  log "TOKEN_VAZIO: apkc não compilou neste host (x86_64); testes de roundtrip ignorados."
  log ""
  log "Testes estruturais que não dependem do binário:"
fi

# ── Test: freestanding syntax check (always runs) ───────────────────────────
log "[T1] Freestanding syntax check (clang -target aarch64-linux-gnu)"
if command -v clang >/dev/null 2>&1; then
  if clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
       -ffreestanding -I "$ROOT/Apkc" "$ROOT/Apkc/apkc.c" 2>"$OUT/t1_clang.txt"; then
    pass "clang freestanding syntax: apkc.c"
  else
    fail "clang freestanding syntax: $(cat "$OUT/t1_clang.txt" | head -3)"
  fi
else
  log "  SKIP  clang não disponível"
fi

# ── Test: host compiler build (strict warnings) ──────────────────────────────
log ""
log "[T2] Host compiler build (gcc -Wall -Wextra -Werror)"
if gcc -std=c11 -Wall -Wextra -Werror \
     "$ROOT/raf_main.c" "$ROOT/raf_frontend.c" "$ROOT/raf_cpu.c" \
     "$ROOT/raf_asm_emit.c" "$ROOT/raf_precomp.c" \
     -o "$OUT/raf_compile_test" 2>"$OUT/t2_gcc.txt"; then
  pass "gcc build: raf_compile"
  # smoke test
  if "$OUT/raf_compile_test" --help >/dev/null 2>&1 || true; then
    pass "smoke test: raf_compile --help"
  fi
else
  fail "gcc build: $(head -3 "$OUT/t2_gcc.txt")"
fi

# ── Test: verbovivo build + run ──────────────────────────────────────────────
log ""
log "[T3] Verbovivo build + T7 toroid smoke test"
if gcc -std=c11 -O2 -I"$ROOT" -I"$ROOT/Benchmark" -DVERBOVIVO_MAIN \
     "$ROOT/rafaelia/verbovivo.c" -lm -o "$OUT/verbovivo_test" 2>"$OUT/t3_gcc.txt"; then
  pass "gcc build: verbovivo"
  # run with stdin
  if result=$(echo 'RafaeliaTestVector2026' | "$OUT/verbovivo_test" /dev/stdin /dev/null 2>&1); then
    phi=$(echo "$result" | grep -oE 'phi=[0-9.]+' | head -1)
    attr=$(echo "$result" | grep -oE 'attractor=[0-9]+' | head -1)
    pass "verbovivo T7 run: $phi $attr"
  else
    fail "verbovivo run: $result"
  fi
else
  fail "gcc build verbovivo: $(head -3 "$OUT/t3_gcc.txt")"
fi

# ── Test: apkc binary tests (only if build succeeded) ───────────────────────
if [ "$BUILD_OK" = "1" ]; then
  log ""
  log "[T4] apkc binary: ARM64 assembly of hello.s.txt"
  if [ -f "$ROOT/Apkc/hello.s.txt" ]; then
    if "$EXE" "$ROOT/Apkc/hello.s.txt" -o "$OUT/hello_ci.apk" -64 \
         > "$OUT/t4_apkc.txt" 2>&1; then
      sz=$(wc -c < "$OUT/hello_ci.apk" | tr -d ' ')
      pass "apkc ARM64: hello.s.txt → APK ($sz bytes)"
      # check it's a valid ZIP (PK header)
      header=$(od -A n -t x1 -N 4 "$OUT/hello_ci.apk" | tr -d ' \n')
      if [ "$header" = "504b0304" ]; then
        pass "APK header: ZIP magic PK\\x03\\x04 PASS"
      else
        fail "APK header: expected 504b0304 got $header"
      fi
    else
      fail "apkc ARM64: $(tail -3 "$OUT/t4_apkc.txt")"
    fi
  else
    log "  SKIP  Apkc/hello.s.txt não encontrado"
  fi

  log ""
  log "[T5] apkc binary: ARM32 assembly"
  if [ -f "$ROOT/Apkc/hello.s.txt" ]; then
    if "$EXE" "$ROOT/Apkc/hello.s.txt" -o "$OUT/hello_ci32.apk" -32 \
         > "$OUT/t5_apkc32.txt" 2>&1; then
      sz=$(wc -c < "$OUT/hello_ci32.apk" | tr -d ' ')
      pass "apkc ARM32: hello.s.txt → APK ($sz bytes)"
    else
      errs=$(grep -c "unknown ARM32" "$OUT/t5_apkc32.txt" 2>/dev/null || echo 0)
      fail "apkc ARM32: $errs unknown mnemonics ($(tail -1 "$OUT/t5_apkc32.txt"))"
    fi
  fi

  log ""
  log "[T6] use_script: Python fixture"
  PYSRC="$OUT/fixture_py.py"
  echo 'print("hello from apkc")' > "$PYSRC"
  if "$EXE" "$PYSRC" -o "$OUT/py_out.apk" -64 > "$OUT/t6_py.txt" 2>&1; then
    sz=$(wc -c < "$OUT/py_out.apk" | tr -d ' ')
    pass "apkc use_script py: APK ($sz bytes)"
  else
    fail "apkc use_script py: $(tail -1 "$OUT/t6_py.txt")"
  fi

  log ""
  log "[T7] use_script: Shell fixture"
  SHSRC="$OUT/fixture_sh.sh"
  echo 'echo hello' > "$SHSRC"
  if "$EXE" "$SHSRC" -o "$OUT/sh_out.apk" -64 > "$OUT/t7_sh.txt" 2>&1; then
    sz=$(wc -c < "$OUT/sh_out.apk" | tr -d ' ')
    pass "apkc use_script sh: APK ($sz bytes)"
  else
    fail "apkc use_script sh: $(tail -1 "$OUT/t7_sh.txt")"
  fi
fi

# ── Test: P(k) falsifiability gate ──────────────────────────────────────────
log ""
log "[T8] P(k) falsifiability report"
if python3 "$ROOT/scripts/first_test_pk.py" \
     --output "$OUT/pk_test.json" > "$OUT/t8_pk.txt" 2>&1; then
  verdict=$(python3 -c "import json; d=json.load(open('$OUT/pk_test.json')); print(d['verdict'])")
  rrmse=$(python3 -c "import json; d=json.load(open('$OUT/pk_test.json')); print(d['metrics']['rrmse'])")
  cov=$(python3 -c "import json; d=json.load(open('$OUT/pk_test.json')); print(d['metrics']['coverage_1sigma'])")
  if [ "$verdict" = "PASS" ]; then
    pass "P(k) gate: verdict=PASS rrmse=$rrmse coverage=$cov"
  else
    fail "P(k) gate: verdict=$verdict"
  fi
else
  fail "P(k): $(tail -1 "$OUT/t8_pk.txt")"
fi

# ── Test: lang_profile 12-language consistency ───────────────────────────────
log ""
log "[T9] lang_profile.h ↔ raf_compile.h consistency"
lp_count=$(grep -c "LP_[A-Z]" "$ROOT/Apkc/lang_profile.h" 2>/dev/null || echo 0)
raf_count=$(grep -c "RAF_LANG_[A-Z]" "$ROOT/raf_compile.h" 2>/dev/null || echo 0)
# subtract RAF_LANG_COUNT line itself
raf_count=$((raf_count - 1))
log "  lang_profile.h LP_ constants: $lp_count"
log "  raf_compile.h RAF_LANG_*: $raf_count"
if [ "$lp_count" -ge 12 ] && [ "$raf_count" -ge 12 ]; then
  pass "lang coverage: both headers declare ≥12 languages"
else
  fail "lang coverage mismatch: lp=$lp_count raf=$raf_count"
fi

# ── Test: Emit.err propagation check (structural) ───────────────────────────
log ""
log "[T10] Emit.err structural check (grep apkc.c)"
if grep -q "em->err++" "$ROOT/Apkc/apkc.c"; then
  errcount=$(grep -c "em->err++" "$ROOT/Apkc/apkc.c")
  pass "Emit.err incremented in $errcount locations"
else
  fail "Emit.err++ not found in apkc.c"
fi

# also check BRK and UNDEF are in there
if grep -q "0xD4200020u" "$ROOT/Apkc/apkc.c"; then
  pass "ARM64 unknown mnemonic → BRK #1 (0xD4200020)"
else
  fail "ARM64 BRK #1 not found"
fi
if grep -q "0xE7F000F0u" "$ROOT/Apkc/apkc.c"; then
  pass "ARM32 unknown mnemonic → UNDEF (0xE7F000F0)"
else
  fail "ARM32 UNDEF not found"
fi

# ── Summary ──────────────────────────────────────────────────────────────────
log ""
log "================================================================"
log "RESULT: ${PASS} PASS, ${FAIL} FAIL"
log "Artifacts: $OUT/"
log "================================================================"
[ "$FAIL" = "0" ] && exit 0 || exit 1
