#!/bin/sh
set -eu

: "${RLL_RELEASE_KEYSTORE_BASE64:?missing RLL_RELEASE_KEYSTORE_BASE64}"
: "${RLL_RELEASE_STORE_PASSWORD:?missing RLL_RELEASE_STORE_PASSWORD}"
: "${RLL_RELEASE_KEY_ALIAS:?missing RLL_RELEASE_KEY_ALIAS}"
: "${RLL_RELEASE_KEY_PASSWORD:?missing RLL_RELEASE_KEY_PASSWORD}"

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
INPUT_APK=${1:-"$ROOT/Apkc/proofs/out/hello.apk"}
OUTPUT_APK=${2:-"$ROOT/Apkc/proofs/out/hello-release-signed.apk"}
APKSIGNER=${APKSIGNER:-apksigner}
KEYSTORE=$(mktemp "${RUNNER_TEMP:-/tmp}/rafpolimata-release-keystore.XXXXXX")
cleanup() { rm -f "$KEYSTORE"; }
trap cleanup EXIT HUP INT TERM

test -f "$INPUT_APK"
printf '%s' "$RLL_RELEASE_KEYSTORE_BASE64" | base64 --decode > "$KEYSTORE"
chmod 600 "$KEYSTORE"

"$APKSIGNER" sign \
  --ks "$KEYSTORE" \
  --ks-key-alias "$RLL_RELEASE_KEY_ALIAS" \
  --ks-pass env:RLL_RELEASE_STORE_PASSWORD \
  --key-pass env:RLL_RELEASE_KEY_PASSWORD \
  --out "$OUTPUT_APK" \
  "$INPUT_APK"

"$APKSIGNER" verify --verbose --print-certs "$OUTPUT_APK"
