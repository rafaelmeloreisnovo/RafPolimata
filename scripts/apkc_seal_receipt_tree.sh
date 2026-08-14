#!/usr/bin/env bash
set -euo pipefail
ROOT=${1:-}
OUT_NAME=${2:-receipt-tree.sha256}
[ -n "$ROOT" ] || { echo "usage: $0 ROOT_DIR [MANIFEST_NAME]" >&2; exit 271; }
[ -d "$ROOT" ] || { echo "receipt_tree_status=FAIL reason=ROOT_MISSING claim_allowed=false" >&2; exit 272; }
case "$OUT_NAME" in ''|*/*|'.'|'..') echo "receipt_tree_status=FAIL reason=MANIFEST_NAME_INVALID claim_allowed=false" >&2; exit 273;; esac
command -v sha256sum >/dev/null 2>&1 || { echo "receipt_tree_status=FAIL reason=SHA256SUM_MISSING claim_allowed=false" >&2; exit 274; }
command -v find >/dev/null 2>&1 || { echo "receipt_tree_status=FAIL reason=FIND_MISSING claim_allowed=false" >&2; exit 275; }
command -v sort >/dev/null 2>&1 || { echo "receipt_tree_status=FAIL reason=SORT_MISSING claim_allowed=false" >&2; exit 276; }
VERIFY_NAME="${OUT_NAME}.verify.txt"
if find -P "$ROOT" -type l -print -quit | grep -q .; then
  echo "receipt_tree_status=FAIL reason=SYMLINK_PRESENT claim_allowed=false" >&2
  exit 277
fi
TMP=$(mktemp "${TMPDIR:-/tmp}/apkc-receipt-tree.XXXXXX") || exit 278
trap 'rm -f "$TMP"' EXIT
(
  cd "$ROOT"
  find -P . -type f ! -path "./$OUT_NAME" ! -path "./$VERIFY_NAME" -print0 \
    | LC_ALL=C sort -z \
    | while IFS= read -r -d '' f; do sha256sum "$f"; done
) > "$TMP"
[ -s "$TMP" ] || { echo "receipt_tree_status=FAIL reason=EMPTY_EVIDENCE_TREE claim_allowed=false" >&2; exit 279; }
mv "$TMP" "$ROOT/$OUT_NAME"
trap - EXIT
(
  cd "$ROOT"
  sha256sum -c "$OUT_NAME" > "$VERIFY_NAME" 2>&1
) || { printf 'receipt_tree_status=FAIL\nreason=VERIFY_FAILED\nclaim_allowed=false\n' >> "$ROOT/$VERIFY_NAME"; exit 280; }
printf 'receipt_tree_status=PASS\nmanifest=%s\nclaim_allowed=false\n' "$OUT_NAME" >> "$ROOT/$VERIFY_NAME"
