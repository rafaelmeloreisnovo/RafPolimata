#!/usr/bin/env bash
set -euo pipefail
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}
SCRIPT=${APKC_C04_TRANSFER_SCRIPT:-$ROOT/scripts/apkc_verify_provider_c04_transfer.sh}
ARTIFACT=${APKC_C04_PROVIDER_ARTIFACT:-${1:-}}
[ -n "$ARTIFACT" ] || { echo "TOKEN_VAZIO_PROVIDER_ARTIFACT_INPUT: set APKC_C04_PROVIDER_ARTIFACT or pass artifact ZIP path" >&2; exit 240; }
[ -f "$SCRIPT" ] || { echo "missing transfer script" >&2; exit 241; }
TMP=$(mktemp -d "${TMPDIR:-/tmp}/apkc-c04-transfer-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
pass=0; total=0
run(){ name=$1; expect=$2; shift 2; total=$((total+1)); set +e; "$@" >"$TMP/$name.out" 2>"$TMP/$name.err"; rc=$?; set -e; if [ "$rc" -eq "$expect" ]; then echo "PASS $name rc=$rc"; pass=$((pass+1)); else echo "FAIL $name rc=$rc expected=$expect"; cat "$TMP/$name.err" >&2 || true; fi; }
run positive 0 bash "$SCRIPT" "$ARTIFACT" "$TMP/positive" extract-only
cp "$ARTIFACT" "$TMP/tampered.zip"
printf X | dd of="$TMP/tampered.zip" bs=1 seek=101 conv=notrunc status=none
run artifact_digest_mismatch 245 bash "$SCRIPT" "$TMP/tampered.zip" "$TMP/tampered-out" extract-only
run missing_artifact 244 bash "$SCRIPT" "$TMP/no-such.zip" "$TMP/missing-out" extract-only
run unknown_mode 253 bash "$SCRIPT" "$ARTIFACT" "$TMP/unknown-out" nonsense
run physical_gate_missing 252 env APKC_TERMUX_GATE="$TMP/no-gate.sh" bash "$SCRIPT" "$ARTIFACT" "$TMP/gate-missing-out" probe-only
sha=$(sha256sum "$TMP/positive/hello.c04.provider.apk" | awk '{print $1}')
[ "$sha" = 7957e4421921da85924cb278fce38db8d6ba747c99ecb9eb7e04edfbe42e9ff3 ] || { echo "FAIL positive_apk_sha256 $sha"; exit 242; }
echo "RESULT pass=$pass total=$total claim_allowed=false"
[ "$pass" -eq "$total" ]
