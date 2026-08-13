#!/usr/bin/env bash
set -u

# ApkC pre-install digest binding gate.
# Binds the bytes selected after signing/path validation to the bytes observed
# immediately before adb/pm install. This is an integrity/TOCTOU gate only;
# it does not promote runtime or signing claims.

MODE="${1:-}"
APK="${2:-}"
STATE_DIR="${3:-}"

usage() {
  printf '%s\n' 'usage: apkc_install_digest_gate.sh <freeze|verify> <apk> <state_dir>' >&2
  exit 110
}

case "$MODE" in
  freeze|verify) ;;
  *) usage ;;
esac

[ -n "$APK" ] && [ -n "$STATE_DIR" ] || usage
command -v sha256sum >/dev/null 2>&1 || {
  printf '%s\n' 'TOKEN_VAZIO: sha256sum unavailable' >&2
  exit 111
}
[ -f "$APK" ] && [ -s "$APK" ] || {
  printf '%s\n' 'TOKEN_VAZIO: selected APK missing or empty' >&2
  exit 112
}
mkdir -p "$STATE_DIR" || exit 113

FREEZE="$STATE_DIR/install-apk.sha256.freeze"
VERIFY="$STATE_DIR/install-apk.sha256.verify"

digest_of() {
  sha256sum "$1" | awk '{print $1}'
}

is_digest() {
  case "$1" in
    *[!0-9a-f]*|'') return 1 ;;
  esac
  [ "${#1}" -eq 64 ]
}

case "$MODE" in
  freeze)
    digest="$(digest_of "$APK")" || exit 114
    is_digest "$digest" || exit 115
    {
      printf 'format=APKC_INSTALL_DIGEST_BINDING_V1\n'
      printf 'claim_allowed=false\n'
      printf 'selected_apk_sha256=%s\n' "$digest"
    } > "$FREEZE" || exit 116
    printf '%s\n' "$digest"
    ;;

  verify)
    [ -f "$FREEZE" ] || {
      printf '%s\n' 'TOKEN_VAZIO: freeze record missing' >&2
      exit 117
    }
    [ "$(wc -l < "$FREEZE" | tr -d ' ')" = 3 ] || exit 118
    [ "$(sed -n '1p' "$FREEZE")" = 'format=APKC_INSTALL_DIGEST_BINDING_V1' ] || exit 118
    [ "$(sed -n '2p' "$FREEZE")" = 'claim_allowed=false' ] || exit 118
    frozen="$(sed -n '3s/^selected_apk_sha256=//p' "$FREEZE")"
    is_digest "$frozen" || exit 118

    current="$(digest_of "$APK")" || exit 119
    is_digest "$current" || exit 119
    if [ "$current" != "$frozen" ]; then
      {
        printf 'binding_status=FAIL\n'
        printf 'claim_allowed=false\n'
        printf 'frozen_sha256=%s\n' "$frozen"
        printf 'observed_sha256=%s\n' "$current"
      } > "$VERIFY"
      printf '%s\n' 'TOKEN_VAZIO: install-boundary digest mismatch' >&2
      exit 120
    fi
    {
      printf 'binding_status=PASS\n'
      printf 'claim_allowed=false\n'
      printf 'frozen_sha256=%s\n' "$frozen"
      printf 'observed_sha256=%s\n' "$current"
    } > "$VERIFY" || exit 121
    printf '%s\n' "$current"
    ;;
esac
