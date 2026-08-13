#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd -P)"
GATE="$ROOT/scripts/apkc_path_canonicality_gate.sh"
TMP="$(mktemp -d)" || exit 1
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
PASS=0
FAIL=0

ok(){ PASS=$((PASS+1)); printf 'PASS %s\n' "$1"; }
bad(){ FAIL=$((FAIL+1)); printf 'FAIL %s\n' "$1" >&2; }
expect_pass(){ name="$1"; shift; if "$@" >/dev/null 2>&1; then ok "$name"; else bad "$name"; fi; }
expect_fail(){ name="$1"; shift; if "$@" >/dev/null 2>&1; then bad "$name"; else ok "$name"; fi; }

mkcase(){ d="$1"; value="$2"; mkdir -p "$d/06_sign_gate"; printf '%s\n' "$value" > "$d/06_sign_gate/install-target.txt"; }

mkcase "$TMP/canonical" '07_signed/app.apk'
expect_pass canonical_relative bash "$GATE" "$TMP/canonical"

mkcase "$TMP/dot" '07_signed/./app.apk'
expect_fail dot_segment_rejected bash "$GATE" "$TMP/dot"

mkcase "$TMP/backslash" '07_signed\app.apk'
expect_fail backslash_alias_rejected bash "$GATE" "$TMP/backslash"

mkcase "$TMP/traversal" '../outside.apk'
expect_fail traversal_rejected bash "$GATE" "$TMP/traversal"

mkcase "$TMP/legacy-ok" '/old/device/run/07_signed/app.apk'
expect_pass legacy_absolute_canonical bash "$GATE" "$TMP/legacy-ok"

mkcase "$TMP/legacy-backslash" '/old/device/run/07_signed\app.apk'
expect_fail legacy_absolute_backslash_rejected bash "$GATE" "$TMP/legacy-backslash"

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
