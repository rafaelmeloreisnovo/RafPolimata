#!/bin/sh
set -eu
SELF_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SRC="${1:-}"
OUT="${2:-$SELF_DIR/work-v4.1}"
EXPECTED_SRC="d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454"
EXPECTED_PATCH="739d0123f953c3bde2546095788dc2fc62143fcf0e016997b2ce2615378c4c68"
PATCH_TMP="${TMPDIR:-/tmp}/rafaelia-v4.1-hardening.$$.patch"
fail(){ printf 'FAIL %s\n' "$*" >&2; rm -f "$PATCH_TMP"; exit 1; }
pass(){ printf 'PASS %s\n' "$*"; }
trap 'rm -f "$PATCH_TMP"' EXIT HUP INT TERM
[ -n "$SRC" ] || fail 'usage: MATERIALIZE_AND_VERIFY.sh /path/RAFAELIA_COMPLETE_v4.zip [out]'
[ -f "$SRC" ] || fail source-not-found
command -v unzip >/dev/null 2>&1 || fail unzip-missing
command -v patch >/dev/null 2>&1 || fail patch-missing
command -v sha256sum >/dev/null 2>&1 || fail sha256sum-missing
ACTUAL_SRC=$(sha256sum "$SRC" | awk '{print $1}')
[ "$ACTUAL_SRC" = "$EXPECTED_SRC" ] || fail source-sha256
pass source-sha256
cat "$SELF_DIR"/PATCH_PARTS/*.part > "$PATCH_TMP" || fail patch-reassembly
ACTUAL_PATCH=$(sha256sum "$PATCH_TMP" | awk '{print $1}')
[ "$ACTUAL_PATCH" = "$EXPECTED_PATCH" ] || fail patch-sha256
pass patch-sha256
rm -rf "$OUT"; mkdir -p "$OUT"
unzip -q "$SRC" -d "$OUT" || fail source-unzip
( cd "$OUT" && patch -p1 --forward --batch < "$PATCH_TMP" ) || fail patch-apply
pass hardening-patch
[ -x "$OUT/verify_hardened.sh" ] || chmod +x "$OUT/verify_hardened.sh"
"$OUT/verify_hardened.sh" "$OUT/.verify-hardened"
