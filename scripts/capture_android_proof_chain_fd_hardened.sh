#!/usr/bin/env bash
set -u

# Compatibility wrapper around the existing hardened Android chain.
# It does not replace existing signing/path/digest/receipt gates. Instead it
# interposes only the installer call so the final handoff is bound to an open
# file descriptor and the already-frozen SHA-256.
# Limitation: same-inode in-place mutation remains TOKEN_VAZIO.

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BRIDGE="${APKC_HARDENED_BRIDGE:-$ROOT/scripts/capture_android_proof_chain_hardened.sh}"
FD_GATE="${APKC_FD_HANDOFF_GATE:-$ROOT/scripts/apkc_install_fd_handoff.sh}"
OUT_DIR="${OUT_DIR:-proofs/run-arm64-full-chain/out}"
SHIM_DIR="$OUT_DIR/.fd-install-shim"
REAL_ADB="$(command -v adb 2>/dev/null || true)"
REAL_PM="$(command -v pm 2>/dev/null || true)"

fail() {
  local rc=$1 msg=$2
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'fd_bridge_status=FAIL\n'
    printf 'claim_allowed=false\n'
    printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
    printf 'reason=%s\n' "$msg"
  } > "$OUT_DIR/fd-bridge.status" 2>/dev/null || true
  exit "$rc"
}

[ -f "$BRIDGE" ] || fail 116 'hardened bridge missing'
[ -f "$FD_GATE" ] || fail 117 'fd handoff gate missing'
command -v bash >/dev/null 2>&1 || fail 118 'bash unavailable'

mkdir -p "$SHIM_DIR" || fail 119 'cannot create installer shim directory'

make_shim() {
  local name=$1 real=$2
  [ -n "$real" ] || return 0
  cat > "$SHIM_DIR/$name" <<'EOF'
#!/usr/bin/env bash
set -u
NAME="$(basename "$0")"
case "$NAME" in
  adb) REAL=${APKC_REAL_ADB:-} ;;
  pm)  REAL=${APKC_REAL_PM:-} ;;
  *) exit 126 ;;
esac
[ -n "$REAL" ] || exit 126

if [ "${1:-}" = install ]; then
  apk=''
  args=()
  for arg in "$@"; do
    case "$arg" in
      *.apk) apk=$arg; args+=("@APK_FD@") ;;
      *) args+=("$arg") ;;
    esac
  done
  [ -n "$apk" ] || exit 120
  freeze="${APKC_FD_OUT_DIR}/07_install_digest_gate/install-apk.sha256.freeze"
  [ -f "$freeze" ] || exit 121
  expected="$(sed -n '3s/^selected_apk_sha256=//p' "$freeze")"
  [ -n "$expected" ] || exit 122
  exec bash "$APKC_FD_GATE" "$apk" "$expected" "$APKC_FD_OUT_DIR/07_fd_handoff" -- "$REAL" "${args[@]}"
fi

exec "$REAL" "$@"
EOF
  chmod +x "$SHIM_DIR/$name" || fail 119 "cannot chmod $name shim"
}

make_shim adb "$REAL_ADB"
make_shim pm "$REAL_PM"

if [ -z "$REAL_ADB" ] && [ -z "$REAL_PM" ]; then
  # Preserve the original bridge behavior: it will emit TOKEN_VAZIO for install.
  rm -rf "$SHIM_DIR"
  exec bash "$BRIDGE"
fi

export APKC_REAL_ADB="$REAL_ADB"
export APKC_REAL_PM="$REAL_PM"
export APKC_FD_GATE="$FD_GATE"
export APKC_FD_OUT_DIR="$OUT_DIR"
export OUT_DIR
export PATH="$SHIM_DIR:$PATH"

{
  printf 'fd_bridge_status=READY\n'
  printf 'claim_allowed=false\n'
  printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
  printf 'real_adb_present=%s\n' "$([ -n "$REAL_ADB" ] && printf true || printf false)"
  printf 'real_pm_present=%s\n' "$([ -n "$REAL_PM" ] && printf true || printf false)"
} > "$OUT_DIR/fd-bridge.status"

bash "$BRIDGE"
rc=$?

# Do not promote runtime or same-inode protection even when the wrapped chain
# returns zero; success here only means all invoked gates accepted the run.
{
  printf 'fd_bridge_exit=%s\n' "$rc"
  printf 'claim_allowed=false\n'
  printf 'same_inode_inplace_protection=TOKEN_VAZIO\n'
} >> "$OUT_DIR/fd-bridge.status"

exit "$rc"
