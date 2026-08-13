#!/usr/bin/env bash
set -u

OUT_DIR="${1:-}"
[ -n "$OUT_DIR" ] || { printf '%s\n' 'usage: apkc_path_canonicality_gate.sh OUT_DIR' >&2; exit 64; }
[ -d "$OUT_DIR" ] || { printf '%s\n' 'OUT_DIR missing' >&2; exit 65; }
OUT_DIR="$(cd "$OUT_DIR" 2>/dev/null && pwd -P)" || exit 65
TARGET_FILE="$OUT_DIR/06_sign_gate/install-target.txt"

[ -f "$TARGET_FILE" ] || { printf '%s\n' 'install target evidence missing' >&2; exit 68; }
[ "$(wc -l < "$TARGET_FILE" | tr -d ' ')" = 1 ] || { printf '%s\n' 'install target must contain exactly one line' >&2; exit 69; }
IFS= read -r TARGET_RAW < "$TARGET_FILE" || exit 70
[ -n "$TARGET_RAW" ] && [ "$TARGET_RAW" != TOKEN_VAZIO ] || exit 71

case "$TARGET_RAW" in
  /*)
    case "$TARGET_RAW" in
      *//*|*'/../'*|*/..|*/./*|*/.|*\\*) printf '%s\n' 'non-canonical legacy absolute install target' >&2; exit 74 ;;
    esac
    ;;
  *)
    case "$TARGET_RAW" in
      ''|/*|*//*|../*|*/../*|*/..|..|.|./*|*/./*|*/.|*\\*) printf '%s\n' 'non-canonical relative install target' >&2; exit 74 ;;
    esac
    ;;
esac
case "$TARGET_RAW" in
  *$'\n'*|*$'\r'*|*$'\t'*) printf '%s\n' 'install target contains forbidden control character' >&2; exit 74 ;;
esac

printf '%s\n' 'path_canonicality=PASS'
printf '%s\n' 'backslash_alias=REJECTED_BY_POLICY'
printf '%s\n' 'redundant_slash_alias=REJECTED_BY_POLICY'
printf '%s\n' 'claim_allowed=false'
exit 0
