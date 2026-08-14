#!/usr/bin/env bash
set -u
F=${1:-scripts/apkc_finalize_stdin_install_receipt.sh}
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
p=0; f=0; ok(){ echo PASS "$1";p=$((p+1));}; bad(){ echo FAIL "$1";f=$((f+1));}
D=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
mk(){ mkdir -p "$1"; cat >"$1/sealed-stdin.status" <<E
sealed_stdin_status=READY
transport_mode=SEALED_MEMFD_STDIN
claim_allowed=false
same_inode_inplace_protection=MITIGATED_BY_SEALED_SNAPSHOT
expected_sha256=$D
sealed_snapshot_sha256=$D
seals=0xf
apk_bytes=123
E
printf 'installer_exit=0\nclaim_allowed=false\n' >"$1/installer-exit.status"; }
mk "$T/a"; "$F" "$T/a"; [ $? -eq 0 ] && grep -qx overall_status=PASS "$T/a/apkc-stdin-install-chain.status" && ok positive || bad positive
mk "$T/b"; sed -i 's/SEALED_MEMFD_STDIN/FD_PATH/' "$T/b/sealed-stdin.status"; "$F" "$T/b" >/dev/null 2>&1; [ $? -eq 186 ] && ok wrong_transport || bad wrong_transport
mk "$T/c"; sed -i 's/apk_bytes=123/apk_bytes=0/' "$T/c/sealed-stdin.status"; "$F" "$T/c" >/dev/null 2>&1; [ $? -eq 187 ] && ok zero_size || bad zero_size
mk "$T/d"; printf 'installer_exit=23\nclaim_allowed=false\n' >"$T/d/installer-exit.status"; "$F" "$T/d" >/dev/null 2>&1; [ $? -eq 23 ] && ok installer_nonzero || bad installer_nonzero
mk "$T/e"; sed -i 's/seals=0xf/seals=0x7/' "$T/e/sealed-stdin.status"; "$F" "$T/e" >/dev/null 2>&1; [ $? -eq 187 ] && ok incomplete_seals || bad incomplete_seals
mk "$T/g"; rm "$T/g/installer-exit.status"; "$F" "$T/g" >/dev/null 2>&1; [ $? -eq 183 ] && ok missing_installer || bad missing_installer
if ! grep -Rqx claim_allowed=true "$T"; then ok claim_gate; else bad claim_gate; fi
echo RESULT pass=$p fail=$f claim_allowed=false; [ $f -eq 0 ]
