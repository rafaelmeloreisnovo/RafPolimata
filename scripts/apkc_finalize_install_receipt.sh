#!/usr/bin/env bash
set -u

OUT_DIR=${1:-}
[ -n "$OUT_DIR" ] || { printf 'usage: %s OUT_DIR\n' "$0" >&2; exit 151; }
SNAP="$OUT_DIR/sealed-memfd.status"
INST="$OUT_DIR/installer-exit.status"
FINAL="$OUT_DIR/apkc-install-chain.status"
TMP="$FINAL.tmp.$$"

emit() {
  local rc=$1 overall=$2 reason=$3 snapshot=${4:-TOKEN_VAZIO} installer=${5:-TOKEN_VAZIO} expected=${6:-TOKEN_VAZIO} observed=${7:-TOKEN_VAZIO} seals=${8:-TOKEN_VAZIO}
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'schema=rafaelia.apkc-install-chain-status/v1\n'
    printf 'overall_status=%s\n' "$overall"
    printf 'snapshot_status=%s\n' "$snapshot"
    printf 'installer_exit=%s\n' "$installer"
    printf 'expected_sha256=%s\n' "$expected"
    printf 'sealed_snapshot_sha256=%s\n' "$observed"
    printf 'seals=%s\n' "$seals"
    printf 'claim_allowed=false\n'
    printf 'reason=%s\n' "$reason"
  } > "$TMP" || exit 159
  mv -f "$TMP" "$FINAL" || exit 160
  exit "$rc"
}

single_value() {
  local file=$1 key=$2 count value
  count=$(grep -c "^${key}=" "$file" 2>/dev/null || true)
  [ "$count" -eq 1 ] || return 1
  value=$(grep "^${key}=" "$file" | cut -d= -f2-)
  [ -n "$value" ] || return 1
  printf '%s' "$value"
}

is_sha256() { [[ ${1:-} =~ ^[0-9a-f]{64}$ ]]; }

[ -f "$SNAP" ] || emit 152 FAIL SNAPSHOT_STATUS_MISSING
[ -f "$INST" ] || emit 153 FAIL INSTALLER_EXIT_STATUS_MISSING

snapshot=$(single_value "$SNAP" sealed_snapshot_status) || emit 154 FAIL SNAPSHOT_STATUS_MALFORMED
expected=$(single_value "$SNAP" expected_sha256) || emit 154 FAIL EXPECTED_SHA256_MISSING_OR_DUPLICATE "$snapshot"
observed=$(single_value "$SNAP" sealed_snapshot_sha256) || emit 154 FAIL OBSERVED_SHA256_MISSING_OR_DUPLICATE "$snapshot" TOKEN_VAZIO "$expected"
seals=$(single_value "$SNAP" seals) || emit 154 FAIL SEALS_MISSING_OR_DUPLICATE "$snapshot" TOKEN_VAZIO "$expected" "$observed"
installer=$(single_value "$INST" installer_exit) || emit 155 FAIL INSTALLER_EXIT_MALFORMED "$snapshot" TOKEN_VAZIO "$expected" "$observed" "$seals"

is_sha256 "$expected" || emit 156 FAIL EXPECTED_SHA256_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals"
is_sha256 "$observed" || emit 156 FAIL OBSERVED_SHA256_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals"
[ "$expected" = "$observed" ] || emit 156 FAIL SNAPSHOT_DIGEST_MISMATCH "$snapshot" "$installer" "$expected" "$observed" "$seals"
[ "$snapshot" = READY ] || emit 157 FAIL SNAPSHOT_NOT_READY "$snapshot" "$installer" "$expected" "$observed" "$seals"
[[ "$seals" =~ ^0x[0-9a-fA-F]+$ ]] || emit 157 FAIL SEALS_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals"
seal_num=$((seals))
(( (seal_num & 0x0f) == 0x0f )) || emit 157 FAIL REQUIRED_SEALS_INCOMPLETE "$snapshot" "$installer" "$expected" "$observed" "$seals"
[[ "$installer" =~ ^[0-9]+$ ]] || emit 155 FAIL INSTALLER_EXIT_NOT_INTEGER "$snapshot" "$installer" "$expected" "$observed" "$seals"
[ "$installer" -eq 0 ] || emit "$installer" FAIL INSTALLER_NONZERO "$snapshot" "$installer" "$expected" "$observed" "$seals"

emit 0 PASS SNAPSHOT_AND_INSTALLER_PASS "$snapshot" "$installer" "$expected" "$observed" "$seals"
