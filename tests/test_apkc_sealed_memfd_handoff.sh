#!/usr/bin/env bash
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/scripts/apkc_install_sealed_memfd_gate.sh"
TMP="${TMPDIR:-/tmp}/apkc-sealed-memfd-test.$$"
PASS=0
FAIL=0
mkdir -p "$TMP"
trap 'rm -rf "$TMP"' EXIT

ok(){ PASS=$((PASS+1)); printf 'PASS %s\n' "$1"; }
bad(){ FAIL=$((FAIL+1)); printf 'FAIL %s\n' "$1"; }

[ -f "$GATE" ] || { printf 'missing sealed gate\n' >&2; exit 1; }
command -v sha256sum >/dev/null 2>&1 || { printf 'sha256sum missing\n' >&2; exit 1; }

cat > "$TMP/mock-installer.sh" <<'EOF'
#!/usr/bin/env bash
set -u
fd=${1:?}
orig=${APKC_TEST_ORIGINAL_PATH:?}
capture=${APKC_TEST_CAPTURE:?}
called=${APKC_TEST_INSTALLER_CALLED:?}
case "${APKC_TEST_MODE:-consume}" in
  mutate_source)
    printf 'MUTATED_IN_PLACE_AFTER_SEAL\n' > "$orig"
    ;;
  mutate_fd)
    if printf 'WRITE_ATTEMPT\n' > "$fd" 2>/dev/null; then
      printf 'fd_write_unexpectedly_succeeded\n' >&2
      exit 70
    fi
    ;;
esac
cat "$fd" > "$capture"
touch "$called"
EOF
chmod +x "$TMP/mock-installer.sh"

# 1. Mutation of the original inode after the sealed snapshot is created must
# not change the bytes consumed by the installer.
APK1="$TMP/app1.apk"
printf 'ORIGINAL_SEALED_BYTES\n' > "$APK1"
SHA1="$(sha256sum "$APK1" | awk '{print $1}')"
CAP1="$TMP/cap1.bin"
CALLED1="$TMP/called1"
OUT1="$TMP/out1"
mkdir -p "$OUT1"
set +e
APKC_TEST_ORIGINAL_PATH="$APK1" APKC_TEST_CAPTURE="$CAP1" APKC_TEST_INSTALLER_CALLED="$CALLED1" APKC_TEST_MODE=mutate_source \
  bash "$GATE" "$APK1" "$SHA1" "$OUT1" -- "$TMP/mock-installer.sh" @APK_FD@
RC=$?
set -e
if [ "$RC" -eq 0 ] && [ -f "$CALLED1" ] && \
   [ "$(sha256sum "$CAP1" | awk '{print $1}')" = "$SHA1" ] && \
   [ "$(sha256sum "$APK1" | awk '{print $1}')" != "$SHA1" ] && \
   grep -q '^same_inode_inplace_protection=MITIGATED_BY_SEALED_SNAPSHOT$' "$OUT1/sealed-memfd.status"; then
  ok same_inode_source_mutation_isolated
else
  bad same_inode_source_mutation_isolated
fi

# 2. The sealed memfd itself must reject writes through its proc-fd path.
APK2="$TMP/app2.apk"
printf 'SEALED_WRITE_PROBE\n' > "$APK2"
SHA2="$(sha256sum "$APK2" | awk '{print $1}')"
CAP2="$TMP/cap2.bin"
CALLED2="$TMP/called2"
OUT2="$TMP/out2"
mkdir -p "$OUT2"
set +e
APKC_TEST_ORIGINAL_PATH="$APK2" APKC_TEST_CAPTURE="$CAP2" APKC_TEST_INSTALLER_CALLED="$CALLED2" APKC_TEST_MODE=mutate_fd \
  bash "$GATE" "$APK2" "$SHA2" "$OUT2" -- "$TMP/mock-installer.sh" @APK_FD@
RC=$?
set -e
if [ "$RC" -eq 0 ] && [ -f "$CALLED2" ] && [ "$(sha256sum "$CAP2" | awk '{print $1}')" = "$SHA2" ] && grep -q '^seals=0xf$' "$OUT2/sealed-memfd.status"; then
  ok sealed_memfd_rejects_write
else
  bad sealed_memfd_rejects_write
fi

# 3. Mutation before snapshot creation must fail closed against the already
# frozen expected digest and never call the installer.
APK3="$TMP/app3.apk"
printf 'FROZEN_BEFORE_MUTATION\n' > "$APK3"
SHA3="$(sha256sum "$APK3" | awk '{print $1}')"
printf 'MUTATED_BEFORE_GATE\n' > "$APK3"
CAP3="$TMP/cap3.bin"
CALLED3="$TMP/called3"
OUT3="$TMP/out3"
mkdir -p "$OUT3"
set +e
APKC_TEST_ORIGINAL_PATH="$APK3" APKC_TEST_CAPTURE="$CAP3" APKC_TEST_INSTALLER_CALLED="$CALLED3" APKC_TEST_MODE=consume \
  bash "$GATE" "$APK3" "$SHA3" "$OUT3" -- "$TMP/mock-installer.sh" @APK_FD@
RC=$?
set -e
if [ "$RC" -eq 126 ] && [ ! -e "$CALLED3" ] && grep -q '^sealed_snapshot_status=FAIL$' "$OUT3/sealed-memfd.status"; then
  ok pre_snapshot_mutation_blocks_installer
else
  bad pre_snapshot_mutation_blocks_installer
fi

# 4. Placeholder multiplicity remains fail-closed.
APK4="$TMP/app4.apk"
printf 'PLACEHOLDER_GUARD\n' > "$APK4"
SHA4="$(sha256sum "$APK4" | awk '{print $1}')"
OUT4="$TMP/out4"
mkdir -p "$OUT4"
set +e
bash "$GATE" "$APK4" "$SHA4" "$OUT4" -- /bin/true @APK_FD@ @APK_FD@
RC=$?
set -e
if [ "$RC" -eq 128 ]; then
  ok duplicate_placeholder_rejected
else
  bad duplicate_placeholder_rejected
fi

printf 'RESULT pass=%d fail=%d claim_allowed=false\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
