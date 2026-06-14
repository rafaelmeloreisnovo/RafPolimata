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
mkdir -p "$OUT"
if ! command -v apksigner >/dev/null 2>&1; then echo 'TOKEN_VAZIO: apksigner ausente; instale Android build-tools.' > "$VERIFY"; exit 0; fi
if ! command -v keytool >/dev/null 2>&1; then echo 'TOKEN_VAZIO: keytool ausente; instale JDK.' > "$VERIFY"; exit 0; fi
if [ ! -f "$APK" ]; then echo 'TOKEN_VAZIO: Apkc/proofs/out/hello.apk ausente; rode bash scripts/apkc_validate.sh.' > "$VERIFY"; exit 0; fi
if [ ! -f "$KS" ]; then
  keytool -genkeypair -v -keystore "$KS" -storepass android -keypass android -alias apkc-debug -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=ApkC Debug,O=Local,C=US' > "$OUT/keytool-debug.txt" 2>&1
fi
cp "$APK" "$SIGNED"
apksigner sign --ks "$KS" --ks-key-alias apkc-debug --ks-pass pass:android --key-pass pass:android --out "$SIGNED" "$APK" > "$OUT/apksigner-sign.txt" 2>&1
apksigner verify --verbose "$SIGNED" > "$VERIFY" 2>&1
