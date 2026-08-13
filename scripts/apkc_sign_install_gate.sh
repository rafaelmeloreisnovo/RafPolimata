#!/usr/bin/env bash
set -u

# ApkC/RAFAELIA signing -> install selection gate.
# It never asserts runtime success. It only exposes an install target when the
# signing policy is satisfied, preserving claim_allowed=false.

RAW=${1:-}
SIGNED=${2:-}
OUT_DIR=${3:-}
DO_SIGN=${DO_SIGN:-auto}

if [ -z "$RAW" ] || [ -z "$SIGNED" ] || [ -z "$OUT_DIR" ]; then
  printf '%s\n' 'usage: apkc_sign_install_gate.sh RAW_APK SIGNED_APK OUT_DIR' >&2
  exit 64
fi

mkdir -p "$OUT_DIR"
TARGET="$OUT_DIR/install-target.txt"
STATUS="$OUT_DIR/signing-status.txt"
VERIFY="$OUT_DIR/apksigner-verify.txt"
SIGN_LOG="$OUT_DIR/apksigner-sign.txt"
VERIFIED_DIGEST="$OUT_DIR/verified-apk.sha256"
: > "$STATUS"
printf 'TOKEN_VAZIO\n' > "$TARGET"
printf 'TOKEN_VAZIO\n' > "$VERIFIED_DIGEST"

fail() {
  code=$1
  shift
  printf 'TOKEN_VAZIO\n' > "$TARGET"
  printf 'TOKEN_VAZIO\n' > "$VERIFIED_DIGEST"
  printf 'status=FAIL\nreason=%s\nclaim_allowed=false\n' "$*" > "$STATUS"
  exit "$code"
}

sha256_file() {
  local file=$1 digest
  digest=$(sha256sum "$file" 2>/dev/null | awk '{print $1}') || return 1
  case "$digest" in
    ''|*[!0-9a-f]*) return 1 ;;
  esac
  [ "${#digest}" -eq 64 ] || return 1
  printf '%s\n' "$digest"
}

verify_and_bind_digest() {
  local apk=$1 before after
  command -v sha256sum >/dev/null 2>&1 || fail 87 'sha256sum unavailable; verified digest cannot be bound'
  before=$(sha256_file "$apk") || fail 87 'unable to hash APK before apksigner verification'
  if ! apksigner verify --verbose --print-certs "$apk" >"$VERIFY" 2>&1; then
    return 1
  fi
  after=$(sha256_file "$apk") || fail 87 'unable to hash APK after apksigner verification'
  [ "$before" = "$after" ] || fail 87 'APK bytes changed during apksigner verification; target withheld'
  printf '%s\n' "$after" > "$VERIFIED_DIGEST"
  return 0
}

[ -s "$RAW" ] || fail 80 'raw APK missing/empty'
case "$DO_SIGN" in
  0|1|auto) ;;
  *) fail 81 'DO_SIGN must be 0, 1, or auto' ;;
esac

# Explicit compatibility escape hatch. The caller consciously disables this
# signing gate; no signing claim is promoted and verified digest stays unknown.
if [ "$DO_SIGN" = 0 ]; then
  printf '%s\n' "$RAW" > "$TARGET"
  printf 'status=SKIPPED_EXPLICIT\nartifact=raw\nverified_digest=TOKEN_VAZIO\nclaim_allowed=false\n' > "$STATUS"
  exit 0
fi

command -v apksigner >/dev/null 2>&1 || fail 82 'apksigner unavailable; install target withheld'

if [ -n "${APKSIGNER_KEYSTORE:-}" ]; then
  rm -f "$SIGNED"
  if ! apksigner sign --ks "$APKSIGNER_KEYSTORE" --out "$SIGNED" "$RAW" >"$SIGN_LOG" 2>&1 || [ ! -s "$SIGNED" ]; then
    rm -f "$SIGNED"
    fail 83 'signing failed or signed APK missing/empty'
  fi
  if ! verify_and_bind_digest "$SIGNED"; then
    rm -f "$SIGNED"
    fail 84 'signed APK failed verification'
  fi
  printf '%s\n' "$SIGNED" > "$TARGET"
  printf 'status=PASS\nartifact=signed\nverification=apksigner_verbose_print_certs\nverified_digest_file=verified-apk.sha256\nclaim_allowed=false\n' > "$STATUS"
  exit 0
fi

# Required signing must never fall through to the raw artifact.
if [ "$DO_SIGN" = 1 ]; then
  fail 85 'DO_SIGN=1 requires APKSIGNER_KEYSTORE; install target withheld'
fi

# Auto mode may reuse an already-signed raw APK, but only after cryptographic
# verification. The certificate details and exact verified digest are preserved.
if verify_and_bind_digest "$RAW"; then
  printf '%s\n' "$RAW" > "$TARGET"
  printf 'status=PASS\nartifact=raw_already_signed\nverification=apksigner_verbose_print_certs\nverified_digest_file=verified-apk.sha256\nclaim_allowed=false\n' > "$STATUS"
  exit 0
fi

fail 86 'auto mode found no verifiably signed artifact; install target withheld'
