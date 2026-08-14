#!/usr/bin/env bash
set -u
GATE=${1:?gate}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
printf 'apk-payload\n' > "$T/x.apk"; SHA=$(sha256sum "$T/x.apk"|awk '{print $1}')
cat > "$T/probe.c" <<'C'
#include <stdio.h>
int main(void){puts("runtime_probe=PASS");puts("claim_allowed=false");return 0;}
C
cat > "$T/probe_badclaim.c" <<'C'
#include <stdio.h>
int main(void){puts("runtime_probe=PASS");return 0;}
C
cat > "$T/probe_fail.c" <<'C'
int main(void){return 9;}
C
cat > "$T/launcher.sh" <<'SH'
#!/usr/bin/env bash
mkdir -p "$3"; printf 'mock=pass\nclaim_allowed=false\n' > "$3/mock.status"; exit "${MOCK_INSTALL_RC:-0}"
SH
chmod +x "$T/launcher.sh"
cat > "$T/adb" <<'SH'
#!/usr/bin/env bash
case "${1:-}" in
 get-state) printf '%s\n' "${MOCK_ADB_STATE:-device}"; exit "${MOCK_ADB_STATE_RC:-0}";;
 get-serialno) printf 'SERIAL-TEST\n';;
 shell) shift; if [ "${1:-}" = getprop ]; then case "${2:-}" in ro.build.version.sdk) echo 35;; ro.product.cpu.abi) echo armeabi-v7a;; ro.build.fingerprint) echo test/fingerprint;; *) echo TOKEN_VAZIO;; esac; else exit 0; fi;;
 *) exit 0;;
esac
SH
chmod +x "$T/adb"
P=0; N=0
run(){ name=$1 exp=$2; shift 2; set +e; "$@" >/dev/null 2>&1; rc=$?; set -e; if [ "$exp" = 0 ]; then ok=$([ "$rc" -eq 0 ] && echo 1 || echo 0); else ok=$([ "$rc" -ne 0 ] && echo 1 || echo 0); fi; if [ "$ok" = 1 ]; then echo "PASS $name rc=$rc"; P=$((P+1)); else echo "FAIL $name rc=$rc"; N=$((N+1)); fi; }
run probe_only 0 env APKC_MEMFD_PROBE_SRC="$T/probe.c" bash "$GATE" "$T/x.apk" "$SHA" "$T/o1" probe-only
run digest_mismatch 1 env APKC_MEMFD_PROBE_SRC="$T/probe.c" bash "$GATE" "$T/x.apk" "$(printf '0%.0s' {1..64})" "$T/o2" probe-only
run probe_claim_missing 1 env APKC_MEMFD_PROBE_SRC="$T/probe_badclaim.c" bash "$GATE" "$T/x.apk" "$SHA" "$T/o3" probe-only
run probe_runtime_fail 1 env APKC_MEMFD_PROBE_SRC="$T/probe_fail.c" bash "$GATE" "$T/x.apk" "$SHA" "$T/o4" probe-only
run unknown_mode 1 env APKC_MEMFD_PROBE_SRC="$T/probe.c" bash "$GATE" "$T/x.apk" "$SHA" "$T/o5" nonsense
run adb_not_ready 1 env APKC_MEMFD_PROBE_SRC="$T/probe.c" APKC_SEALED_STDIN_LAUNCHER="$T/launcher.sh" APKC_ADB_BIN="$T/adb" MOCK_ADB_STATE=offline bash "$GATE" "$T/x.apk" "$SHA" "$T/o6" adb-shell-pm
run adb_install_mock_pass 0 env APKC_MEMFD_PROBE_SRC="$T/probe.c" APKC_SEALED_STDIN_LAUNCHER="$T/launcher.sh" APKC_ADB_BIN="$T/adb" bash "$GATE" "$T/x.apk" "$SHA" "$T/o7" adb-shell-pm
run install_mock_fail 1 env APKC_MEMFD_PROBE_SRC="$T/probe.c" APKC_SEALED_STDIN_LAUNCHER="$T/launcher.sh" APKC_ADB_BIN="$T/adb" MOCK_INSTALL_RC=23 bash "$GATE" "$T/x.apk" "$SHA" "$T/o8" adb-shell-pm
grep -q '^final_result=PASS$' "$T/o7/receipt-verify.txt" || { echo 'FAIL receipt_positive'; N=$((N+1)); }
! grep -R -Fq 'SERIAL-TEST' "$T/o7" || { echo 'FAIL raw_serial_leak'; N=$((N+1)); }
if grep -q '^final_result=PASS$' "$T/o7/receipt-verify.txt" && ! grep -R -Fq 'SERIAL-TEST' "$T/o7"; then echo 'PASS receipt_and_serial_pseudonymization'; P=$((P+1)); fi
printf 'RESULT pass=%s fail=%s claim_allowed=false\n' "$P" "$N"
exit $([ "$N" -eq 0 ] && echo 0 || echo 1)
