#!/usr/bin/env bash
# raf_source_to_binary_proof.sh — reproducible source→binary transcript (L1).
#
# Closes audit lacuna L1 ("Prova source→binary do executável apkc"): builds the
# apkc compiler from the *versioned* source at the current commit, on the current
# host, with a recorded toolchain, and emits a transcript that a third party can
# reproduce bit-for-bit (same commit + same clang → same sha256).
#
# What this proves (PASS) and what it does NOT (TOKEN_VAZIO):
#   PASS  — Apkc/apkc.c compiles+links to a valid AArch64 ELF and to an ARM32
#           relocatable object, from committed source, with logged provenance.
#   TZ    — running apkc to *generate an APK* (and the arm64-v8a .so inside it,
#           L4 on-device) requires an ARM runtime/Termux/qemu; not done here.
#
# Usage: bash tools/raf_source_to_binary_proof.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

OUT="Apkc/proofs/out"
mkdir -p "$OUT"
COMMIT="$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)"
SHORT="$(git rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
DATE_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
HOST_ARCH="$(uname -m)"
CLANGV="$(clang --version 2>/dev/null | head -1 || echo 'clang: TOKEN_VAZIO')"

A64_ELF="/tmp/apkc_a64.elf"
A64_OBJ="/tmp/apkc_a64.o"
A32_OBJ="/tmp/apkc_a32.o"
SRC="Apkc/apkc.c"
CC_BASE="clang -ffreestanding -nostdlib -nostdinc -I Apkc"

PASS=0; TZ=0
log() { printf '%s\n' "$*"; }

TRANSCRIPT="$OUT/apkc-compile.txt"
# Preserve prior evidence: append a new dated section, never overwrite history.
{
  echo ""
  echo "=================================================================="
  echo "REPRODUCIBLE SOURCE->BINARY TRANSCRIPT  (L1)  ${DATE_UTC}"
  echo "=================================================================="
  echo "commit:      ${COMMIT}"
  echo "host_arch:   ${HOST_ARCH}"
  echo "toolchain:   ${CLANGV}"
  echo "source:      ${SRC}"
} >> "$TRANSCRIPT"

# 1) AArch64 freestanding ELF (prefer a fully-linked executable via lld)
if $CC_BASE -target aarch64-linux-gnu -Wl,-e,_start -fuse-ld=lld "$SRC" -o "$A64_ELF" 2>/tmp/_l64.err; then
    SHA="$(sha256sum "$A64_ELF" | awk '{print $1}')"
    HDR="$(readelf -h "$A64_ELF" | grep -E 'Class|Data|Machine|Type|Entry' | sed 's/^/    /')"
    {
      echo ""
      echo "[A64-ELF] command: ${CC_BASE} -target aarch64-linux-gnu -Wl,-e,_start -fuse-ld=lld ${SRC} -o apkc_a64.elf"
      echo "[A64-ELF] sha256:  ${SHA}"
      echo "[A64-ELF] readelf:"
      echo "$HDR"
      echo "[A64-ELF] STATUS:  PASS (AArch64 ELF built from committed source)"
    } >> "$TRANSCRIPT"
    { echo "# apkc compiler binary — AArch64 ELF (source->binary L1), ${DATE_UTC}, commit ${SHORT}";
      readelf -h "$A64_ELF"; } > "$OUT/apkc-binary-arm64.txt"
    PASS=$((PASS+1))
else
    echo "[A64-ELF] STATUS:  TOKEN_VAZIO (link failed; see below)" >> "$TRANSCRIPT"
    head -3 /tmp/_l64.err >> "$TRANSCRIPT"
    # Fall back to a relocatable object so we still have a real artifact.
    if $CC_BASE -target aarch64-linux-gnu -c "$SRC" -o "$A64_OBJ" 2>/dev/null; then
        echo "[A64-OBJ] sha256:  $(sha256sum "$A64_OBJ" | awk '{print $1}') (REL object, PASS)" >> "$TRANSCRIPT"
        PASS=$((PASS+1))
    fi
    TZ=$((TZ+1))
fi

# 2) ARM32 relocatable object (ELF32 ARM) — structural proof of A32 codegen path
if $CC_BASE -target arm-linux-gnueabihf -c "$SRC" -o "$A32_OBJ" 2>/tmp/_a32.err; then
    SHA="$(sha256sum "$A32_OBJ" | awk '{print $1}')"
    HDR="$(readelf -h "$A32_OBJ" | grep -E 'Class|Data|Machine|Type' | sed 's/^/    /')"
    {
      echo ""
      echo "[A32-OBJ] command: ${CC_BASE} -target arm-linux-gnueabihf -c ${SRC} -o apkc_a32.o"
      echo "[A32-OBJ] sha256:  ${SHA}"
      echo "[A32-OBJ] readelf:"
      echo "$HDR"
      echo "[A32-OBJ] STATUS:  PASS (ELF32 ARM object built from committed source)"
    } >> "$TRANSCRIPT"
    { echo "# apkc compiler binary — ELF32 ARM object (L1), ${DATE_UTC}, commit ${SHORT}";
      readelf -h "$A32_OBJ"; } > "$OUT/apkc-binary-arm32.txt"
    PASS=$((PASS+1))
else
    echo "[A32-OBJ] STATUS:  TOKEN_VAZIO (compile failed)" >> "$TRANSCRIPT"
    TZ=$((TZ+1))
fi

# 3) The arm64-v8a .so INSIDE a generated APK (L4 on-device) is NOT produced here.
{
  echo ""
  echo "[APK-SO] STATUS:  TOKEN_VAZIO — generating the APK payload (lib/arm64-v8a/*.so)"
  echo "[APK-SO]          requires running apkc, which is freestanding ARM64 and"
  echo "[APK-SO]          cannot execute on a ${HOST_ARCH} host. Run on ARM/Termux:"
  echo "[APK-SO]            ./apkc Apkc/hello.s.txt -o hello.apk -both"
  echo "[APK-SO]            unzip -p hello.apk 'lib/arm64-v8a/*.so' | readelf -h -"
} >> "$TRANSCRIPT"
TZ=$((TZ+1))

log "source->binary proof: ${PASS} PASS, ${TZ} TOKEN_VAZIO"
log "transcript: ${TRANSCRIPT}"
exit 0
