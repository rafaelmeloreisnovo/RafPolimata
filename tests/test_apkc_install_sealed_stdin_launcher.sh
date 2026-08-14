#!/usr/bin/env bash
set -u
LAUNCH=${1:-scripts/apkc_install_sealed_stdin.sh}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
p=0; f=0; ok(){ echo PASS "$1";p=$((p+1));}; bad(){ echo FAIL "$1";f=$((f+1));}
mkdir -p "$T/root/scripts"; cp "${APKC_TEST_HELPER_SRC:-scripts/apkc_install_sealed_stdin.c}" "$T/root/scripts/"; cp "${APKC_TEST_FINALIZER:-scripts/apkc_finalize_stdin_install_receipt.sh}" "$T/root/scripts/"
printf 'LAUNCHER-APK-BYTES\n' >"$T/app.apk"; d=$(sha256sum "$T/app.apk"|awk '{print $1}')
cat >"$T/adb" <<'E'
#!/bin/sh
set -eu
[ "$1" = shell ] && [ "$2" = pm ] && [ "$3" = install ] && [ "$4" = -S ] && [ "$6" = - ]
size=$5; cat >"$MOCK_CAPTURE"; [ "$(wc -c <"$MOCK_CAPTURE"|tr -d ' ')" = "$size" ]
E
chmod +x "$T/adb"
MOCK_CAPTURE="$T/cap.apk" APKC_ROOT="$T/root" APKC_ADB_BIN="$T/adb" "$LAUNCH" "$T/app.apk" "$d" "$T/out1" adb-shell-pm
if [ $? -eq 0 ] && cmp -s "$T/app.apk" "$T/cap.apk" && grep -qx overall_status=PASS "$T/out1/apkc-stdin-install-chain.status"; then ok adb_shell_pm_exact; else bad adb_shell_pm_exact; fi
APKC_ROOT="$T/root" APKC_ADB_BIN="$T/no-adb" "$LAUNCH" "$T/app.apk" "$d" "$T/out2" adb-shell-pm >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 198 ] && grep -qx reason=ADB_MISSING "$T/out2/sealed-stdin-launcher.status"; then ok missing_adb_fail_closed; else bad missing_adb_fail_closed; fi
APKC_ROOT="$T/root" "$LAUNCH" "$T/app.apk" "$d" "$T/out3" pathname-fallback >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 200 ] && grep -qx reason=UNKNOWN_MODE "$T/out3/sealed-stdin-launcher.status"; then ok no_pathname_fallback; else bad no_pathname_fallback; fi
if ! grep -Rqx claim_allowed=true "$T" 2>/dev/null; then ok claim_gate; else bad claim_gate; fi
echo RESULT pass=$p fail=$f claim_allowed=false; [ $f -eq 0 ]
