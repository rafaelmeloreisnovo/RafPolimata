#!/usr/bin/env bash
# Validate the canonical full-chain Android proof output.
# Exit 0=observed scoped runtime evidence; 1=falsified; 2=TOKEN_VAZIO/incomplete.
set -u -o pipefail

OUT="${1:-proofs/run-arm64-full-chain/out}"
STATUS="$OUT/status.tsv"
MANIFEST="$OUT/manifest.json"
LOGCAT="$OUT/10_logcat_nativeactivity.txt"
ENVFILE="$OUT/00_env.txt"

for f in "$STATUS" "$MANIFEST" "$LOGCAT" "$ENVFILE"; do
  if [ ! -s "$f" ]; then
    echo "L2 TOKEN_VAZIO_EVIDENCE missing=$f"
    exit 2
  fi
done

require_status() {
  local gate="$1" state="$2"
  awk -F '\t' -v g="$gate" -v s="$state" '$1==g && $2==s {found=1} END{exit(found?0:1)}' "$STATUS"
}

for gate in runtime_hardening compile_apkc generate_apk android_install android_launch manifest; do
  if require_status "$gate" FAIL; then
    echo "L2 FAIL gate=$gate status=FAIL" >&2
    exit 1
  fi
  if ! require_status "$gate" PASS; then
    echo "L2 TOKEN_VAZIO_EVIDENCE gate=$gate pass_not_observed"
    exit 2
  fi
done

# The capture script deliberately records android_runtime=AUDIT. Promotion here
# requires semantic inspection of the captured log plus successful install/launch.
if grep -Eiq 'FATAL EXCEPTION|Fatal signal|dlopen failed|Unable to start activity|Process: .* has died|native crash|SIG(SEGV|ABRT|BUS|ILL)' "$LOGCAT"; then
  echo 'L2 FAIL fatal_runtime_marker_in_logcat' >&2
  grep -Ein 'FATAL EXCEPTION|Fatal signal|dlopen failed|Unable to start activity|Process: .* has died|native crash|SIG(SEGV|ABRT|BUS|ILL)' "$LOGCAT" | head -20 >&2 || true
  exit 1
fi

# Require custody metadata to be current and non-empty. Historical evidence is
# valid as history but does not silently prove the current checkout.
if ! command -v python3 >/dev/null 2>&1; then
  echo 'L2 TOKEN_VAZIO_TOOLCHAIN missing=python3_for_manifest_validation'
  exit 2
fi

CURRENT_HEAD="$(git rev-parse HEAD 2>/dev/null || true)"
python3 - "$MANIFEST" "$CURRENT_HEAD" <<'PY'
import json, sys
from pathlib import Path
p=Path(sys.argv[1])
head=sys.argv[2]
try:
    m=json.loads(p.read_text(encoding='utf-8'))
except Exception as e:
    print(f'L2 FAIL invalid_manifest_json={e}', file=sys.stderr)
    raise SystemExit(1)
required=['git_commit','git_branch','raw_apkc_sha256','runtime_hardened_apkc_sha256','source_sha256']
missing=[k for k in required if not m.get(k) or str(m.get(k)).startswith('TOKEN_VAZIO')]
if missing:
    print('L2 TOKEN_VAZIO_CUSTODY missing=' + ','.join(missing))
    raise SystemExit(2)
if head and m.get('git_commit') != head:
    print(f"L2 TOKEN_VAZIO_STALE_RECEIPT manifest_head={m.get('git_commit')} current_head={head}")
    raise SystemExit(2)
print(f"L2 manifest custody PASS git_commit={m.get('git_commit')}")
PY
rc=$?
[ $rc -eq 0 ] || exit $rc

printf 'L2 PASS_RUNTIME_NO_FATAL_IN_CAPTURE_SCOPE out=%s\n' "$OUT"
printf 'L2 scope=install_success+launch_success+logcat_semantic_no_fatal+same_commit_custody claim_allowed=false\n'
exit 0
