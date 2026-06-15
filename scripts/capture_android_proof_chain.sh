#!/usr/bin/env bash
# =============================================================================
# capture_android_proof_chain.sh
#
# Captura uma rodada rastreavel source -> apkc -> APK -> install -> launch -> logcat.
# Nao inventa PASS: quando uma ferramenta/etapa falta, registra TOKEN_VAZIO.
#
# Uso recomendado no Termux/host com repo clonado:
#   bash scripts/capture_android_proof_chain.sh
#
# Variaveis opcionais:
#   SRC=Apkc/hello.s.txt
#   OUT_DIR=proofs/run-arm64-full-chain/out
#   PKG=com.rafael.teste
#   LABEL=RafaelTeste
#   LIB=hello
#   APK_NAME=hello-both.apk
#   APKC_CC="cc"
#   APKC_CFLAGS="-std=c11 -Oz -Wno-unused-function -nostartfiles -Wl,-e,_start"
#   APKC_ARCH="-both"     # -64, -32 ou -both
#   DO_INSTALL=1          # 1 tenta adb/pm install; 0 apenas gera artefatos
#   DO_SIGN=auto          # auto|1|0 tenta apksigner quando existir
#
# RAFAELIA invariant:
#   codigo != execucao
#   fala != prova
#   ausencia de evidencia = TOKEN_VAZIO
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
APK_RAW="$OUT_DIR/$APK_NAME"
APK_SIGNED="$OUT_DIR/${APK_NAME%.apk}-signed.apk"
MANIFEST="$OUT_DIR/manifest.json"
STATUS="$OUT_DIR/status.tsv"

: > "$STATUS"

log_status() {
  # $1 gate, $2 status, $3 file, $4 note
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
  # $1 output_file, remaining command
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
  echo "shell=$SHELL"
  echo "termux_prefix=${PREFIX:-TOKEN_VAZIO}"
  echo "git_commit=$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "git_branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "git_status_porcelain_begin"
  git status --porcelain 2>/dev/null || true
  echo "git_status_porcelain_end"
  echo "cc_path=$(command -v "$APKC_CC" 2>/dev/null || echo TOKEN_VAZIO)"
  "$APKC_CC" --version 2>/dev/null || true
  echo "adb_path=$(command -v adb 2>/dev/null || echo TOKEN_VAZIO)"
  echo "apksigner_path=$(command -v apksigner 2>/dev/null || echo TOKEN_VAZIO)"
  echo "readelf_path=$(command -v readelf 2>/dev/null || echo TOKEN_VAZIO)"
  echo "unzip_path=$(command -v unzip 2>/dev/null || echo TOKEN_VAZIO)"
  echo "getprop_begin"
  getprop 2>/dev/null | grep -E '^\[(ro\.(product|build|system)|dalvik\.vm|ro\.hardware|ro\.arch)' || true
  echo "getprop_end"
} > "$OUT_DIR/00_env.txt" 2>&1
log_status "env" "AUDIT" "00_env.txt" "environment captured best-effort"

# 01 — compilar apkc
if [ ! -f "Apkc/apkc.c" ]; then
  write_token_vazio "$OUT_DIR/01_compile_apkc.txt" "Apkc/apkc.c ausente"
  log_status "compile_apkc" "TOKEN_VAZIO" "01_compile_apkc.txt" "source missing"
else
  # shellcheck disable=SC2086
  run_capture "$OUT_DIR/01_compile_apkc.txt" "$APKC_CC" $APKC_CFLAGS Apkc/apkc.c -o "$APKC_BIN"
  if [ -x "$APKC_BIN" ]; then
    sha256_one "$APKC_BIN" > "$OUT_DIR/01_apkc.sha256"
    log_status "compile_apkc" "PASS" "01_compile_apkc.txt" "apkc binary created"
  else
    log_status "compile_apkc" "FAIL" "01_compile_apkc.txt" "apkc binary not executable"
  fi
fi

# 02 — gerar APK
if [ ! -x "$APKC_BIN" ]; then
  write_token_vazio "$OUT_DIR/02_generate_apk.txt" "apkc nao compilou; geracao bloqueada"
  log_status "generate_apk" "TOKEN_VAZIO" "02_generate_apk.txt" "compile failed or missing"
elif [ ! -f "$SRC" ]; then
  write_token_vazio "$OUT_DIR/02_generate_apk.txt" "source de entrada ausente: $SRC"
  log_status "generate_apk" "TOKEN_VAZIO" "02_generate_apk.txt" "input source missing"
else
  run_capture "$OUT_DIR/02_generate_apk.txt" "$APKC_BIN" "$SRC" -o "$APK_RAW" -p "$PKG" -l "$LABEL" -n "$LIB" "$APKC_ARCH"
  if [ -f "$APK_RAW" ]; then
    sha256_one "$APK_RAW" > "$OUT_DIR/02_apk.sha256"
    log_status "generate_apk" "PASS" "02_generate_apk.txt" "APK generated"
  else
    log_status "generate_apk" "FAIL" "02_generate_apk.txt" "APK missing after command"
  fi
fi

# 03 — listar ZIP
if [ -f "$APK_RAW" ] && command -v unzip >/dev/null 2>&1; then
  run_capture "$OUT_DIR/03_unzip_list.txt" unzip -l "$APK_RAW"
  log_status "unzip_list" "PASS" "03_unzip_list.txt" "APK entries listed"
elif [ -f "$APK_RAW" ]; then
  write_token_vazio "$OUT_DIR/03_unzip_list.txt" "unzip ausente"
  log_status "unzip_list" "TOKEN_VAZIO" "03_unzip_list.txt" "unzip missing"
else
  write_token_vazio "$OUT_DIR/03_unzip_list.txt" "APK ausente"
  log_status "unzip_list" "TOKEN_VAZIO" "03_unzip_list.txt" "APK missing"
fi

# 04 — extrair/readelf ARM32 e ARM64
if [ -f "$APK_RAW" ] && command -v unzip >/dev/null 2>&1; then
  unzip -p "$APK_RAW" "lib/armeabi-v7a/lib${LIB}.so" > "$OUT_DIR/lib-${LIB}-arm32.so" 2>/dev/null || true
  unzip -p "$APK_RAW" "lib/arm64-v8a/lib${LIB}.so" > "$OUT_DIR/lib-${LIB}-arm64.so" 2>/dev/null || true
else
  : > "$OUT_DIR/lib-${LIB}-arm32.so"
  : > "$OUT_DIR/lib-${LIB}-arm64.so"
fi

if [ -s "$OUT_DIR/lib-${LIB}-arm32.so" ] && command -v readelf >/dev/null 2>&1; then
  run_capture "$OUT_DIR/04_readelf_arm32.txt" readelf -h "$OUT_DIR/lib-${LIB}-arm32.so"
  log_status "readelf_arm32" "PASS" "04_readelf_arm32.txt" "ARM32 ELF inspected"
else
  write_token_vazio "$OUT_DIR/04_readelf_arm32.txt" "lib/armeabi-v7a/lib${LIB}.so ausente ou readelf ausente"
  log_status "readelf_arm32" "TOKEN_VAZIO" "04_readelf_arm32.txt" "ARM32 ELF/readelf missing"
fi

if [ -s "$OUT_DIR/lib-${LIB}-arm64.so" ] && command -v readelf >/dev/null 2>&1; then
  run_capture "$OUT_DIR/05_readelf_arm64.txt" readelf -h "$OUT_DIR/lib-${LIB}-arm64.so"
  log_status "readelf_arm64" "PASS" "05_readelf_arm64.txt" "ARM64 ELF inspected"
else
  write_token_vazio "$OUT_DIR/05_readelf_arm64.txt" "lib/arm64-v8a/lib${LIB}.so ausente ou readelf ausente"
  log_status "readelf_arm64" "TOKEN_VAZIO" "05_readelf_arm64.txt" "ARM64 ELF/readelf missing"
fi

# 06 — assinatura opcional
APK_TO_INSTALL="$APK_RAW"
if [ -f "$APK_RAW" ]; then
  if { [ "$DO_SIGN" = "1" ] || [ "$DO_SIGN" = "auto" ]; } && command -v apksigner >/dev/null 2>&1; then
    # Se nao houver keystore configurado, apksigner falhara e o log vira evidencia.
    if [ -n "${APKSIGNER_KEYSTORE:-}" ]; then
      run_capture "$OUT_DIR/06_apksigner.txt" apksigner sign --ks "$APKSIGNER_KEYSTORE" --out "$APK_SIGNED" "$APK_RAW"
      if [ -f "$APK_SIGNED" ]; then
        APK_TO_INSTALL="$APK_SIGNED"
        sha256_one "$APK_SIGNED" > "$OUT_DIR/06_signed_apk.sha256"
        run_capture "$OUT_DIR/06_apksigner_verify.txt" apksigner verify --verbose "$APK_SIGNED"
        log_status "apksigner" "PASS" "06_apksigner.txt" "APK signed with provided keystore"
      else
        log_status "apksigner" "FAIL" "06_apksigner.txt" "sign requested but output missing"
      fi
    else
      run_capture "$OUT_DIR/06_apksigner_verify.txt" apksigner verify --verbose "$APK_RAW"
      log_status "apksigner" "AUDIT" "06_apksigner_verify.txt" "verified raw APK or captured failure; no APKSIGNER_KEYSTORE"
    fi
  else
    write_token_vazio "$OUT_DIR/06_apksigner.txt" "apksigner ausente ou DO_SIGN=0"
    log_status "apksigner" "TOKEN_VAZIO" "06_apksigner.txt" "signing not available"
  fi
else
  write_token_vazio "$OUT_DIR/06_apksigner.txt" "APK ausente"
  log_status "apksigner" "TOKEN_VAZIO" "06_apksigner.txt" "APK missing"
fi

# 07 — install / launch / logcat
if [ "$DO_INSTALL" = "1" ] && [ -f "$APK_TO_INSTALL" ]; then
  if command -v adb >/dev/null 2>&1; then
    run_capture "$OUT_DIR/07_adb_devices.txt" adb devices -l
    run_capture "$OUT_DIR/08_adb_install.txt" adb install -r "$APK_TO_INSTALL"
    run_capture "$OUT_DIR/09_launch.txt" adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1
    run_capture "$OUT_DIR/10_logcat_nativeactivity.txt" sh -c "adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true"
    log_status "android_runtime" "AUDIT" "08_adb_install.txt,09_launch.txt,10_logcat_nativeactivity.txt" "adb path executed; inspect logs for PASS/FAIL"
  elif command -v pm >/dev/null 2>&1; then
    # Modo Termux no proprio aparelho sem adb. Pode exigir permissao/limites do Android.
    run_capture "$OUT_DIR/08_pm_install.txt" pm install -r "$APK_TO_INSTALL"
    run_capture "$OUT_DIR/09_launch.txt" monkey -p "$PKG" -c android.intent.category.LAUNCHER 1
    run_capture "$OUT_DIR/10_logcat_nativeactivity.txt" sh -c "logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|fatal|crash|$PKG|lib${LIB}' || true"
    log_status "android_runtime" "AUDIT" "08_pm_install.txt,09_launch.txt,10_logcat_nativeactivity.txt" "pm/logcat path executed; inspect logs for PASS/FAIL"
  else
    write_token_vazio "$OUT_DIR/08_install.txt" "adb/pm ausentes"
    write_token_vazio "$OUT_DIR/09_launch.txt" "adb/monkey ausentes"
    write_token_vazio "$OUT_DIR/10_logcat_nativeactivity.txt" "adb/logcat ausentes"
    log_status "android_runtime" "TOKEN_VAZIO" "08_install.txt" "no adb or pm"
  fi
else
  write_token_vazio "$OUT_DIR/08_install.txt" "DO_INSTALL=0 ou APK ausente"
  write_token_vazio "$OUT_DIR/09_launch.txt" "DO_INSTALL=0 ou APK ausente"
  write_token_vazio "$OUT_DIR/10_logcat_nativeactivity.txt" "DO_INSTALL=0 ou APK ausente"
  log_status "android_runtime" "TOKEN_VAZIO" "08_install.txt" "install disabled or APK missing"
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
  "schema": "rafpolimata.android_proof_chain.v1",
  "date_utc": "$(date -u +%FT%TZ 2>/dev/null || true)",
  "git_commit": "$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)",
  "git_branch": "$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo TOKEN_VAZIO)",
  "source": "$SRC",
  "package": "$PKG",
  "label": "$LABEL",
  "lib": "$LIB",
  "apkc_arch": "$APKC_ARCH",
  "out_dir": "$OUT_DIR",
  "apkc_sha256": "$APKC_SHA",
  "apk_sha256": "$APK_SHA",
  "signed_apk_sha256": "$SIGNED_SHA",
  "status_file": "status.tsv",
  "rule": "TOKEN_VAZIO is preserved for missing evidence; inspect logs before promoting PASS"
}
JSON
log_status "manifest" "PASS" "manifest.json" "manifest emitted"

cat <<EOF
[RAFPOLIMATA PROOF CHAIN]
out_dir=$OUT_DIR
status=$STATUS
manifest=$MANIFEST

Leia primeiro:
  cat $STATUS
  cat $MANIFEST

Promocao para PASS exige inspecionar logs, nao apenas existencia dos arquivos.
EOF
