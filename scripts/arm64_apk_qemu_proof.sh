#!/usr/bin/env bash
# arm64_apk_qemu_proof.sh — ARM64 APK generation proof (L4).
#
# Builds ApkC from the generated runtime-hardened translation unit, runs it
# under qemu-aarch64-static, validates the APK/native ELF/DEX/signature chain,
# and falsifies the bounded source-input contract at the exact 1 MiB boundary.
# Raw Apkc/apkc.c is never passed directly to a compiler by this proof.
#
# Prerequisites (auto-checked): python3, clang, qemu-aarch64-static,
# apksigner, zipalign. Optional: aapt.
#
# Usage: bash scripts/arm64_apk_qemu_proof.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

OUT="Apkc/proofs/out"
mkdir -p "$OUT"
DATE_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
COMMIT="$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)"
SHORT="$(git rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
TRANSCRIPT="$OUT/arm64-apk-proof.txt"

CLANGV="$(clang --version 2>/dev/null | head -1 || echo TOKEN_VAZIO)"
QEMUV="$(qemu-aarch64-static --version 2>/dev/null | head -1 || echo TOKEN_VAZIO)"
SIGNV="$(apksigner --version 2>/dev/null || echo TOKEN_VAZIO)"

PASS=0; TZ=0; DEX_PROVEN=0; AAPT_PROVEN=0; BOUNDARY_PROVEN=0
log() { printf '%s\n' "$*"; }

# Truncate transcript at the start of each run so prior runs don't pollute evidence.
{
  echo ""
  echo "=================================================================="
  echo "ARM64 APK QEMU PROOF  (L4)  ${DATE_UTC}"
  echo "=================================================================="
  echo "commit:      ${COMMIT}"
  echo "clang:       ${CLANGV}"
  echo "qemu:        ${QEMUV}"
  echo "apksigner:   ${SIGNV}"
  echo "claim_allowed: false"
} > "$TRANSCRIPT"

# ── Stage 0: mandatory runtime-hardened translation unit ──────────────────
RUNTIME_SRC="/tmp/apkc.runtime-hardened.qemu.c"
RUNTIME_MANIFEST="$OUT/apkc-runtime-source-hardening-qemu.json"
RUNTIME_LOG="$OUT/apkc-runtime-source-hardening-qemu.txt"
rm -f "$RUNTIME_SRC" "$RUNTIME_MANIFEST" "$RUNTIME_LOG"

if ! command -v python3 >/dev/null 2>&1; then
    echo "[RUNTIME-HARDENING] STATUS: FAIL (python3 absent; refusing raw-source compile)" >> "$TRANSCRIPT"
    exit 1
fi
if [ ! -f scripts/patch_apkc_runtime_source.py ]; then
    echo "[RUNTIME-HARDENING] STATUS: FAIL (transformer missing)" >> "$TRANSCRIPT"
    exit 1
fi
if python3 scripts/patch_apkc_runtime_source.py \
      --input Apkc/apkc.c \
      --output "$RUNTIME_SRC" \
      --write-manifest "$RUNTIME_MANIFEST" >"$RUNTIME_LOG" 2>&1 \
   && [ -s "$RUNTIME_SRC" ] \
   && [ -s "$RUNTIME_MANIFEST" ] \
   && grep -q 'source exceeds 1MiB bounded input' "$RUNTIME_SRC" \
   && grep -q 'APKC-RH-013' "$RUNTIME_MANIFEST" \
   && ! grep -Fq 'if (n<=0) break;' "$RUNTIME_SRC"; then
    RAW_SHA="$(sha256sum Apkc/apkc.c | awk '{print $1}')"
    RUNTIME_SHA="$(sha256sum "$RUNTIME_SRC" | awk '{print $1}')"
    {
      echo ""
      echo "[RUNTIME-HARDENING] raw_source: Apkc/apkc.c"
      echo "[RUNTIME-HARDENING] raw_sha256: ${RAW_SHA}"
      echo "[RUNTIME-HARDENING] generated: ${RUNTIME_SRC}"
      echo "[RUNTIME-HARDENING] generated_sha256: ${RUNTIME_SHA}"
      echo "[RUNTIME-HARDENING] manifest: ${RUNTIME_MANIFEST}"
      echo "[RUNTIME-HARDENING] bounded-source change: APKC-RH-013"
      echo "[RUNTIME-HARDENING] STATUS: PASS (exact-anchor runtime hardening established)"
    } >> "$TRANSCRIPT"
    PASS=$((PASS+1))
else
    echo "[RUNTIME-HARDENING] STATUS: FAIL (transform/manifest/marker gate)" >> "$TRANSCRIPT"
    head -20 "$RUNTIME_LOG" >> "$TRANSCRIPT" 2>/dev/null || true
    exit 1
fi

# ── Stage 1: cross-compile hardened ApkC as static AArch64 ELF ────────────
APKC_ELF="/tmp/apkc_a64_qemu.elf"
rm -f "$APKC_ELF" /tmp/_xcompile.err
if clang -ffreestanding -nostdlib -nostdinc -I Apkc \
     -target aarch64-linux-gnu -static -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none \
     -O2 "$RUNTIME_SRC" -o "$APKC_ELF" 2>/tmp/_xcompile.err; then
    SHA_APKC="$(sha256sum "$APKC_ELF" | awk '{print $1}')"
    {
      echo ""
      echo "[CROSS-COMPILE] input_sha256: ${RUNTIME_SHA}"
      echo "[CROSS-COMPILE] command: clang -ffreestanding -nostdlib -nostdinc -I Apkc -target aarch64-linux-gnu -static -fuse-ld=lld -Wl,-e,_start -Wl,--build-id=none -O2 <runtime-hardened.c>"
      echo "[CROSS-COMPILE] output:  $(wc -c < "$APKC_ELF" | tr -d ' ') bytes"
      echo "[CROSS-COMPILE] sha256:  ${SHA_APKC}"
      echo "[CROSS-COMPILE] STATUS:  PASS (AArch64 static ELF from runtime-hardened TU)"
    } >> "$TRANSCRIPT"
    PASS=$((PASS+1))
else
    echo "[CROSS-COMPILE] STATUS: FAIL" >> "$TRANSCRIPT"
    head -20 /tmp/_xcompile.err >> "$TRANSCRIPT" 2>/dev/null || true
    log "FAIL: hardened cross-compile step"; exit 1
fi

# ── Stage 2: run hardened ApkC under qemu-aarch64-static ──────────────────
APK_OUT="/tmp/hello-arm64-proof.apk"
rm -f "$APK_OUT"
if command -v qemu-aarch64-static >/dev/null 2>&1; then
    APKC_STDOUT="$(qemu-aarch64-static "$APKC_ELF" Apkc/hello.s.txt -o "$APK_OUT" -64 2>&1)"
    APKC_EXIT=$?
    APK_SHA="$(sha256sum "$APK_OUT" 2>/dev/null | awk '{print $1}')"
    {
      echo ""
      echo "[QEMU-RUN] command: qemu-aarch64-static <hardened-apkc> Apkc/hello.s.txt -o hello-arm64-proof.apk -64"
      echo "[QEMU-RUN] stdout:  ${APKC_STDOUT}"
      if [ $APKC_EXIT -eq 0 ] && [ -s "$APK_OUT" ]; then
          echo "[QEMU-RUN] apk_size: $(wc -c < "$APK_OUT" | tr -d ' ') bytes"
          echo "[QEMU-RUN] sha256:   ${APK_SHA}"
          echo "[QEMU-RUN] STATUS:   PASS (APK generated by runtime-hardened ApkC via qemu)"
      else
          echo "[QEMU-RUN] STATUS:   FAIL (exit ${APKC_EXIT}; APK missing/empty)"
      fi
    } >> "$TRANSCRIPT"
    if [ $APKC_EXIT -eq 0 ] && [ -s "$APK_OUT" ]; then
        PASS=$((PASS+1))
        unzip -l "$APK_OUT" > "$OUT/unzip.txt" 2>/dev/null || true
        if unzip -p "$APK_OUT" classes.dex > /tmp/_apkc_proof_dex.bin 2>/dev/null \
           && [ -s /tmp/_apkc_proof_dex.bin ]; then
            {
              printf '# classes.dex SHA-1 — hardened qemu proof commit %s %s\n' "$SHORT" "$DATE_UTC"
              sha1sum /tmp/_apkc_proof_dex.bin | awk '{print $1"  classes.dex"}'
            } > "$OUT/dex-sha1.txt"
            DEX_INTERNAL_STATUS="TOKEN_VAZIO (python3 absent)"
            if command -v python3 >/dev/null 2>&1; then
                if python3 - /tmp/_apkc_proof_dex.bin <<'PYEOF'
import hashlib, sys
data = open(sys.argv[1], 'rb').read()
if len(data) < 32:
    sys.exit(1)
stored = data[12:32]
computed = hashlib.sha1(data[32:]).digest()
sys.exit(0 if stored == computed else 1)
PYEOF
                then
                    DEX_INTERNAL_STATUS="PASS (dex[12:32] == sha1(dex[32:]))"
                    DEX_PROVEN=1
                else
                    DEX_INTERNAL_STATUS="FAIL (internal SHA-1 mismatch)"
                fi
            fi
            echo "[DEX-INTERNAL-SHA1] ${DEX_INTERNAL_STATUS}" >> "$TRANSCRIPT"
            if [ "$DEX_INTERNAL_STATUS" = "FAIL (internal SHA-1 mismatch)" ]; then
                log "FAIL: classes.dex internal SHA-1 field is invalid"; exit 1
            fi
        fi
    else
        TZ=$((TZ+1))
        log "FAIL: qemu hardened apkc run"; exit 1
    fi
else
    echo "[QEMU-RUN] STATUS: TOKEN_VAZIO (qemu-aarch64-static absent)" >> "$TRANSCRIPT"
    TZ=$((TZ+1)); log "TOKEN_VAZIO: qemu absent"; exit 0
fi

# ── Stage 2B: exact source-cap boundary falsifier ──────────────────────────
# Accepted payload capacity is sizeof(_src_local)-1 == 1,048,575 bytes.
BOUND_BELOW="/tmp/apkc-boundary-below.s"
BOUND_EXACT="/tmp/apkc-boundary-exact.s"
BOUND_OVER="/tmp/apkc-boundary-over.s"
if python3 - Apkc/hello.s.txt "$BOUND_BELOW" "$BOUND_EXACT" "$BOUND_OVER" <<'PYEOF'
from pathlib import Path
import sys
base = Path(sys.argv[1]).read_bytes()
for target, path in zip((1048574, 1048575, 1048576), sys.argv[2:]):
    if len(base) > target:
        raise SystemExit(f"fixture base too large: {len(base)} > {target}")
    # Trailing ASCII spaces preserve the assembly program while producing an
    # exact byte-length fixture for the input-capacity gate.
    data = base + (b" " * (target - len(base)))
    if len(data) != target:
        raise SystemExit("fixture length invariant failed")
    Path(path).write_bytes(data)
PYEOF
then
    BOUNDARY_OK=1
    for spec in "below:$BOUND_BELOW:1048574:pass" "exact:$BOUND_EXACT:1048575:pass" "over:$BOUND_OVER:1048576:fail"; do
        IFS=: read -r label fixture expected_bytes expectation <<<"$spec"
        apk="/tmp/apkc-boundary-${label}.apk"
        logf="/tmp/apkc-boundary-${label}.log"
        rm -f "$apk" "$logf"
        qemu-aarch64-static "$APKC_ELF" "$fixture" -o "$apk" -64 >"$logf" 2>&1
        rc=$?
        got_bytes="$(wc -c < "$fixture" | tr -d ' ')"
        if [ "$got_bytes" != "$expected_bytes" ]; then
            BOUNDARY_OK=0
            echo "[BOUNDARY-${label}] FAIL fixture bytes=${got_bytes} expected=${expected_bytes}" >> "$TRANSCRIPT"
            continue
        fi
        if [ "$expectation" = pass ]; then
            if [ "$rc" -eq 0 ] && [ -s "$apk" ]; then
                echo "[BOUNDARY-${label}] PASS bytes=${got_bytes} rc=0 apk_sha256=$(sha256sum "$apk" | awk '{print $1}')" >> "$TRANSCRIPT"
            else
                BOUNDARY_OK=0
                echo "[BOUNDARY-${label}] FAIL bytes=${got_bytes} rc=${rc} apk_present=$([ -s "$apk" ] && echo yes || echo no)" >> "$TRANSCRIPT"
            fi
        else
            if [ "$rc" -ne 0 ] && [ ! -e "$apk" ] && grep -q 'source exceeds 1MiB bounded input' "$logf"; then
                echo "[BOUNDARY-${label}] PASS bytes=${got_bytes} rc=${rc} no_apk=yes overflow_marker=yes" >> "$TRANSCRIPT"
            else
                BOUNDARY_OK=0
                echo "[BOUNDARY-${label}] FAIL bytes=${got_bytes} rc=${rc} apk_present=$([ -e "$apk" ] && echo yes || echo no) overflow_marker=$(grep -q 'source exceeds 1MiB bounded input' "$logf" && echo yes || echo no)" >> "$TRANSCRIPT"
            fi
        fi
    done
    if [ "$BOUNDARY_OK" -eq 1 ]; then
        BOUNDARY_PROVEN=1
        PASS=$((PASS+1))
        echo "[BOUNDARY-TRIAD] STATUS: PASS (<cap PASS, =cap PASS, cap+1 fail-closed/no APK)" >> "$TRANSCRIPT"
    else
        echo "[BOUNDARY-TRIAD] STATUS: FAIL" >> "$TRANSCRIPT"
        log "FAIL: bounded source triad"; exit 1
    fi
else
    echo "[BOUNDARY-TRIAD] STATUS: FAIL (fixture generation)" >> "$TRANSCRIPT"
    exit 1
fi

# ── Stage 3: extract lib/arm64-v8a/libmain.so and validate ELF header ─────
SO_OUT="/tmp/libmain_arm64_proof.so"
if unzip -p "$APK_OUT" lib/arm64-v8a/libmain.so > "$SO_OUT" 2>/dev/null && [ -s "$SO_OUT" ]; then
    MAGIC="$(od -An -tx1 -N4 "$SO_OUT" | tr -d ' \n')"
    ELF_HDR="$(readelf -h "$SO_OUT" 2>/dev/null | grep -E 'Class|Data|Machine|Type' | sed 's/^/    /')"
    SHA_SO="$(sha256sum "$SO_OUT" | awk '{print $1}')"
    ELF_CLASS="$(readelf -h "$SO_OUT" 2>/dev/null | grep 'Class:' | grep -c 'ELF64' || true)"
    ELF_MACHINE="$(readelf -h "$SO_OUT" 2>/dev/null | grep 'Machine:' | grep -c 'AArch64' || true)"
    ELF_TYPE="$(readelf -h "$SO_OUT" 2>/dev/null | grep 'Type:' | grep -c 'DYN' || true)"
    if [ "${ELF_CLASS:-0}" -gt 0 ] && [ "${ELF_MACHINE:-0}" -gt 0 ] && [ "${ELF_TYPE:-0}" -gt 0 ]; then
        ELF_VERDICT="PASS (ELF64 AArch64 DYN — arm64-v8a .so inside generated APK)"
    else
        ELF_VERDICT="FAIL (unexpected ELF class/machine/type — not ELF64 AArch64 DYN)"
    fi
    {
      echo ""
      echo "[ELF-SO] archive member: lib/arm64-v8a/libmain.so"
      echo "[ELF-SO] magic:   0x${MAGIC}"
      echo "[ELF-SO] sha256:  ${SHA_SO}"
      echo "[ELF-SO] readelf:"
      echo "$ELF_HDR"
      echo "[ELF-SO] STATUS:  ${ELF_VERDICT}"
    } >> "$TRANSCRIPT"
    { echo "# lib/arm64-v8a/libmain.so ELF header — hardened ARM64 APK proof (L4), ${DATE_UTC}, commit ${SHORT}";
      readelf -h "$SO_OUT"; } > "$OUT/readelf-arm64.txt"
    if [ "$ELF_VERDICT" = "PASS (ELF64 AArch64 DYN — arm64-v8a .so inside generated APK)" ]; then
        PASS=$((PASS+1))
    else
        log "FAIL: libmain.so ELF header validation: ${ELF_VERDICT}"; exit 1
    fi
else
    echo "[ELF-SO] STATUS: FAIL — lib/arm64-v8a/libmain.so not in APK" >> "$TRANSCRIPT"
    log "FAIL: libmain.so not in APK"; exit 1
fi

# ── Stage 4: zipalign + apksigner (debug, ephemeral keystore) ─────────────
APK_ALIGNED="/tmp/hello-arm64-aligned.apk"
APK_SIGNED="/tmp/hello-arm64-signed.apk"
KEYSTORE="/tmp/apkc_debug_proof.keystore"
SIGN_SUCCEEDED=0
rm -f "$APK_ALIGNED" "$APK_SIGNED"

if command -v zipalign >/dev/null 2>&1 && command -v apksigner >/dev/null 2>&1; then
    keytool -genkeypair -keystore "$KEYSTORE" -storepass android -keypass android \
        -alias androiddebugkey -keyalg RSA -keysize 2048 \
        -dname "CN=ApkC Debug, O=Rafael, C=BR" \
        -validity 365 >/dev/null 2>&1 || true

    if [ -f "$KEYSTORE" ]; then
        rm -f "$APK_ALIGNED" "$APK_SIGNED"
        if zipalign -f 4 "$APK_OUT" "$APK_ALIGNED" 2>/dev/null && \
           apksigner sign --ks "$KEYSTORE" --ks-pass pass:android --key-pass pass:android \
               --out "$APK_SIGNED" "$APK_ALIGNED" 2>/dev/null; then
            VERIFY_OUT="$(apksigner verify --verbose "$APK_SIGNED" 2>&1)"
            VERIFY_EXIT=$?
            if [ $VERIFY_EXIT -eq 0 ]; then
                SIG_LINES="$(printf '%s\n' "$VERIFY_OUT" | grep -E 'Verified|v1|v2|v3' | head -5)"
                SHA_SIGNED="$(sha256sum "$APK_SIGNED" | awk '{print $1}')"
                {
                  echo ""
                  echo "[SIGN] tool: apksigner ${SIGNV}"
                  echo "[SIGN] sha256 signed: ${SHA_SIGNED}"
                  echo "[SIGN] verify output:"
                  echo "$SIG_LINES" | sed 's/^/    /'
                  echo "[SIGN] STATUS: PASS (debug-signed APK verified; keystore ephemeral /tmp)"
                } >> "$TRANSCRIPT"
                printf '%s\n' "$VERIFY_OUT" > "$OUT/apksigner-verify.txt"
                PASS=$((PASS+1))
                SIGN_SUCCEEDED=1
            else
                echo "[SIGN] STATUS: FAIL (apksigner verify exit ${VERIFY_EXIT})" >> "$TRANSCRIPT"
                printf '%s\n' "$VERIFY_OUT" | head -5 >> "$TRANSCRIPT"
                log "FAIL: apksigner verify failed"; exit 1
            fi
        else
            echo "[SIGN] STATUS: FAIL (zipalign or apksigner sign failed)" >> "$TRANSCRIPT"
            log "FAIL: zipalign or apksigner sign step failed"; exit 1
        fi
    else
        echo "[SIGN] STATUS: TOKEN_VAZIO (keytool failed to generate keystore)" >> "$TRANSCRIPT"
        TZ=$((TZ+1))
    fi
else
    echo "[SIGN] STATUS: TOKEN_VAZIO (zipalign/apksigner absent)" >> "$TRANSCRIPT"
    TZ=$((TZ+1))
fi

# ── Stage 5: aapt manifest dump ───────────────────────────────────────────
if command -v aapt >/dev/null 2>&1; then
    if [ "$SIGN_SUCCEEDED" -eq 1 ]; then AAPT_SRC="$APK_SIGNED"; else AAPT_SRC="$APK_OUT"; fi
    AAPT_OUT="$(aapt dump xmltree "$AAPT_SRC" AndroidManifest.xml 2>/dev/null || echo TOKEN_VAZIO)"
    {
      echo ""
      echo "[AAPT] command: aapt dump xmltree hello-arm64.apk AndroidManifest.xml"
      echo "$AAPT_OUT" | head -15 | sed 's/^/    /'
    } >> "$TRANSCRIPT"
    echo "$AAPT_OUT" > "$OUT/aapt-xmltree.txt"
    if echo "$AAPT_OUT" | grep -iq "manifest" && \
       echo "$AAPT_OUT" | grep -iq "NativeActivity" && \
       echo "$AAPT_OUT" | grep -iq "lib_name"; then
        PASS=$((PASS+1)); AAPT_PROVEN=1
    else
        echo "[AAPT] STATUS: FAIL (NativeActivity or lib_name markers absent)" >> "$TRANSCRIPT"
        log "FAIL: aapt manifest missing NativeActivity or lib_name"; exit 1
    fi
fi

# ── Stage 6: update validation-summary with QEMU proof overrides ──────────
VSUM="$OUT/validation-summary.md"
if [ -f "$VSUM" ] && [ "$PASS" -ge 3 ]; then
    APK_SZ="$(wc -c < "$APK_OUT" 2>/dev/null | tr -d ' ' || echo '?')"
    {
      printf '\n## Hardened ARM64 qemu proof overrides — commit %s %s\n\n' "$SHORT" "$DATE_UTC"
      echo "| Gate | Status | Evidência |"
      echo "|---|---|---|"
      echo "| H0-RUNTIME | PASS | runtime-hardened TU; raw=${RAW_SHA}; hardened=${RUNTIME_SHA}; APC-RH-013 |"
      echo "| F2 | PASS | hello-arm64-proof.apk via qemu (${APK_SZ} bytes), sha256=${APK_SHA} |"
      echo "| F2B | $([ "$BOUNDARY_PROVEN" -eq 1 ] && echo PASS || echo TOKEN_VAZIO) | source boundary triad <cap/=cap/cap+1 |"
      echo "| F3 | PASS | unzip.txt regenerated from hardened qemu APK |"
      if [ "$AAPT_PROVEN" -eq 1 ]; then echo "| F4 | PASS | NativeActivity + lib_name confirmed |"; else echo "| F4 | TOKEN_VAZIO | aapt absent |"; fi
      if [ "$DEX_PROVEN" -eq 1 ]; then echo "| F5 | PASS | classes.dex internal SHA-1 confirmed |"; else echo "| F5 | TOKEN_VAZIO | DEX proof unavailable |"; fi
      echo "| F6 | PASS | readelf-arm64.txt: ELF64 AArch64 DYN |"
    } >> "$VSUM"
fi

log "arm64-apk-qemu hardened proof: ${PASS} PASS, ${TZ} TOKEN_VAZIO"
log "transcript: ${TRANSCRIPT}"
exit 0
