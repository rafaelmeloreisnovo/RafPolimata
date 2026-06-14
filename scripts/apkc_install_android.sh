#!/bin/sh
set -eu
# Uso: bash scripts/apkc_install_android.sh
# Instala APK assinado em device Android real via adb e captura logcat mínimo.
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/Apkc/proofs/out"
APK="$OUT/hello-signed.apk"
INSTALL="$OUT/adb-install.txt"
LOGCAT="$OUT/logcat-nativeactivity.txt"
mkdir -p "$OUT"
if ! command -v adb >/dev/null 2>&1; then echo 'TOKEN_VAZIO: adb ausente; instale Android platform-tools.' > "$INSTALL"; echo 'TOKEN_VAZIO: adb ausente; logcat indisponível.' > "$LOGCAT"; exit 0; fi
if [ ! -f "$APK" ]; then echo 'TOKEN_VAZIO: hello-signed.apk ausente; rode bash scripts/apkc_sign_debug.sh.' > "$INSTALL"; echo 'TOKEN_VAZIO: APK assinado ausente.' > "$LOGCAT"; exit 0; fi
adb devices > "$INSTALL" 2>&1
if ! awk 'NR>1 && $2=="device" {found=1} END{exit found?0:1}' "$INSTALL"; then echo 'TOKEN_VAZIO: nenhum device adb conectado/autorizado.' >> "$INSTALL"; echo 'TOKEN_VAZIO: nenhum device adb conectado/autorizado.' > "$LOGCAT"; exit 0; fi
adb install -r "$APK" >> "$INSTALL" 2>&1
adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|apkc|fatal|crash' > "$LOGCAT" 2>&1 || true
