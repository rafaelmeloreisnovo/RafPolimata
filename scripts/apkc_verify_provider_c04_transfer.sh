#!/usr/bin/env bash
set -euo pipefail
ARTIFACT=${1:-}
OUT_DIR=${2:-}
MODE=${3:-extract-only}
[ -n "$ARTIFACT" ] && [ -n "$OUT_DIR" ] || { echo "usage: $0 ARTIFACT_ZIP OUT_DIR [extract-only|probe-only|adb-shell-pm|pm-local]" >&2; exit 241; }
EXPECTED_ARTIFACT_SHA=5b1666efbb9f7be69b6db900287e10aab4caea476ee95462c5797ca6d336aa94
EXPECTED_APK_SHA=7957e4421921da85924cb278fce38db8d6ba747c99ecb9eb7e04edfbe42e9ff3
EXPECTED_MEMBER='Apkc/proofs/out/hello.apk'
PROVIDER_RUN=31812435821
PROVIDER_HEAD=8b46572c2fe3ad8610ca543ebdc2fd7b2b06ffea
PROVIDER_MERGE=3980c48a759dcd4ad469a7237df74e257e2a06e3
ROOT=${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." 2>/dev/null && pwd || pwd)}
GATE=${APKC_TERMUX_GATE:-$ROOT/scripts/apkc_termux_sealed_stdin_gate.sh}
mkdir -p "$OUT_DIR"
STATUS="$OUT_DIR/provider-c04-transfer.status"
write_status(){ printf 'transfer_status=%s\nmode=%s\nreason=%s\nclaim_allowed=false\n' "$1" "$MODE" "$2" > "$STATUS"; }
command -v sha256sum >/dev/null 2>&1 || { write_status FAIL SHA256SUM_MISSING; exit 242; }
command -v unzip >/dev/null 2>&1 || { write_status FAIL UNZIP_MISSING; exit 243; }
[ -f "$ARTIFACT" ] && [ -s "$ARTIFACT" ] || { write_status FAIL ARTIFACT_MISSING_OR_EMPTY; exit 244; }
actual_artifact_sha=$(sha256sum "$ARTIFACT" | awk '{print $1}')
printf 'expected_artifact_sha256=%s\nactual_artifact_sha256=%s\nmatch=%s\nclaim_allowed=false\n' "$EXPECTED_ARTIFACT_SHA" "$actual_artifact_sha" "$([ "$actual_artifact_sha" = "$EXPECTED_ARTIFACT_SHA" ] && echo true || echo false)" > "$OUT_DIR/artifact-digest.status"
[ "$actual_artifact_sha" = "$EXPECTED_ARTIFACT_SHA" ] || { write_status FAIL ARTIFACT_DIGEST_MISMATCH; exit 245; }
count=$(unzip -Z1 "$ARTIFACT" | awk -v x="$EXPECTED_MEMBER" '$0==x{n++} END{print n+0}')
[ "$count" -eq 1 ] || { write_status FAIL APK_MEMBER_CARDINALITY_NOT_ONE; exit 246; }
tmp=$(mktemp "${TMPDIR:-/tmp}/apkc-c04-apk.XXXXXX") || { write_status FAIL TMP_CREATE_FAILED; exit 247; }
trap 'rm -f "$tmp"' EXIT
unzip -p "$ARTIFACT" "$EXPECTED_MEMBER" > "$tmp" || { write_status FAIL APK_MEMBER_EXTRACT_FAILED; exit 248; }
[ -s "$tmp" ] || { write_status FAIL APK_MEMBER_EMPTY; exit 249; }
actual_apk_sha=$(sha256sum "$tmp" | awk '{print $1}')
printf 'expected_apk_sha256=%s\nactual_apk_sha256=%s\nmatch=%s\nmember=%s\nclaim_allowed=false\n' "$EXPECTED_APK_SHA" "$actual_apk_sha" "$([ "$actual_apk_sha" = "$EXPECTED_APK_SHA" ] && echo true || echo false)" "$EXPECTED_MEMBER" > "$OUT_DIR/apk-digest.status"
[ "$actual_apk_sha" = "$EXPECTED_APK_SHA" ] || { write_status FAIL APK_DIGEST_MISMATCH; exit 250; }
APK_OUT="$OUT_DIR/hello.c04.provider.apk"
cp "$tmp" "$APK_OUT" || { write_status FAIL APK_COPY_FAILED; exit 251; }
printf 'provider_run=%s\nprovider_head=%s\nprovider_merge=%s\nartifact_sha256=%s\napk_sha256=%s\nstructural_scope=PASS_STRUCTURAL\nruntime_scope=TOKEN_VAZIO\nclaim_allowed=false\n' "$PROVIDER_RUN" "$PROVIDER_HEAD" "$PROVIDER_MERGE" "$EXPECTED_ARTIFACT_SHA" "$EXPECTED_APK_SHA" > "$OUT_DIR/custody.status"
case "$MODE" in
 extract-only)
   printf 'physical_probe_attempted=false\nphysical_install_attempted=false\nruntime_result=TOKEN_VAZIO_NOT_ATTEMPTED\nclaim_allowed=false\n' > "$OUT_DIR/runtime-boundary.status"
   write_status PASS BYTE_EXACT_PROVIDER_APK_EXTRACTED
   ;;
 probe-only|adb-shell-pm|pm-local)
   [ -f "$GATE" ] || { write_status FAIL PHYSICAL_GATE_MISSING; exit 252; }
   set +e
   bash "$GATE" "$APK_OUT" "$EXPECTED_APK_SHA" "$OUT_DIR/physical" "$MODE" >"$OUT_DIR/physical-gate.stdout" 2>"$OUT_DIR/physical-gate.stderr"
   rc=$?
   set -e
   printf 'physical_gate_exit=%s\nclaim_allowed=false\n' "$rc" > "$OUT_DIR/physical-gate.status"
   [ "$rc" -eq 0 ] || { write_status FAIL PHYSICAL_GATE_REJECTED; exit "$rc"; }
   write_status PASS BYTE_EXACT_PROVIDER_APK_PHYSICAL_GATE_PASS
   ;;
 *) write_status FAIL UNKNOWN_MODE; exit 253;;
esac
(
 cd "$OUT_DIR"
 find . -maxdepth 1 -type f ! -name 'receipt.sha256' ! -name 'receipt-verify.txt' -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$OUT_DIR/receipt.sha256"
(cd "$OUT_DIR" && sha256sum -c receipt.sha256 > receipt-verify.txt 2>&1) || { write_status FAIL RECEIPT_VERIFY_FAILED; exit 254; }
printf 'final_result=PASS\nclaim_allowed=false\n' >> "$OUT_DIR/receipt-verify.txt"
