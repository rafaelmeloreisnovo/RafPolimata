#!/bin/sh
set -eu
# Uso: bash scripts/apkc_sign_debug.sh
# Gera keystore debug local e assina ApkC/proofs/out/hello.apk quando apksigner/keytool existem.
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/Apkc/proofs/out"
KS="$ROOT/Apkc/proofs/debug.keystore"
APK="$OUT/hello.apk"
SIGNED="$OUT/hello-signed.apk"
VERIFY="$OUT/apksigner-verify.txt"
SIGNLOG="$OUT/apksigner-sign.txt"
mkdir -p "$OUT"
{
  echo "# apksigner debug signing"
  echo "date_utc=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  echo "repo_commit=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
  echo "input_apk=$APK"
  if command -v sha256sum >/dev/null 2>&1 && [ -f "$APK" ]; then sha256sum "$APK"; fi
} > "$SIGNLOG"
if ! command -v apksigner >/dev/null 2>&1; then echo 'TOKEN_VAZIO: apksigner ausente; instale Android build-tools.' > "$VERIFY"; exit 0; fi
if ! command -v keytool >/dev/null 2>&1; then echo 'TOKEN_VAZIO: keytool ausente; instale JDK.' > "$VERIFY"; exit 0; fi
if [ ! -f "$APK" ]; then echo 'TOKEN_VAZIO: Apkc/proofs/out/hello.apk ausente; rode bash scripts/apkc_validate.sh.' > "$VERIFY"; exit 0; fi
if [ ! -f "$KS" ]; then
  keytool -genkeypair -v -keystore "$KS" -storepass android -keypass android -alias apkc-debug -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=ApkC Debug,O=Rafael,C=BR' > "$OUT/keytool-debug.txt" 2>&1
fi
apksigner sign --ks "$KS" --ks-key-alias apkc-debug --ks-pass pass:android --key-pass pass:android --out "$SIGNED" "$APK" >> "$SIGNLOG" 2>&1
{
  echo "# apksigner verify"
  echo "date_utc=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
  if command -v sha256sum >/dev/null 2>&1; then sha256sum "$APK" "$SIGNED"; fi
  apksigner verify --verbose "$SIGNED"
} > "$VERIFY" 2>&1
