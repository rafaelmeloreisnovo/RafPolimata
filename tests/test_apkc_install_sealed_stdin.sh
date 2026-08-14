#!/usr/bin/env bash
set -u
BIN=${1:-./apkc_install_sealed_stdin}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
pass=0; fail=0
ok(){ printf 'PASS %s\n' "$1"; pass=$((pass+1)); }
bad(){ printf 'FAIL %s\n' "$1"; fail=$((fail+1)); }
mk_apk(){ printf 'APK-SEALED-STDIN-CONTENT\n' > "$1"; }
sha(){ sha256sum "$1" | awk '{print $1}'; }

cat > "$TMP/mock_ok.sh" <<'EOM'
#!/bin/sh
set -eu
[ "$1" = install ] && [ "$2" = -S ] && [ "$4" = - ]
size=$3
cat > "$MOCK_CAPTURE"
[ "$(wc -c < "$MOCK_CAPTURE" | tr -d ' ')" = "$size" ]
exit 0
EOM
chmod +x "$TMP/mock_ok.sh"

mkdir "$TMP/t1"; mk_apk "$TMP/t1/app.apk"; d=$(sha "$TMP/t1/app.apk")
MOCK_CAPTURE="$TMP/t1/captured.apk" "$BIN" "$TMP/t1/app.apk" "$d" "$TMP/t1/out" -- "$TMP/mock_ok.sh" install -S @APK_SIZE@ @APK_STDIN@
rc=$?
if [ "$rc" -eq 0 ] && [ "$(sha "$TMP/t1/captured.apk")" = "$d" ] && grep -qx 'transport_mode=SEALED_MEMFD_STDIN' "$TMP/t1/out/sealed-stdin.status"; then ok positive_exact_stdin; else bad positive_exact_stdin; fi

mkdir "$TMP/t2"; mk_apk "$TMP/t2/app.apk"; d=$(sha "$TMP/t2/app.apk")
cat > "$TMP/mock_delay.sh" <<'EOM'
#!/bin/sh
set -eu
[ "$1" = install ] && [ "$2" = -S ] && [ "$4" = - ]
size=$3
sleep 1
cat > "$MOCK_CAPTURE"
[ "$(wc -c < "$MOCK_CAPTURE" | tr -d ' ')" = "$size" ]
exit 0
EOM
chmod +x "$TMP/mock_delay.sh"
MOCK_CAPTURE="$TMP/t2/captured.apk" "$BIN" "$TMP/t2/app.apk" "$d" "$TMP/t2/out" -- "$TMP/mock_delay.sh" install -S @APK_SIZE@ @APK_STDIN@ & pid=$!
for _ in $(seq 1 100); do [ -f "$TMP/t2/out/sealed-stdin.status" ] && grep -q '^sealed_stdin_status=READY$' "$TMP/t2/out/sealed-stdin.status" && break; sleep 0.01; done
printf 'MUTATED-SAME-INODE-AFTER-READY\n' > "$TMP/t2/app.apk"
wait "$pid"; rc=$?
if [ "$rc" -eq 0 ] && [ "$(sha "$TMP/t2/captured.apk")" = "$d" ] && [ "$(sha "$TMP/t2/app.apk")" != "$d" ]; then ok same_inode_mutation_isolated; else bad same_inode_mutation_isolated; fi

mkdir "$TMP/t3"; mk_apk "$TMP/t3/app.apk"; d=$(printf x | sha256sum | awk '{print $1}')
MOCK_CAPTURE="$TMP/t3/captured.apk" "$BIN" "$TMP/t3/app.apk" "$d" "$TMP/t3/out" -- "$TMP/mock_ok.sh" install -S @APK_SIZE@ @APK_STDIN@ >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 170 ] && [ ! -e "$TMP/t3/captured.apk" ]; then ok digest_mismatch_blocks; else bad digest_mismatch_blocks; fi

mkdir "$TMP/t4"; mk_apk "$TMP/t4/app.apk"; d=$(sha "$TMP/t4/app.apk")
"$BIN" "$TMP/t4/app.apk" "$d" "$TMP/t4/out" -- "$TMP/mock_ok.sh" install -S @APK_SIZE@ - >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 172 ]; then ok missing_stdin_placeholder_rejected; else bad missing_stdin_placeholder_rejected; fi

mkdir "$TMP/t5"; mk_apk "$TMP/t5/app.apk"; d=$(sha "$TMP/t5/app.apk")
cat > "$TMP/mock_fail.sh" <<'EOM'
#!/bin/sh
cat >/dev/null
exit 23
EOM
chmod +x "$TMP/mock_fail.sh"
"$BIN" "$TMP/t5/app.apk" "$d" "$TMP/t5/out" -- "$TMP/mock_fail.sh" @APK_SIZE@ @APK_STDIN@ >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 23 ] && grep -qx 'installer_exit=23' "$TMP/t5/out/installer-exit.status"; then ok installer_nonzero_propagated; else bad installer_nonzero_propagated; fi

if ! grep -Rqx 'claim_allowed=true' "$TMP" 2>/dev/null; then ok claim_gate_preserved; else bad claim_gate_preserved; fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$pass" "$fail"
[ "$fail" -eq 0 ]
