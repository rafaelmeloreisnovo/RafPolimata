#!/usr/bin/env bash
set -u
OUT_DIR=${1:-}
[ -n "$OUT_DIR" ] || { printf 'usage: %s OUT_DIR\n' "$0" >&2; exit 181; }
SNAP="$OUT_DIR/sealed-stdin.status"
INST="$OUT_DIR/installer-exit.status"
FINAL="$OUT_DIR/apkc-stdin-install-chain.status"
TMP="$FINAL.tmp.$$"
emit(){
  local rc=$1 overall=$2 reason=$3 snapshot=${4:-TOKEN_VAZIO} installer=${5:-TOKEN_VAZIO} expected=${6:-TOKEN_VAZIO} observed=${7:-TOKEN_VAZIO} seals=${8:-TOKEN_VAZIO} bytes=${9:-TOKEN_VAZIO}
  mkdir -p "$OUT_DIR" 2>/dev/null || true
  {
    printf 'schema=rafaelia.apkc-stdin-install-chain-status/v1\n'
    printf 'overall_status=%s\ntransport_mode=SEALED_MEMFD_STDIN\n' "$overall"
    printf 'snapshot_status=%s\ninstaller_exit=%s\n' "$snapshot" "$installer"
    printf 'expected_sha256=%s\nsealed_snapshot_sha256=%s\n' "$expected" "$observed"
    printf 'seals=%s\napk_bytes=%s\nclaim_allowed=false\nreason=%s\n' "$seals" "$bytes" "$reason"
  } > "$TMP" || exit 189
  mv -f "$TMP" "$FINAL" || exit 190
  exit "$rc"
}
single_value(){ local file=$1 key=$2 count value; count=$(grep -c "^${key}=" "$file" 2>/dev/null || true); [ "$count" -eq 1 ] || return 1; value=$(grep "^${key}=" "$file" | cut -d= -f2-); [ -n "$value" ] || return 1; printf '%s' "$value"; }
is_sha256(){ [[ ${1:-} =~ ^[0-9a-f]{64}$ ]]; }
[ -f "$SNAP" ] || emit 182 FAIL SNAPSHOT_STATUS_MISSING
[ -f "$INST" ] || emit 183 FAIL INSTALLER_EXIT_STATUS_MISSING
snapshot=$(single_value "$SNAP" sealed_stdin_status) || emit 184 FAIL SNAPSHOT_STATUS_MALFORMED
transport=$(single_value "$SNAP" transport_mode) || emit 184 FAIL TRANSPORT_MODE_MISSING_OR_DUPLICATE "$snapshot"
expected=$(single_value "$SNAP" expected_sha256) || emit 184 FAIL EXPECTED_SHA256_MISSING_OR_DUPLICATE "$snapshot"
observed=$(single_value "$SNAP" sealed_snapshot_sha256) || emit 184 FAIL OBSERVED_SHA256_MISSING_OR_DUPLICATE "$snapshot" TOKEN_VAZIO "$expected"
seals=$(single_value "$SNAP" seals) || emit 184 FAIL SEALS_MISSING_OR_DUPLICATE "$snapshot" TOKEN_VAZIO "$expected" "$observed"
bytes=$(single_value "$SNAP" apk_bytes) || emit 184 FAIL APK_BYTES_MISSING_OR_DUPLICATE "$snapshot" TOKEN_VAZIO "$expected" "$observed" "$seals"
installer=$(single_value "$INST" installer_exit) || emit 185 FAIL INSTALLER_EXIT_MALFORMED "$snapshot" TOKEN_VAZIO "$expected" "$observed" "$seals" "$bytes"
[ "$transport" = SEALED_MEMFD_STDIN ] || emit 186 FAIL TRANSPORT_MODE_INVALID "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
is_sha256 "$expected" || emit 186 FAIL EXPECTED_SHA256_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
is_sha256 "$observed" || emit 186 FAIL OBSERVED_SHA256_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[ "$expected" = "$observed" ] || emit 186 FAIL SNAPSHOT_DIGEST_MISMATCH "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[ "$snapshot" = READY ] || emit 187 FAIL SNAPSHOT_NOT_READY "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[[ "$seals" =~ ^0x[0-9a-fA-F]+$ ]] || emit 187 FAIL SEALS_MALFORMED "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
seal_num=$((seals)); (( (seal_num & 0x0f) == 0x0f )) || emit 187 FAIL REQUIRED_SEALS_INCOMPLETE "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[[ "$bytes" =~ ^[1-9][0-9]*$ ]] || emit 187 FAIL APK_BYTES_INVALID "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[[ "$installer" =~ ^[0-9]+$ ]] || emit 185 FAIL INSTALLER_EXIT_NOT_INTEGER "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
[ "$installer" -eq 0 ] || emit "$installer" FAIL INSTALLER_NONZERO "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
emit 0 PASS SEALED_STDIN_AND_INSTALLER_PASS "$snapshot" "$installer" "$expected" "$observed" "$seals" "$bytes"
