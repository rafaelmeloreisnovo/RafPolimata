#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
WRAP="$ROOT/scripts/apkc_termux_sealed_stdin_gate_receipted.sh"
TMP=$(mktemp -d "${TMPDIR:-/tmp}/apkc-wrap-test.XXXXXX")
trap 'rm -rf "$TMP"' EXIT
pass=0; fail=0
ok(){ printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad(){ printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }
APK="$TMP/x.apk"; printf 'apk-bytes\n' > "$APK"; SHA=$(sha256sum "$APK"|awk '{print $1}')
MOCK="$TMP/mock-gate.sh"
cat > "$MOCK" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
out=$3
mkdir -p "$out/install-chain/deep"
printf 'gate_status=PASS\nclaim_allowed=false\n' > "$out/termux-sealed-stdin-gate.status"
printf 'installer_exit=0\nclaim_allowed=false\n' > "$out/install-chain/deep/installer.status"
SH
chmod +x "$MOCK"
O="$TMP/out"
if APKC_PHYSICAL_GATE="$MOCK" bash "$WRAP" "$APK" "$SHA" "$O" probe-only >/dev/null && grep -Fq './install-chain/deep/installer.status' "$O/receipt-tree.sha256"; then ok nested_installer_evidence_bound; else bad nested_installer_evidence_bound; fi
printf 'installer_exit=99\n' > "$O/install-chain/deep/installer.status"
if (cd "$O" && sha256sum -c receipt-tree.sha256 >/dev/null 2>&1); then bad nested_postseal_tamper_rejected; else ok nested_postseal_tamper_rejected; fi
FAILG="$TMP/fail-gate.sh"; printf '#!/usr/bin/env bash\nexit 37\n' > "$FAILG"; chmod +x "$FAILG"
if APKC_PHYSICAL_GATE="$FAILG" bash "$WRAP" "$APK" "$SHA" "$TMP/out-fail" probe-only >/dev/null 2>&1; then bad physical_gate_failure_propagated; else rc=$?; [ "$rc" -eq 37 ] && ok physical_gate_failure_propagated || bad physical_gate_failure_propagated; fi
SYMG="$TMP/sym-gate.sh"; cat > "$SYMG" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
out=$3; mkdir -p "$out/deep"; printf x > "$out/deep/x"; ln -s deep/x "$out/link"
SH
chmod +x "$SYMG"
if APKC_PHYSICAL_GATE="$SYMG" bash "$WRAP" "$APK" "$SHA" "$TMP/out-sym" probe-only >/dev/null 2>&1; then bad symlink_tree_rejected; else ok symlink_tree_rejected; fi
if grep -Fxq 'claim_allowed=false' "$O/recursive-custody.status"; then ok claim_gate_preserved; else bad claim_gate_preserved; fi
printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
