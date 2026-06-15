#!/bin/sh
set -eu
# Uso: bash scripts/apkc_install_android.sh
# Instala APK assinado em device Android real via adb e captura package visibility + logcat.
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/Apkc/proofs/out"
APK="$OUT/hello-signed.apk"
INSTALL="$OUT/adb-install.txt"
LOGCAT="$OUT/logcat-nativeactivity.txt"
LAUNCH="$OUT/adb-launch.txt"
PKG="com.rafael.teste"
mkdir -p "$OUT"
{
  echo "# adb install/package proof"
  echo "date_utc=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  echo "repo_commit=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "apk=$APK"
  if command -v sha256sum >/dev/null 2>&1 && [ -f "$APK" ]; then sha256sum "$APK"; fi
} > "$INSTALL"
if ! command -v adb >/dev/null 2>&1; then echo 'TOKEN_VAZIO: adb ausente; instale Android platform-tools.' >> "$INSTALL"; echo 'TOKEN_VAZIO: adb ausente; logcat indisponivel.' > "$LOGCAT"; exit 0; fi
if [ ! -f "$APK" ]; then echo 'TOKEN_VAZIO: hello-signed.apk ausente; rode bash scripts/apkc_sign_debug.sh.' >> "$INSTALL"; echo 'TOKEN_VAZIO: APK assinado ausente.' > "$LOGCAT"; exit 0; fi
adb devices >> "$INSTALL" 2>&1
if ! adb devices | awk 'NR>1 && $2=="device" {found=1} END{exit found?0:1}'; then echo 'TOKEN_VAZIO: nenhum device adb conectado/autorizado.' >> "$INSTALL"; echo 'TOKEN_VAZIO: nenhum device adb conectado/autorizado.' > "$LOGCAT"; exit 0; fi
adb install -r "$APK" >> "$INSTALL" 2>&1
adb shell pm list packages "$PKG" >> "$INSTALL" 2>&1 || true
adb logcat -c >/dev/null 2>&1 || true
adb shell monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 > "$LAUNCH" 2>&1 || true
sleep 2
adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|apkc|fatal|crash' > "$LOGCAT" 2>&1 || true
