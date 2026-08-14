#!/usr/bin/env bash
set -u

# Bind installation to an already-open APK object instead of re-resolving the
# original pathname after the final digest check. This mitigates pathname
# replacement/rename TOCTOU. It does NOT claim protection against same-inode
# in-place mutation; claim_allowed remains false.

APK=${1:-}
EXPECTED_SHA256=${2:-}
OUT_DIR=${3:-}
shift 3 2>/dev/null || true

usage() {
  printf '%s\n' 'usage: apkc_install_fd_handoff.sh APK EXPECTED_SHA256 OUT_DIR -- COMMAND [ARGS... @APK_FD@ ...]' >&2
}

fail() {
  local rc=$1 msg=$2
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'handoff_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
    printf 'reason=%s\n' "$msg"
  } > "$OUT_DIR/fd-handoff.status" 2>/dev/null || true
  exit "$rc"
}

is_sha256() {
  local d=${1:-}
  case "$d" in ''|*[!0-9a-f]*) return 1 ;; esac
  [ "${#d}" -eq 64 ]
}

[ -n "$APK" ] && [ -n "$EXPECTED_SHA256" ] && [ -n "$OUT_DIR" ] || { usage; exit 106; }
[ "${1:-}" = -- ] || { usage; exit 106; }
shift
[ "$#" -gt 0 ] || { usage; exit 106; }
command -v sha256sum >/dev/null 2>&1 || fail 107 'sha256sum unavailable'
[ -f "$APK" ] && [ -s "$APK" ] || fail 108 'APK missing or empty'
is_sha256 "$EXPECTED_SHA256" || fail 109 'expected digest malformed'
mkdir -p "$OUT_DIR" || exit 110

# Open exactly once. FD 9 stays held by this shell across the child installer.
exec 9<"$APK" || fail 111 'cannot open APK for fd handoff'
FD_PATH="/proc/$$/fd/9"
[ -r "$FD_PATH" ] || fail 112 'proc fd path unavailable/unreadable'

OBSERVED_SHA256=$(sha256sum "$FD_PATH" 2>/dev/null | awk '{print $1}') || fail 113 'cannot hash fd-bound APK'
is_sha256 "$OBSERVED_SHA256" || fail 113 'fd-bound digest malformed'

if [ "$OBSERVED_SHA256" != "$EXPECTED_SHA256" ]; then
  {
    printf 'handoff_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
    printf 'expected_sha256=%s\n' "$EXPECTED_SHA256"
    printf 'fd_sha256=%s\n' "$OBSERVED_SHA256"
    printf 'reason=digest mismatch before installer handoff\n'
  } > "$OUT_DIR/fd-handoff.status"
  exit 114
fi

args=()
replaced=0
for arg in "$@"; do
  if [ "$arg" = '@APK_FD@' ]; then
    args+=("$FD_PATH")
    replaced=$((replaced + 1))
  else
    args+=("$arg")
  fi
done
[ "$replaced" -eq 1 ] || fail 115 'command must contain exactly one @APK_FD@ placeholder'

{
  printf 'handoff_status=READY\n'
  printf 'claim_allowed=false\n'
  printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
  printf 'expected_sha256=%s\n' "$EXPECTED_SHA256"
  printf 'fd_sha256=%s\n' "$OBSERVED_SHA256"
  printf 'fd_path=%s\n' "$FD_PATH"
} > "$OUT_DIR/fd-handoff.status"

"${args[@]}"
rc=$?

{
  printf 'installer_exit=%s\n' "$rc"
  printf 'claim_allowed=false\n'
} > "$OUT_DIR/installer-exit.status"

exit "$rc"
