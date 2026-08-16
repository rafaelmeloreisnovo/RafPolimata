#!/bin/sh
set -eu
# tools/raf_clean_proof_run.sh — one clean reproducible proof run (closes L6).
#
# Writes a header (commit_sha, date_utc, host_arch, toolchain versions) and runs
# every HOST-TRACTABLE gate on this x86_64 host, capturing PASS / FAIL /
# TOKEN_VAZIO per gate into a fresh timestamped dir:  Apkc/proofs/runs/<UTC>/
#
# Honesty contract: never PASS without running the command. Gates that need
# ARM/NDK/device are marked TOKEN_VAZIO with an explicit reason.
# Idempotent (own dir per run) and quiet-on-success.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

# Closure anchors used by generated evidence receipts. They bind missing
# provenance/toolchain observations to L1, device/runtime gaps to L2, and
# ARM64 artifact validation gaps to L3 without promoting them to PASS.
CLOSURE_PROVENANCE=CLOSURE_L1
CLOSURE_RUNTIME=CLOSURE_L2
CLOSURE_ARM64=CLOSURE_L3

COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo "TOKEN_VAZIO $CLOSURE_PROVENANCE")
DATE_UTC=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
STAMP=$(date -u '+%Y%m%dT%H%M%SZ')
HOST_ARCH=$(uname -m 2>/dev/null || echo "TOKEN_VAZIO $CLOSURE_PROVENANCE")

RUNDIR="$ROOT/Apkc/proofs/runs/$STAMP"
mkdir -p "$RUNDIR"
GATES="$RUNDIR/gates.txt"
HEADER="$RUNDIR/header.txt"
SUMMARY="$RUNDIR/summary.txt"
: > "$GATES"

ver(){ command -v "$1" >/dev/null 2>&1 && "$@" 2>&1 | head -1 || echo "$1: TOKEN_VAZIO (ausente); $CLOSURE_PROVENANCE"; }

{
  echo '# RafPolimata clean proof run'
  echo "commit_sha : $COMMIT"
  echo "date_utc   : $DATE_UTC"
  echo "host_arch  : $HOST_ARCH"
  echo "run_dir    : $RUNDIR"
  echo "governance : $CLOSURE_PROVENANCE $CLOSURE_RUNTIME $CLOSURE_ARM64"
  echo '# toolchain versions'
  echo "clang      : $(ver clang --version)"
  echo "gcc        : $(ver gcc --version)"
  echo "python3    : $(ver python3 --version)"
  echo "readelf    : $(ver readelf --version)"
  echo "node       : $(ver node --version)"
} > "$HEADER"

PASS=0
TOKV=0
FAIL=0
# gate <id> <STATUS> <evidence>
gate(){
  printf '%-22s %-12s %s\n' "$1" "$2" "$3" >> "$GATES"
  case "$2" in
    PASS) PASS=$((PASS+1));;
    TOKEN_VAZIO) TOKV=$((TOKV+1));;
    FAIL) FAIL=$((FAIL+1));;
  esac
}

# --- Gate 0: source-cap hardening transformer + real-source consumption -----
# Never compile the vulnerable anchor in this proof path. If the transform or
# its falsifier fails, downstream compiler gates fail closed.
G0LOG="$RUNDIR/g0_source_cap.txt"
HARD_SRC="$RUNDIR/apkc_hardened.c"
HARDENED_OK=0
if python3 tests/test_apkc_source_cap_patch.py > "$G0LOG" 2>&1 \
   && python3 scripts/patch_apkc_source_cap.py Apkc/apkc.c "$HARD_SRC" >> "$G0LOG" 2>&1 \
   && grep -q 'source exceeds SRC_CAP' "$HARD_SRC" \
   && grep -q 'if (n<0).*source read failed' "$HARD_SRC"; then
  SRC_SHA=$(sha256sum Apkc/apkc.c 2>/dev/null | cut -d' ' -f1 || echo "TOKEN_VAZIO $CLOSURE_PROVENANCE")
  HARD_SHA=$(sha256sum "$HARD_SRC" 2>/dev/null | cut -d' ' -f1 || echo "TOKEN_VAZIO $CLOSURE_PROVENANCE")
  {
    echo "source_sha256: $SRC_SHA"
    echo "hardened_sha256: $HARD_SHA"
    echo "guard: source exceeds SRC_CAP"
    echo "legacy_anchor_present: no"
  } >> "$G0LOG"
  HARDENED_OK=1
  gate source-cap-hardening PASS "transform+falsifier+real-source consumption; sha256=$HARD_SHA"
else
  gate source-cap-hardening FAIL 'transform/falsifier/anchor gate failed -> g0_source_cap.txt'
fi

# --- Gate 1: freestanding aarch64 syntax check ------------------------------
G1LOG="$RUNDIR/g1_syntax.txt"
if [ "$HARDENED_OK" -eq 1 ]; then
  if clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc \
       -ffreestanding -I Apkc "$HARD_SRC" > "$G1LOG" 2>&1; then
    gate freestanding-syntax PASS 'clang aarch64 hardened source -fsyntax-only clean -> g1_syntax.txt'
  else
    gate freestanding-syntax FAIL 'hardened source syntax failed -> g1_syntax.txt'
  fi
else
  echo 'FAIL: hardened source unavailable; refusing vulnerable-source syntax gate.' > "$G1LOG"
  gate freestanding-syntax FAIL 'blocked by source-cap-hardening'
fi

# --- Gate 2: cross-compile hardened apkc to AArch64 ELF object --------------
G2OBJ="$RUNDIR/apkc_a64.o"
G2LOG="$RUNDIR/g2_object.txt"
if [ "$HARDENED_OK" -eq 1 ] && \
   clang -target aarch64-linux-gnu -ffreestanding -nostdlib -nostdinc -I Apkc \
     -c "$HARD_SRC" -o "$G2OBJ" > "$G2LOG" 2>&1; then
  if command -v readelf >/dev/null 2>&1; then
    readelf -h "$G2OBJ" >> "$G2LOG" 2>&1 || true
    CLASS=$(readelf -h "$G2OBJ" 2>/dev/null | sed -n 's/.*Class:[[:space:]]*//p' | head -1)
    MACH=$(readelf -h "$G2OBJ" 2>/dev/null | sed -n 's/.*Machine:[[:space:]]*//p' | head -1)
  else
    CLASS="TOKEN_VAZIO $CLOSURE_ARM64"
    MACH="TOKEN_VAZIO $CLOSURE_ARM64"
  fi
  SHA=$(sha256sum "$G2OBJ" 2>/dev/null | cut -d' ' -f1 || echo "TOKEN_VAZIO $CLOSURE_PROVENANCE")
  echo "sha256: $SHA" >> "$G2LOG"
  gate cross-object-a64 PASS "hardened source Class=$CLASS Machine=$MACH sha256=$SHA"
else
  gate cross-object-a64 FAIL 'hardened cross-compile to AArch64 object failed/blocked -> g2_object.txt'
fi

# --- Gate 3: verbovivo build + smoke ----------------------------------------
G3BIN="$RUNDIR/verbovivo_ci"
G3LOG="$RUNDIR/g3_verbovivo.txt"
if gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \
     rafaelia/verbovivo.c rafaelia/fiber_relmat.c -lm -o "$G3BIN" > "$G3LOG" 2>&1; then
  if echo 'RAFAELIA proof' | "$G3BIN" /dev/stdin "$RUNDIR/engram.svg" 2>&1 \
       | grep -E 'verbovivo:.*phi=' >> "$G3LOG"; then
    gate verbovivo-build-smoke PASS 'build+smoke; phi line -> g3_verbovivo.txt'
  else
    gate verbovivo-build-smoke FAIL 'smoke produced no phi line -> g3_verbovivo.txt'
  fi
else
  gate verbovivo-build-smoke FAIL 'verbovivo build failed -> g3_verbovivo.txt'
fi

# --- Gate 4: ARM32 encoder golden tests -------------------------------------
G4LOG="$RUNDIR/g4_arm32_encoders.txt"
if python3 tests/test_arm32_encoders.py > "$G4LOG" 2>&1; then
  gate arm32-encoders PASS "$(tail -1 "$G4LOG")"
else
  gate arm32-encoders FAIL 'see g4_arm32_encoders.txt'
fi

# --- Gate 5: ARM64 encoder golden tests -------------------------------------
G5LOG="$RUNDIR/g5_arm64_encoders.txt"
if python3 tests/test_arm64_encoders.py > "$G5LOG" 2>&1; then
  gate arm64-encoders PASS "$(tail -1 "$G5LOG")"
else
  gate arm64-encoders FAIL 'see g5_arm64_encoders.txt'
fi

# --- Gate 6: actual APK generation by running apkc --------------------------
# apkc is freestanding ARM64 code; it is not runnable on this x86 host.
G6LOG="$RUNDIR/g6_apk_generation.txt"
echo "TOKEN_VAZIO: hardened apkc is freestanding ARM64, not runnable on x86 host; needs ARM/Termux/qemu. $CLOSURE_RUNTIME $CLOSURE_ARM64" > "$G6LOG"
gate apk-generation TOKEN_VAZIO "requires ARM/Termux/qemu/device -> g6_apk_generation.txt; $CLOSURE_RUNTIME $CLOSURE_ARM64"

# --- summary / global fail-closed -------------------------------------------
{
  echo "commit_sha : $COMMIT"
  echo "date_utc   : $DATE_UTC"
  echo "host_arch  : $HOST_ARCH"
  echo "clean-proof-run: $PASS PASS, $FAIL FAIL, $TOKV TOKEN_VAZIO [$CLOSURE_RUNTIME $CLOSURE_ARM64]"
} > "$SUMMARY"

echo "run_dir: $RUNDIR"
echo "clean-proof-run: $PASS PASS, $FAIL FAIL, $TOKV TOKEN_VAZIO [$CLOSURE_RUNTIME $CLOSURE_ARM64]"

# A host-tractable FAIL is a failing proof run. TOKEN_VAZIO remains auditable
# but does not masquerade as failure or PASS.
[ "$FAIL" -eq 0 ]
