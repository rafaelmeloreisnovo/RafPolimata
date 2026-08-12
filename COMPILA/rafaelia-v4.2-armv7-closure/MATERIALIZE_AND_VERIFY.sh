#!/bin/sh
set -eu
SELF_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SRC="${1:-}"
OUT="${2:-$SELF_DIR/work-v4.2}"
PARENT="$SELF_DIR/../rafaelia-v4.1-hardened/MATERIALIZE_AND_VERIFY.sh"
PATCH_B64="$SELF_DIR/v4.1-to-v4.2.patch.gz.b64"
EXPECTED_SRC="d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454"
EXPECTED_PATCH="50aa730140f0577f2672c205f3294ce6acf02b07302cec57c8dc847e071a2a56"
PATCH_TMP="${TMPDIR:-/tmp}/rafaelia-v4.2-armv7.$$.patch"
fail(){ printf 'FAIL %s\n' "$*" >&2; rm -f "$PATCH_TMP"; exit 1; }
pass(){ printf 'PASS %s\n' "$*"; }
trap 'rm -f "$PATCH_TMP"' EXIT HUP INT TERM
[ -n "$SRC" ] || fail 'usage: MATERIALIZE_AND_VERIFY.sh /path/RAFAELIA_COMPLETE_v4.zip [out]'
[ -x "$PARENT" ] || fail parent-v4.1-materializer-missing
[ -f "$SRC" ] || fail source-not-found
[ -f "$PATCH_B64" ] || fail v4.2-patch-base64-missing
command -v sha256sum >/dev/null 2>&1 || fail sha256sum-missing
command -v base64 >/dev/null 2>&1 || fail base64-missing
command -v gzip >/dev/null 2>&1 || fail gzip-missing
command -v patch >/dev/null 2>&1 || fail patch-missing
ACTUAL_SRC=$(sha256sum "$SRC" | awk '{print $1}')
[ "$ACTUAL_SRC" = "$EXPECTED_SRC" ] || fail source-sha256
pass source-sha256-v4
"$PARENT" "$SRC" "$OUT" || fail parent-v4.1-materialization
pass parent-v4.1-materialization
base64 -d "$PATCH_B64" | gzip -dc > "$PATCH_TMP" || fail v4.2-patch-decompression
ACTUAL_PATCH=$(sha256sum "$PATCH_TMP" | awk '{print $1}')
[ "$ACTUAL_PATCH" = "$EXPECTED_PATCH" ] || fail v4.2-patch-sha256
pass v4.2-patch-sha256
( cd "$OUT" && patch -p1 --forward --batch < "$PATCH_TMP" ) || fail v4.2-patch-apply
chmod +x "$OUT/verify_hardened.sh" "$OUT/omega_final_gate.sh"
pass v4.2-patch-apply
"$OUT/verify_hardened.sh" "$OUT/.verify-v4.2"
