#!/usr/bin/env bash
# =============================================================================
# capture_android_proof_chain.sh
#
# Captura uma rodada rastreavel:
# raw ApkC -> runtime-hardened ApkC TU -> apkc ELF -> input source -> APK ->
# signature -> install -> launch -> logcat.
#
# Nao inventa PASS. Saida stale e exit code falho nunca podem promover gate.
# Runtime/device paths compile only a runtime-hardened generated TU; raw
# Apkc/apkc.c is never passed directly to the compiler by this script.
# =============================================================================

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit 2

OUT_DIR="${OUT_DIR:-proofs/run-arm64-full-chain/out}"
SRC="${SRC:-Apkc/hello.s.txt}"
PKG="${PKG:-com.rafael.teste}"
LABEL="${LABEL:-RafaelTeste}"
LIB="${LIB:-hello}"
APK_NAME="${APK_NAME:-hello-both.apk}"
APKC_CC="${APKC_CC:-cc}"
APKC_CFLAGS="${APKC_CFLAGS:--std=c11 -Oz -Wno-unused-function -nostartfiles -Wl,-e,_start}"
APKC_ARCH="${APKC_ARCH:--both}"
DO_INSTALL="${DO_INSTALL:-1}"
DO_SIGN="${DO_SIGN:-auto}"

mkdir -p "$OUT_DIR"

APKC_BIN="$OUT_DIR/apkc"
APKC_HARD_SRC="$OUT_DIR/apkc-runtime-hardened.c"
APKC_HARD_MANIFEST="$OUT_DIR/01_runtime_hardening_manifest.json"
APKC_HARD_LOG="$OUT_DIR/01_runtime_hardening.txt"
APK_RAW="$OUT_DIR/$APK_NAME"
APK_SIGNED="$OUT_DIR/${APK_NAME%.apk}-signed.apk"
MANIFEST="$OUT_DIR/manifest.json"
STATUS="$OUT_DIR/status.tsv"

: > "$STATUS"

log_status() {
  printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "$4" >> "$STATUS"
}

write_token_vazio() {
  local file="$1" msg="$2"
  {
    echo "TOKEN_VAZIO: $msg"
    echo "date_utc=$(date -u +%FT%TZ 2>/dev/null || true)"
  } > "$file"
}

run_capture() {
  local out="$1"; shift
  {
    echo "# command: $*"
    echo "# cwd: $(pwd)"
    echo "# date_utc: $(date -u +%FT%TZ 2>/dev/null || true)"
    echo "# ---- stdout/stderr ----"
    "$@"
  } > "$out" 2>&1
}

sha256_one() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$1" | awk '{print $1}'
  else
    echo "TOKEN_VAZIO_SHA256_TOOL_MISSING"
  fi
}

# 00 — ambiente
{
  echo "# env"
  echo "date_utc=$(date -u +%FT%TZ 2>/dev/null || true)"
  echo "pwd=$(pwd)"
  echo "user=$(id 2>/dev/null || true)"
  echo "uname=$(uname -a 2>/dev/null || true)"
  echo "shell=${SHELL:-TOKEN_VAZIO}"
  echo "termux_prefix=${PREFIX:-TOKEN_VAZIO}"
  echo "git_commit=$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "git_branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "git_status_porcelain_begin"
  git status --porcelain 2>/dev/null || true
  echo "git_status_porcelain_end"
  echo "cc_path=$(command -v "$APKC_CC" 2>/dev/null || echo TOKEN_VAZIO)"
  "$APKC_CC" --version 2>/dev/null || true
  echo "python3_path=$(command -v python3 2>/dev/null || echo TOKEN_VAZIO)"
  echo "adb_path=$(command -v adb 2>/dev/null || echo TOKEN_VAZIO)"
  echo "apksigner_path=$(command -v apksigner 2>/dev/null || echo TOKEN_VAZIO)"
  echo "readelf_path=$(command -v readelf 2>/dev/null || echo TOKEN_VAZIO)"
  echo "unzip_path=$(command -v unzip 2>/dev/null || echo TOKEN_VAZIO)"
  echo "getprop_begin"
  getprop 2>/dev/null | grep -E '^\[(ro\.(product|build|system)|dalvik\.vm|ro\.hardware|ro\.arch)' || true
  echo "getprop_end"
} > "$OUT_DIR/00_env.txt" 2>&1
log_status "env" "AUDIT" "00_env.txt" "environment captured best-effort"

# 01A — gerar e validar TU runtime-hardened antes de qualquer compilacao
HARD_OK=0
RAW_APKC_SHA="TOKEN_VAZIO"
HARD_APKC_SHA="TOKEN_VAZIO"
rm -f "$APKC_HARD_SRC" "$APKC_HARD_MANIFEST" "$APKC_HARD_LOG" "$APKC_BIN"

if [ ! -f "Apkc/apkc.c" ]; then
  write_token_vazio "$APKC_HARD_LOG" "Apkc/apkc.c ausente"
  log_status "runtime_hardening" "TOKEN_VAZIO" "01_runtime_hardening.txt" "raw source missing"
elif ! command -v python3 >/dev/null 2>&1; then
  write_token_vazio "$APKC_HARD_LOG" "python3 ausente; recusando compilar fonte raw"
  log_status "runtime_hardening" "TOKEN_VAZIO" "01_runtime_hardening.txt" "python3 missing; compile blocked"
elif [ ! -f scripts/patch_apkc_runtime_source.py ]; then
  write_token_vazio "$APKC_HARD_LOG" "patch_apkc_runtime_source.py ausente"
  log_status "runtime_hardening" "TOKEN_VAZIO" "01_runtime_hardening.txt" "transformer missing; compile blocked"
else
  if run_capture "$APKC_HARD_LOG" python3 scripts/patch_apkc_runtime_source.py \
       --input Apkc/apkc.c --output "$APKC_HARD_SRC" --write-manifest "$APKC_HARD_MANIFEST" \
     && [ -s "$APKC_HARD_SRC" ] \
     && [ -s "$APKC_HARD_MANIFEST" ] \
     && grep -q 'source exceeds 1MiB bounded input' "$APKC_HARD_SRC" \
     && grep -q 'APKC-RH-013' "$APKC_HARD_MANIFEST" \
     && ! grep -Fq 'if (n<=0) break;' "$APKC_HARD_SRC"; then
    RAW_APKC_SHA="$(sha256_one Apkc/apkc.c)"
    HARD_APKC_SHA="$(sha256_one "$APKC_HARD_SRC")"
    {
      echo "raw_apkc_sha256=$RAW_APKC_SHA"
      echo "runtime_hardened_apkc_sha256=$HARD_APKC_SHA"
      echo "runtime_manifest=$APKC_HARD_MANIFEST"
      echo "bounded_source_change=APKC-RH-013"
    } >> "$APKC_HARD_LOG"
    HARD_OK=1
    log_status "runtime_hardening" "PASS" "01_runtime_hardening.txt" "runtime-hardened TU + APC-RH-013 verified"
  else
    log_status "runtime_hardening" "FAIL" "01_runtime_hardening.txt" "transform/manifest/marker gate failed; compile blocked"
  fi
fi

# 01B — compilar apkc somente do TU hardened
if [ "$HARD_OK" -ne 1 ]; then
  write_token_vazio "$OUT_DIR/01_compile_apkc.txt" "runtime hardening nao estabelecido; compilacao bloqueada"
  log_status "compile_apkc" "TOKEN_VAZIO" "01_compile_apkc.txt" "blocked by runtime_hardening"
else
  rm -f "$APKC_BIN"
  # shellcheck disable=SC2086
  if run_capture "$OUT_DIR/01_compile_apkc.txt" "$APKC_CC" $APKC_CFLAGS -I Apkc "$APKC_HARD_SRC" -o "$APKC_BIN" \
     && [ -x "$APKC_BIN" ]; then
    sha256_one "$APKC_BIN" > "$OUT_DIR/01_apkc.sha256"
    log_status "compile_apkc" "PASS" "01_compile_apkc.txt" "runtime-hardened apkc binary created"
  else
    rm -f "$APKC_BIN"
    log_status "compile_apkc" "FAIL" "01_compile_apkc.txt" "compiler failed or executable missing"
  fi
fi

# 02 — gerar APK; stale output nunca promove PASS
SOURCE_SHA="TOKEN_VAZIO"
[ -f "$SRC" ] && SOURCE_SHA="$(sha256_one "$SRC")"
rm -f "$APK_RAW"
if [ ! -x "$APKC_BIN" ]; then
  write_token_vazio "$OUT_DIR/02_generate_apk.txt" "apkc hardened nao compilou; geracao bloqueada"
  log_status "generate_apk" "TOKEN_VAZIO" "02_generate_apk.txt" "compile failed or missing"
elif [ ! -f "$SRC" ]; then
  write_token_vazio "$OUT_DIR/02_generate_apk.txt" "source de entrada ausente: $SRC"
  log_status "generate_apk" "TOKEN_VAZIO" "02_generate_apk.txt" "input source missing"
else
  if run_capture "$OUT_DIR/02_generate_apk.txt" "$APKC_BIN" "$SRC" -o "$APK_RAW" -p "$PKG" -l "$LABEL" -n "$LIB" "$APKC_ARCH" \
     && [ -s "$APK_RAW" ]; then
    sha256_one "$APK_RAW" > "$OUT_DIR/02_apk.sha256"
    log_status "generate_apk" "PASS" "02_generate_apk.txt" "APK generated from hardened apkc; exit=0; non-empty"
  else
    rm -f "$APK_RAW"
    log_status "generate_apk" "FAIL" "02_generate_apk.txt" "command failed or APK missing/empty"
  fi
fi

# 03 — listar ZIP com exit code real
if [ -f "$APK_RAW" ] && command -v unzip >/dev/null 2>&1; then
  if run_capture "$OUT_DIR/03_unzip_list.txt" unzip -l "$APK_RAW"; then
    log_status "unzip_list" "PASS" "03_unzip_list.txt" "APK entries listed; exit=0"
  else
    log_status "unzip_list" "FAIL" "03_unzip_list.txt" "unzip returned non-zero"
  fi
elif [ -f "$APK_RAW" ]; then
  write_token_vazio "$OUT_DIR/03_unzip_list.txt" "unzip ausente"
  log_status "unzip_list" "TOKEN_VAZIO" "03_unzip_list.txt" "unzip missing"
else
  write_token_vazio "$OUT_DIR/03_unzip_list.txt" "APK ausente"
  log_status "unzip_list" "TOKEN_VAZIO" "03_unzip_list.txt" "APK missing"
fi

# 04/05 — extrair/readelf ARM32 e ARM64
if [ -f "$APK_RAW" ] && command -v unzip >/dev/null 2>&1; then
  unzip -p "$APK_RAW" "lib/armeabi-v7a/lib${LIB}.so" > "$OUT_DIR/lib-${LIB}-arm32.so" 2>/dev/null || true
  unzip -p "$APK_RAW" "lib/arm64-v8a/lib${LIB}.so" > "$OUT_DIR/lib-${LIB}-arm64.so" 2>/dev/null || true
else
  : > "$OUT_DIR/lib-${LIB}-arm32.so"
  : > "$OUT_DIR/lib-${LIB}-arm64.so"
fi

if [ -s "$OUT_DIR/lib-${LIB}-arm32.so" ] && command -v readelf >/dev/null 2>&1; then
  if run_capture "$OUT_DIR/04_readelf_arm32.txt" readelf -h "$OUT_DIR/lib-${LIB}-arm32.so"; then
    log_status "readelf_arm32" "PASS" "04_readelf_arm32.txt" "ARM32 ELF inspected; exit=0"
  else
    log_status "readelf_arm32" "FAIL" "04_readelf_arm32.txt" "readelf returned non-zero"
  fi
else
  write_token_vazio "$OUT_DIR/04_readelf_arm32.txt" "lib/armeabi-v7a/lib${LIB}.so ausente ou readelf ausente"
  log_status "readelf_arm32" "TOKEN_VAZIO" "04_readelf_arm32.txt" "ARM32 ELF/readelf missing"
fi

if [ -s "$OUT_DIR/lib-${LIB}-arm64.so" ] && command -v readelf >/dev/null 2>&1; then
  if run_capture "$OUT_DIR/05_readelf_arm64.txt" readelf -h "$OUT_DIR/lib-${LIB}-arm64.so"; then
    log_status "readelf_arm64" "PASS" "05_readelf_arm64.txt" "ARM64 ELF inspected; exit=0"
  else
    log_status "readelf_arm64" "FAIL" "05_readelf_arm64.txt" "readelf returned non-zero"
  fi
else
  write_token_vazio "$OUT_DIR/05_readelf_arm64.txt" "lib/arm64-v8a/lib${LIB}.so ausente ou readelf ausente"
  log_status "readelf_arm64" "TOKEN_VAZIO" "05_readelf_arm64.txt" "ARM64 ELF/readelf missing"
fi

# 06 — assinatura opcional; verify e gate separado
APK_TO_INSTALL="$APK_RAW"
rm -f "$APK_SIGNED"
if [ -f "$APK_RAW" ]; then
  if { [ "$DO_SIGN" = "1" ] || [ "$DO_SIGN" = "auto" ]; } && command -v apksigner >/dev/null 2>&1; then
    if [ -n "${APKSIGNER_KEYSTORE:-}" ]; then
      if run_capture "$OUT_DIR/06_apksigner.txt" apksigner sign --ks "$APKSIGNER_KEYSTORE" --out "$APK_SIGNED" "$APK_RAW" \
         && [ -s "$APK_SIGNED" ]; then
        if run_capture "$OUT_DIR/06_apksigner_verify.txt" apksigner verify --verbose "$APK_SIGNED"; then
          APK_TO_INSTALL="$APK_SIGNED"
          sha256_one "$APK_SIGNED" > "$OUT_DIR/06_signed_apk.sha256"
          log_status "apksigner" "PASS" "06_apksigner.txt,06_apksigner_verify.txt" "sign+verify exit=0"
        else
          rm -f "$APK_SIGNED"
          APK_TO_INSTALL="$APK_RAW"
          log_status "apksigner" "FAIL" "06_apksigner_verify.txt" "signed output failed verification"
        fi
      else
        rm -f "$APK_SIGNED"
        log_status "apksigner" "FAIL" "06_apksigner.txt" "sign command failed or output missing"
      fi
    else
      if run_capture "$OUT_DIR/06_apksigner_verify.txt" apksigner verify --verbose "$APK_RAW"; then
        log_status "apksigner" "AUDIT" "06_apksigner_verify.txt" "raw APK already verifies; no signing custody asserted"
      else
        log_status "apksigner" "AUDIT" "06_apksigner_verify.txt" "raw APK signature verification failed; no APKSIGNER_KEYSTORE"
      fi
    fi
  else
    write_token_vazio "$OUT_DIR/06_apksigner.txt" "apksigner ausente ou DO_SIGN=0"
    log_status "apksigner" "TOKEN_VAZIO" "06_apksigner.txt" "signing not available"
  fi
else
  write_token_vazio "$OUT_DIR/06_apksigner.txt" "APK ausente"
  log_status "apksigner" "TOKEN_VAZIO" "06_apksigner.txt" "APK missing"
fi

# 07 — install / launch / logcat; install/launch get explicit exit gates.
if [ "$DO_INSTALL" = "1" ] && [ -f "$APK_TO_INSTALL" ]; then
  if command -v adb >/dev/null 2>&1; then
    run_capture "$OUT_DIR/07_adb_devices.txt" adb devices -l || true
    if run_capture "$OUT_DIR/08_adb_install.txt" adb install -r "$APK_TO_INSTALL"; then
      log_status "android_install" "PASS" "08_adb_install.txt" "adb install exit=0"
      if run_capture "$OUT_DIR/09_launch.txt" adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1; then
        log_status "android_launch" "PASS" "09_launch.txt" "launch command exit=0; app health still requires logcat"
      else
        log_status "android_launch" "FAIL" "09_launch.txt" "launch command non-zero"
      fi
    else
      log_status "android_install" "FAIL" "08_adb_install.txt" "adb install non-zero"
      write_token_vazio "$OUT_DIR/09_launch.txt" "install falhou; launch bloqueado"
      log_status "android_launch" "TOKEN_VAZIO" "09_launch.txt" "blocked by install failure"
    fi
    run_capture "$OUT_DIR/10_logcat_nativeactivity.txt" sh -c "adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true" || true
    log_status "android_runtime" "AUDIT" "10_logcat_nativeactivity.txt" "logcat captured; no crash-free PASS without semantic inspection"
  elif command -v pm >/dev/null 2>&1; then
    if run_capture "$OUT_DIR/08_pm_install.txt" pm install -r "$APK_TO_INSTALL"; then
      log_status "android_install" "PASS" "08_pm_install.txt" "pm install exit=0"
      if run_capture "$OUT_DIR/09_launch.txt" monkey -p "$PKG" -c android.intent.category.LAUNCHER 1; then
        log_status "android_launch" "PASS" "09_launch.txt" "launch command exit=0; app health still requires logcat"
      else
        log_status "android_launch" "FAIL" "09_launch.txt" "launch command non-zero"
      fi
    else
      log_status "android_install" "FAIL" "08_pm_install.txt" "pm install non-zero"
      write_token_vazio "$OUT_DIR/09_launch.txt" "install falhou; launch bloqueado"
      log_status "android_launch" "TOKEN_VAZIO" "09_launch.txt" "blocked by install failure"
    fi
    run_capture "$OUT_DIR/10_logcat_nativeactivity.txt" sh -c "logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true" || true
    log_status "android_runtime" "AUDIT" "10_logcat_nativeactivity.txt" "logcat captured; no crash-free PASS without semantic inspection"
  else
    write_token_vazio "$OUT_DIR/08_install.txt" "adb/pm ausentes"
    write_token_vazio "$OUT_DIR/09_launch.txt" "adb/monkey ausentes"
    write_token_vazio "$OUT_DIR/10_logcat_nativeactivity.txt" "adb/logcat ausentes"
    log_status "android_install" "TOKEN_VAZIO" "08_install.txt" "no adb or pm"
    log_status "android_launch" "TOKEN_VAZIO" "09_launch.txt" "no adb or pm"
    log_status "android_runtime" "TOKEN_VAZIO" "10_logcat_nativeactivity.txt" "no adb/pm/logcat"
  fi
else
  write_token_vazio "$OUT_DIR/08_install.txt" "DO_INSTALL=0 ou APK ausente"
  write_token_vazio "$OUT_DIR/09_launch.txt" "DO_INSTALL=0 ou APK ausente"
  write_token_vazio "$OUT_DIR/10_logcat_nativeactivity.txt" "DO_INSTALL=0 ou APK ausente"
  log_status "android_install" "TOKEN_VAZIO" "08_install.txt" "install disabled or APK missing"
  log_status "android_launch" "TOKEN_VAZIO" "09_launch.txt" "install disabled or APK missing"
  log_status "android_runtime" "TOKEN_VAZIO" "10_logcat_nativeactivity.txt" "install disabled or APK missing"
fi

# 11 — resumo JSON best-effort
APKC_SHA="TOKEN_VAZIO"
APK_SHA="TOKEN_VAZIO"
SIGNED_SHA="TOKEN_VAZIO"
[ -f "$OUT_DIR/01_apkc.sha256" ] && APKC_SHA="$(cat "$OUT_DIR/01_apkc.sha256")"
[ -f "$OUT_DIR/02_apk.sha256" ] && APK_SHA="$(cat "$OUT_DIR/02_apk.sha256")"
[ -f "$OUT_DIR/06_signed_apk.sha256" ] && SIGNED_SHA="$(cat "$OUT_DIR/06_signed_apk.sha256")"

cat > "$MANIFEST" <<JSON
{
  "schema": "rafpolimata.android_proof_chain.v2",
  "date_utc": "$(date -u +%FT%TZ 2>/dev/null || true)",
  "git_commit": "$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)",
  "git_branch": "$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo TOKEN_VAZIO)",
  "claim_allowed": false,
  "raw_apkc_source": "Apkc/apkc.c",
  "raw_apkc_sha256": "$RAW_APKC_SHA",
  "runtime_hardened_apkc_source": "$APKC_HARD_SRC",
  "runtime_hardened_apkc_sha256": "$HARD_APKC_SHA",
  "runtime_hardening_manifest": "$APKC_HARD_MANIFEST",
  "source": "$SRC",
  "source_sha256": "$SOURCE_SHA",
  "package": "$PKG",
  "label": "$LABEL",
  "lib": "$LIB",
  "apkc_arch": "$APKC_ARCH",
  "out_dir": "$OUT_DIR",
  "apkc_sha256": "$APKC_SHA",
  "apk_sha256": "$APK_SHA",
  "signed_apk_sha256": "$SIGNED_SHA",
  "status_file": "status.tsv",
  "rule": "TOKEN_VAZIO is preserved for missing evidence; runtime PASS requires semantic log evidence, not file existence"
}
JSON
log_status "manifest" "PASS" "manifest.json" "custody manifest v2 emitted"

cat <<EOF
[RAFPOLIMATA HARDENED PROOF CHAIN]
out_dir=$OUT_DIR
status=$STATUS
manifest=$MANIFEST

Leia primeiro:
  cat $STATUS
  cat $MANIFEST

Promocao para PASS exige exit code, hash e evidencia apropriada ao claim.
EOF
