#!/usr/bin/env bash
set -u
SCRIPT=${1:-scripts/apkc_finalize_install_receipt.sh}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
SHA_A=$(printf 'a%.0s' {1..64})
SHA_B=$(printf 'b%.0s' {1..64})
pass=0 fail=0

snapshot() {
  local d=$1 status=${2:-READY} expected=${3:-$SHA_A} observed=${4:-$SHA_A} seals=${5:-0xf}
  mkdir -p "$d"
  cat > "$d/sealed-memfd.status" <<EOT
sealed_snapshot_status=$status
claim_allowed=false
expected_sha256=$expected
sealed_snapshot_sha256=$observed
seals=$seals
EOT
}
installer() { printf 'installer_exit=%s\nclaim_allowed=false\n' "$2" > "$1/installer-exit.status"; }
run_case() {
  local name=$1 expected_rc=$2 dir=$3
  set +e
  "$SCRIPT" "$dir" >/dev/null 2>&1
  rc=$?
  set -e
  ok=0
  if [ "$expected_rc" = zero ]; then [ "$rc" -eq 0 ] && ok=1; else [ "$rc" -ne 0 ] && ok=1; fi
  if [ "$ok" -eq 1 ] && grep -q '^claim_allowed=false$' "$dir/apkc-install-chain.status"; then
    printf 'PASS %s rc=%s\n' "$name" "$rc"; pass=$((pass+1))
  else
    printf 'FAIL %s rc=%s\n' "$name" "$rc"; fail=$((fail+1))
  fi
}
set -e

d="$TMP/positive"; snapshot "$d"; installer "$d" 0; run_case positive zero "$d"; grep -q '^overall_status=PASS$' "$d/apkc-install-chain.status"
d="$TMP/nonzero"; snapshot "$d"; installer "$d" 23; run_case installer_nonzero nonzero "$d"; grep -q '^reason=INSTALLER_NONZERO$' "$d/apkc-install-chain.status"
d="$TMP/missing_inst"; snapshot "$d"; run_case missing_installer_status nonzero "$d"; grep -q '^reason=INSTALLER_EXIT_STATUS_MISSING$' "$d/apkc-install-chain.status"
d="$TMP/dup_exit"; snapshot "$d"; installer "$d" 0; printf 'installer_exit=0\n' >> "$d/installer-exit.status"; run_case duplicate_installer_exit nonzero "$d"
d="$TMP/digest"; snapshot "$d" READY "$SHA_A" "$SHA_B" 0xf; installer "$d" 0; run_case digest_mismatch nonzero "$d"; grep -q '^reason=SNAPSHOT_DIGEST_MISMATCH$' "$d/apkc-install-chain.status"
d="$TMP/notready"; snapshot "$d" FAIL; installer "$d" 0; run_case snapshot_not_ready nonzero "$d"
d="$TMP/seals"; snapshot "$d" READY "$SHA_A" "$SHA_A" 0x7; installer "$d" 0; run_case incomplete_seals nonzero "$d"
d="$TMP/malformed"; snapshot "$d"; installer "$d" nope; run_case malformed_installer_exit nonzero "$d"

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$pass" -eq 8 ] && [ "$fail" -eq 0 ]
