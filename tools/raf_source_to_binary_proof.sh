#!/usr/bin/env bash
# raf_source_to_binary_proof.sh — reproducible, fail-closed source→binary proof.
#
# The canonical proof is promoted only when BOTH required targets are built,
# identified by an ELF reader and reproduced byte-for-byte. Failed or partial
# runs are preserved under Apkc/proofs/runs/ and never become a false PASS.
# Anti-retraction: raw Apkc/apkc.c is never compiled directly.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

ALLOW_PARTIAL=0
if [ "${1:-}" = "--allow-partial" ]; then
    ALLOW_PARTIAL=1
elif [ -n "${1:-}" ]; then
    printf 'usage: %s [--allow-partial]\n' "$0" >&2
    exit 2
fi

for cmd in git clang sha256sum cmp date uname mkdir cp head grep awk python3; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        printf 'source-to-binary proof: missing command: %s\n' "$cmd" >&2
        exit 127
    fi
done

if command -v readelf >/dev/null 2>&1; then
    ELF_READER=readelf
elif command -v llvm-readelf >/dev/null 2>&1; then
    ELF_READER=llvm-readelf
else
    printf 'source-to-binary proof: missing readelf or llvm-readelf\n' >&2
    exit 127
fi

OUT="Apkc/proofs/out"
RUNS="Apkc/proofs/runs"
ARCHIVE="Apkc/proofs/archive"
RUN_ID="$(date -u +%Y%m%dT%H%M%SZ)"
RUN_DIR="$RUNS/$RUN_ID/source-to-binary"
mkdir -p "$OUT" "$RUN_DIR" "$ARCHIVE"

COMMIT="$(git rev-parse HEAD 2>/dev/null)" || COMMIT="TOKEN_VAZIO"
SHORT="$(git rev-parse --short HEAD 2>/dev/null)" || SHORT="TOKEN_VAZIO"
DATE_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
HOST_ARCH="$(uname -m)"
CLANGV="$(clang --version 2>/dev/null | head -1)" || CLANGV="clang: TOKEN_VAZIO"
LLDV="$(ld.lld --version 2>/dev/null | head -1)" || LLDV="lld: TOKEN_VAZIO"
ELFV="$($ELF_READER --version 2>/dev/null | head -1)" || ELFV="$ELF_READER"

RAW_SRC="Apkc/apkc.c"
SRC="$RUN_DIR/apkc_hardened.c"
HARD_LOG="$RUN_DIR/source-cap-hardening.txt"
A64_1="$RUN_DIR/apkc-aarch64-run1.elf"
A64_2="$RUN_DIR/apkc-aarch64-run2.elf"
A32_1="$RUN_DIR/apkc-arm32-run1.o"
A32_2="$RUN_DIR/apkc-arm32-run2.o"
A64_ERR="$RUN_DIR/aarch64.stderr.txt"
A32_ERR="$RUN_DIR/arm32.stderr.txt"
A64_HDR="$RUN_DIR/readelf-aarch64.txt"
A32_HDR="$RUN_DIR/readelf-arm32.txt"
TRANSCRIPT="$RUN_DIR/apkc-compile.txt"
STATUS_JSON="$RUN_DIR/status.json"

# Mandatory hardening gate before any compiler invocation.
if [ ! -f scripts/patch_apkc_source_cap.py ] || [ ! -f tests/test_apkc_source_cap_patch.py ] || [ ! -f scripts/verify_apkc_source_cap_output.py ]; then
    printf 'source-to-binary proof: source-cap transformer/falsifier/verifier missing\n' >&2
    exit 1
fi
if ! python3 tests/test_apkc_source_cap_patch.py >"$HARD_LOG" 2>&1 ||
   ! python3 scripts/patch_apkc_source_cap.py "$RAW_SRC" "$SRC" >>"$HARD_LOG" 2>&1 ||
   ! python3 scripts/verify_apkc_source_cap_output.py "$SRC" >>"$HARD_LOG" 2>&1; then
    printf 'source-to-binary proof: source-cap hardening failed; see %s\n' "$HARD_LOG" >&2
    exit 1
fi
RAW_SHA="$(sha256sum "$RAW_SRC" | awk '{print $1}')"
HARD_SHA="$(sha256sum "$SRC" | awk '{print $1}')"
{
    printf 'raw_source_sha256=%s\n' "$RAW_SHA"
    printf 'hardened_source_sha256=%s\n' "$HARD_SHA"
    printf '%s\n' 'guard=source exceeds SRC_CAP'
    printf '%s\n' 'legacy_source_read_anchor_present=no'
    printf '%s\n' 'verification=exact_contract_not_global_substring'
} >>"$HARD_LOG"

COMMON=(clang -ffreestanding -fno-builtin -nostdlib -nostdinc -I Apkc)
A64_CMD=("${COMMON[@]}" --target=aarch64-linux-gnu -fuse-ld=lld \
         -Wl,-e,_start -Wl,--build-id=none "$SRC" -o "$A64_1")
A64_CMD_2=("${COMMON[@]}" --target=aarch64-linux-gnu -fuse-ld=lld \
           -Wl,-e,_start -Wl,--build-id=none "$SRC" -o "$A64_2")
A32_CMD=("${COMMON[@]}" --target=arm-linux-gnueabihf -c "$SRC" -o "$A32_1")
A32_CMD_2=("${COMMON[@]}" --target=arm-linux-gnueabihf -c "$SRC" -o "$A32_2")

A64_BUILD="TOKEN_VAZIO"
A64_IDENTITY="TOKEN_VAZIO"
A64_REPRO="TOKEN_VAZIO"
A32_BUILD="TOKEN_VAZIO"
A32_IDENTITY="TOKEN_VAZIO"
A32_REPRO="TOKEN_VAZIO"
A64_SHA="TOKEN_VAZIO"
A32_SHA="TOKEN_VAZIO"

print_cmd() {
    printf '  '
    printf '%q ' "$@"
    printf '\n'
}

{
    printf '==================================================================\n'
    printf 'REPRODUCIBLE SOURCE->BINARY TRANSCRIPT  %s\n' "$DATE_UTC"
    printf '==================================================================\n'
    printf 'run_id:      %s\n' "$RUN_ID"
    printf 'commit:      %s\n' "$COMMIT"
    printf 'host_arch:   %s\n' "$HOST_ARCH"
    printf 'toolchain:   %s\n' "$CLANGV"
    printf 'lld:         %s\n' "$LLDV"
    printf 'elf_reader:  %s\n' "$ELFV"
    printf 'raw_source:  %s\n' "$RAW_SRC"
    printf 'raw_sha256:  %s\n' "$RAW_SHA"
    printf 'source:      %s\n' "$SRC"
    printf 'source_sha:  %s\n\n' "$HARD_SHA"
    printf '[A64] command:\n'
    print_cmd "${A64_CMD[@]}"
} > "$TRANSCRIPT"

if "${A64_CMD[@]}" 2>"$A64_ERR"; then
    A64_BUILD="PASS"
    "$ELF_READER" -h "$A64_1" > "$A64_HDR"
    if grep -Eq 'Class:[[:space:]]+ELF64' "$A64_HDR" &&
       grep -Eq 'Machine:[[:space:]]+AArch64' "$A64_HDR"; then
        A64_IDENTITY="PASS"
    else
        A64_IDENTITY="FAIL"
    fi
    if "${A64_CMD_2[@]}" 2>>"$A64_ERR"; then
        A64_SHA="$(sha256sum "$A64_1" | awk '{print $1}')"
        A64_SHA_2="$(sha256sum "$A64_2" | awk '{print $1}')"
        if [ "$A64_SHA" = "$A64_SHA_2" ] && cmp -s "$A64_1" "$A64_2"; then
            A64_REPRO="PASS"
        else
            A64_REPRO="FAIL"
        fi
    else
        A64_REPRO="FAIL"
    fi
else
    A64_BUILD="FAIL"
fi

{
    printf '[A64] build=%s identity=%s reproducibility=%s sha256=%s\n' \
        "$A64_BUILD" "$A64_IDENTITY" "$A64_REPRO" "$A64_SHA"
    if [ -s "$A64_ERR" ]; then
        printf '[A64] stderr (first 20 lines):\n'
        head -20 "$A64_ERR"
    fi
    printf '\n[A32] command:\n'
    print_cmd "${A32_CMD[@]}"
} >> "$TRANSCRIPT"

if "${A32_CMD[@]}" 2>"$A32_ERR"; then
    A32_BUILD="PASS"
    "$ELF_READER" -h "$A32_1" > "$A32_HDR"
    if grep -Eq 'Class:[[:space:]]+ELF32' "$A32_HDR" &&
       grep -Eq 'Machine:[[:space:]]+ARM' "$A32_HDR"; then
        A32_IDENTITY="PASS"
    else
        A32_IDENTITY="FAIL"
    fi
    if "${A32_CMD_2[@]}" 2>>"$A32_ERR"; then
        A32_SHA="$(sha256sum "$A32_1" | awk '{print $1}')"
        A32_SHA_2="$(sha256sum "$A32_2" | awk '{print $1}')"
        if [ "$A32_SHA" = "$A32_SHA_2" ] && cmp -s "$A32_1" "$A32_2"; then
            A32_REPRO="PASS"
        else
            A32_REPRO="FAIL"
        fi
    else
        A32_REPRO="FAIL"
    fi
else
    A32_BUILD="FAIL"
fi

{
    printf '[A32] build=%s identity=%s reproducibility=%s sha256=%s\n' \
        "$A32_BUILD" "$A32_IDENTITY" "$A32_REPRO" "$A32_SHA"
    if [ -s "$A32_ERR" ]; then
        printf '[A32] stderr (first 20 lines):\n'
        head -20 "$A32_ERR"
    fi
} >> "$TRANSCRIPT"

COMPLETE=0
if [ "$A64_BUILD" = PASS ] && [ "$A64_IDENTITY" = PASS ] && [ "$A64_REPRO" = PASS ] &&
   [ "$A32_BUILD" = PASS ] && [ "$A32_IDENTITY" = PASS ] && [ "$A32_REPRO" = PASS ]; then
    COMPLETE=1
fi

if [ "$COMPLETE" -eq 1 ]; then
    FINAL_STATE="PASS"
else
    FINAL_STATE="FAIL"
fi

cat > "$STATUS_JSON" <<JSON
{
  "schema": "raf.apkc.source-to-binary-proof.v3",
  "run_id": "$RUN_ID",
  "date_utc": "$DATE_UTC",
  "commit": "$COMMIT",
  "host_arch": "$HOST_ARCH",
  "elf_reader": "$ELF_READER",
  "raw_source_sha256": "$RAW_SHA",
  "hardened_source_sha256": "$HARD_SHA",
  "source_cap_hardening": "PASS",
  "aarch64": {
    "build": "$A64_BUILD",
    "identity": "$A64_IDENTITY",
    "reproducibility": "$A64_REPRO",
    "sha256": "$A64_SHA"
  },
  "arm32": {
    "build": "$A32_BUILD",
    "identity": "$A32_IDENTITY",
    "reproducibility": "$A32_REPRO",
    "sha256": "$A32_SHA"
  },
  "apk_payload_runtime": "TOKEN_VAZIO",
  "state": "$FINAL_STATE",
  "claim_allowed": false
}
JSON

printf '\n[FINAL] state=%s claim_allowed=false\n' "$FINAL_STATE" >> "$TRANSCRIPT"
cp "$STATUS_JSON" "$OUT/apkc-compile.status.json"

if [ "$COMPLETE" -eq 1 ]; then
    if [ -s "$OUT/apkc-compile.txt" ]; then
        OLD_SHA="$(sha256sum "$OUT/apkc-compile.txt" | awk '{print $1}')"
        cp "$OUT/apkc-compile.txt" "$ARCHIVE/apkc-compile.pre-${RUN_ID}.${OLD_SHA}.txt"
    fi
    cp "$TRANSCRIPT" "$OUT/apkc-compile.txt"
    cp "$A64_HDR" "$OUT/apkc-binary-arm64.txt"
    cp "$A32_HDR" "$OUT/apkc-binary-arm32.txt"
    printf 'source->binary proof: PASS (%s)\n' "$RUN_DIR"
    exit 0
fi

cp "$TRANSCRIPT" "$OUT/apkc-compile.pending.txt"
printf 'source->binary proof: FAIL; canonical PASS was not promoted (%s)\n' "$RUN_DIR" >&2
if [ "$ALLOW_PARTIAL" -eq 1 ]; then
    exit 0
fi
exit 1
