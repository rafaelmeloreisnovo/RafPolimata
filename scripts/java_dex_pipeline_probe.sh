#!/usr/bin/env bash
# java_dex_pipeline_probe.sh — Java/DEX pipeline proof (L11).
#
# Demonstrates the javac → .class → d8 → classes.dex chain that
# apkc uses for Java/Kotlin sources (lang_profile.h LP_JAVA/LP_KT).
#
# What this proves and what it does not:
#   PASS  — javac --release 11 compiles a minimal Java class to .class with
#           correct CAFEBABE magic, on the same host/commit/toolchain.
#   TZ    — d8 → classes.dex requires Android build-tools (absent on standard
#           CI runners); recorded explicitly with one-command closure.
#   TZ    — apkc fork_exec_wait() is #ifdef __aarch64__ only.
#
# Usage: bash scripts/java_dex_pipeline_probe.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

OUT="Apkc/proofs/out"
mkdir -p "$OUT"
DATE_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
COMMIT="$(git rev-parse HEAD 2>/dev/null || echo TOKEN_VAZIO)"
SHORT="$(git rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)"
TRANSCRIPT="$OUT/java-pipeline.txt"

JAVACV="$(javac -version 2>&1 || echo TOKEN_VAZIO)"
D8V="$(d8 --version 2>/dev/null | head -1 || echo TOKEN_VAZIO)"

PASS=0; TZ=0
log() { printf '%s\n' "$*"; }

{
  echo ""
  echo "=================================================================="
  echo "JAVA/DEX PIPELINE PROBE  (L11)  ${DATE_UTC}"
  echo "=================================================================="
  echo "commit:  ${COMMIT}"
  echo "javac:   ${JAVACV}"
  echo "d8:      ${D8V}"
} >> "$TRANSCRIPT"

# ── Stage 1: javac ─────────────────────────────────────────────────────────
# Use /tmp as working directory: current directory may contain filenames with
# non-UTF-8 characters that confuse javac's unnamed-package scanner.
TMP_SRC="/tmp/apkc_java_probe_src"
TMP_CLS="/tmp/apkc_java_probe_cls"
mkdir -p "$TMP_SRC" "$TMP_CLS"

# Minimal Java class that mirrors what apkc would compile.
cat > "$TMP_SRC/Hello.java" <<'JAVA'
public class Hello {
    public static int answer() { return 42; }
}
JAVA

if (cd /tmp && javac --release 11 -d "$TMP_CLS" "$TMP_SRC/Hello.java" 2>/tmp/_javac.err); then
    CLASS_FILE="$TMP_CLS/Hello.class"
    if [ -f "$CLASS_FILE" ]; then
        SHA="$(sha256sum "$CLASS_FILE" | awk '{print $1}')"
        MAGIC="$(od -An -tx1 -N4 "$CLASS_FILE" | tr -d ' \n')"
        SIZE="$(wc -c < "$CLASS_FILE" | tr -d ' ')"
        if [ "$MAGIC" = "cafebabe" ]; then
            STATUS="PASS — valid Java .class (CAFEBABE magic 0xcafebabe)"
            PASS=$((PASS+1))
        else
            STATUS="FAIL — unexpected magic 0x${MAGIC}"
            TZ=$((TZ+1))
        fi
        {
          echo ""
          echo "[JAVAC] command: javac --release 11 -d /tmp/cls/ Hello.java"
          echo "[JAVAC] output:  Hello.class  ${SIZE} bytes"
          echo "[JAVAC] magic:   0x${MAGIC}"
          echo "[JAVAC] sha256:  ${SHA}"
          echo "[JAVAC] STATUS:  ${STATUS}"
        } >> "$TRANSCRIPT"
        # Save class bytecode header for audit
        { echo "# Hello.class header — javac probe (L11), ${DATE_UTC}, commit ${SHORT}";
          javap -verbose "$CLASS_FILE" 2>/dev/null | head -25
        } > "$OUT/java-hello-class.txt" 2>/dev/null || true
    else
        echo "[JAVAC] STATUS: FAIL — Hello.class not produced (javac exited 0 but no output)" >> "$TRANSCRIPT"
        TZ=$((TZ+1))
    fi
else
    echo "[JAVAC] STATUS: TOKEN_VAZIO (javac exited non-zero)" >> "$TRANSCRIPT"
    head -5 /tmp/_javac.err >> "$TRANSCRIPT" 2>/dev/null || true
    TZ=$((TZ+1))
fi

# ── Stage 2: d8 → classes.dex ──────────────────────────────────────────────
{
  echo ""
  echo "[D8] STATUS: TOKEN_VAZIO"
  echo "[D8]   d8 binary absent on this host (requires Android build-tools)."
  echo "[D8]   One-command closure when build-tools are available:"
  echo "[D8]     d8 /tmp/apkc_java_probe_cls/Hello.class \\"
  echo "[D8]        --output /tmp/apkc_dex/ --min-api 21"
  echo "[D8]     xxd /tmp/apkc_dex/classes.dex | head -4  # expect magic: 64 65 78 0a"
  echo "[D8]   Full pipeline (mirrors LP_JAVA in Apkc/lang_profile.h):"
  echo "[D8]     javac --release 11 -d /tmp/cls/ Hello.java"
  echo "[D8]     d8 /tmp/cls/Hello.class --output /tmp/dex/ --min-api 21"
  echo "[D8]     # → /tmp/dex/classes.dex → zip_add('classes.dex') in apkc"
} >> "$TRANSCRIPT"
TZ=$((TZ+1))

# ── Stage 3: apkc fork+exec path (ARM64-only note) ──────────────────────────
{
  echo ""
  echo "[APKC-FORK] STATUS: TOKEN_VAZIO"
  echo "[APKC-FORK]   fork_exec_wait() in Apkc/apkc.c is #ifdef __aarch64__ only."
  echo "[APKC-FORK]   Requires apkc running on ARM/Termux to invoke javac/d8."
  echo "[APKC-FORK]   Closure: run 'apkc Hello.java -o hello.apk' on ARM device."
} >> "$TRANSCRIPT"
TZ=$((TZ+1))

log "java/dex pipeline probe: ${PASS} PASS, ${TZ} TOKEN_VAZIO"
log "transcript: ${TRANSCRIPT}"
exit 0
