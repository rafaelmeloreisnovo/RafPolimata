#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SEAL="$ROOT/scripts/apkc_seal_receipt_tree.sh"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/apkc-tree-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
pass=0; fail=0
ok(){ printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad(){ printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }

# 1. Nested evidence is included and verifies.
D="$TMP/nested"; mkdir -p "$D/install-chain/deep"; printf 'root\n' > "$D/top.txt"; printf 'installer\n' > "$D/install-chain/deep/status.txt"
bash "$SEAL" "$D" >/dev/null
if grep -Fq './install-chain/deep/status.txt' "$D/receipt-tree.sha256" && (cd "$D" && sha256sum -c receipt-tree.sha256 >/dev/null); then ok nested_evidence_bound; else bad nested_evidence_bound; fi

# 2. Post-receipt nested tamper is detected.
printf 'tampered\n' > "$D/install-chain/deep/status.txt"
if (cd "$D" && sha256sum -c receipt-tree.sha256 >/dev/null 2>&1); then bad nested_tamper_detected; else ok nested_tamper_detected; fi

# 3. Symlink anywhere in the evidence tree is rejected fail-closed.
S="$TMP/symlink"; mkdir -p "$S/deep"; printf 'x\n' > "$S/deep/a"; ln -s deep/a "$S/link"
if bash "$SEAL" "$S" >/dev/null 2>&1; then bad symlink_rejected; else ok symlink_rejected; fi

# 4. Manifest is deterministic when evidence bytes are unchanged.
R="$TMP/repro"; mkdir -p "$R/a b"; printf '1\n' > "$R/z"; printf '2\n' > "$R/a b/y"
bash "$SEAL" "$R" >/dev/null; cp "$R/receipt-tree.sha256" "$TMP/m1"
bash "$SEAL" "$R" >/dev/null; cp "$R/receipt-tree.sha256" "$TMP/m2"
if cmp -s "$TMP/m1" "$TMP/m2"; then ok deterministic_manifest; else bad deterministic_manifest; fi

# 5. Empty evidence tree is rejected.
E="$TMP/empty"; mkdir -p "$E"
if bash "$SEAL" "$E" >/dev/null 2>&1; then bad empty_tree_rejected; else ok empty_tree_rejected; fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
