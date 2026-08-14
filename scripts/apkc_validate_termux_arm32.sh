#!/data/data/com.termux/files/usr/bin/sh
set -eu

# ApkC/RAFAELIA — evidence-first Termux ARM32 validator.
# Reproduces the physically observed path while preserving fail-closed,
# append-only evidence and claim_allowed=false for unmeasured runtime claims.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
BASE_OUT="$APKC/proofs/termux-arm32"
COMMIT=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf TOKEN_VAZIO)
COMMIT_SHORT=$(printf '%s' "$COMMIT" | cut -c1-12)
RUN_STAMP=$(date -u '+%Y%m%dT%H%M%SZ')
RUN_ID="${RUN_STAMP}_${COMMIT_SHORT}_$$"
OUT="$BASE_OUT/runs/$RUN_ID"
EXEC_ROOT=${APKC_EXEC_ROOT:-"${TMPDIR:-$PREFIX/tmp}/apkc_termux_arm32_$$"}
mkdir -p "$OUT" "$EXEC_ROOT"

SUMMARY="$OUT/validation-summary.md"
RECEIPT="$OUT/receipt.sha256"
HARD_SRC="$EXEC_ROOT/apkc_hardened.c"
EXE="$EXEC_ROOT/apkc"
APK="$OUT/hello.apk"
APK_REPRO="$OUT/hello.repro.apk"

# Finalize every run, including failures. Preserve the original gate exit code
# separately from finalizer/receipt status so chain-of-custody errors never
# rewrite the historical cause of failure.
finalize(){
  gate_rc=$?
  final_rc=$gate_rc
  trap - EXIT HUP INT TERM

  printf 'gate_exit_code=%s\n' "$gate_rc" > "$OUT/run-exit.txt"
  printf 'receipt_status=TOKEN_VAZIO\n' > "$OUT/finalization-status.txt"

  if command -v sha256sum >/dev/null 2>&1; then
    rm -f "$RECEIPT" "$OUT/receipt-verify.txt"
    (
      cd "$OUT"
      find . -maxdepth 1 -type f ! -name 'receipt.sha256' ! -name 'receipt-verify.txt' -print \
        | LC_ALL=C sort \
        | while IFS= read -r f; do sha256sum "$f"; done
    ) > "$RECEIPT"

    if sha256sum -c "$RECEIPT" > "$OUT/receipt-verify.txt" 2>&1; then
      printf 'receipt_status=PASS\ngate_exit_code=%s\n' "$gate_rc" > "$OUT/finalization-status.txt"
      (
        cd "$OUT"
        find . -maxdepth 1 -type f ! -name 'receipt.sha256' ! -name 'receipt-verify.txt' -print \
          | LC_ALL=C sort \
          | while IFS= read -r f; do sha256sum "$f"; done
      ) > "$RECEIPT"
      if ! sha256sum -c "$RECEIPT" > "$OUT/receipt-verify.txt" 2>&1; then
        printf 'receipt_status=FAIL\ngate_exit_code=%s\n' "$gate_rc" > "$OUT/finalization-status.txt"
        final_rc=1
      fi
    else
      printf 'receipt_status=FAIL\ngate_exit_code=%s\n' "$gate_rc" > "$OUT/finalization-status.txt"
      final_rc=1
    fi
  else
    printf '%s\n' 'TOKEN_VAZIO: sha256sum ausente; receipt não materializado' > "$OUT/receipt-verify.txt"
    printf 'receipt_status=TOKEN_VAZIO\ngate_exit_code=%s\n' "$gate_rc" > "$OUT/finalization-status.txt"
    final_rc=1
  fi

  printf 'final_exit_code=%s\n' "$final_rc" >> "$OUT/receipt-verify.txt"
  rm -rf "$EXEC_ROOT"
  exit "$final_rc"
}
trap finalize EXIT
trap 'exit 130' HUP INT TERM

: > "$SUMMARY"
status(){ printf '| %s | %s | %s |\n' "$1" "$2" "$3" >> "$SUMMARY"; }
need(){ command -v "$1" >/dev/null 2>&1 || { status "$2" TOKEN_VAZIO "$1 ausente"; printf 'TOKEN_VAZIO: %s ausente\n' "$1" >&2; exit 1; }; }

DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
{
  echo '# ApkC Termux ARM32 validation'
  echo
  echo "- date_utc: $DATE"
  echo "- run_id: $RUN_ID"
  echo "- commit: $COMMIT"
  echo '- evidence_model: measured/local'
  echo '- evidence_storage: append-only/run-scoped'
  echo '- claim_allowed: false'
  echo '- runtime_claim: TOKEN_VAZIO'
  echo
  echo '| Gate | Status | Evidência |'
  echo '|---|---|---|'
} > "$SUMMARY"

# HC: chain-of-custody primitive is mandatory; without it this run must fail closed.
need sha256sum HC
status HC PASS 'sha256sum disponível para receipt final de sucesso ou falha'

# H0: source-cap hardening is mandatory before compilation.
# Use the exact structural verifier; the generic `if (n<=0) break;` text may
# legitimately exist in a different bounded I/O loop and is not a global falsifier.
need python3 H0
[ -f "$ROOT/scripts/patch_apkc_source_cap.py" ] || { status H0 FAIL 'patch_apkc_source_cap.py ausente'; exit 1; }
[ -f "$ROOT/tests/test_apkc_source_cap_patch.py" ] || { status H0 FAIL 'test_apkc_source_cap_patch.py ausente'; exit 1; }
[ -f "$ROOT/scripts/verify_apkc_source_cap_output.py" ] || { status H0 FAIL 'verify_apkc_source_cap_output.py ausente'; exit 1; }
python3 "$ROOT/tests/test_apkc_source_cap_patch.py" > "$OUT/source-cap-test.txt" 2>&1 || { status H0 FAIL 'falsificador source-cap falhou'; exit 1; }
python3 "$ROOT/scripts/patch_apkc_source_cap.py" "$APKC/apkc.c" "$HARD_SRC" > "$OUT/source-cap-transform.txt" 2>&1 || { status H0 FAIL 'transformação source-cap falhou'; exit 1; }
python3 "$ROOT/scripts/verify_apkc_source_cap_output.py" "$HARD_SRC" > "$OUT/source-cap-verify.txt" 2>&1 || { status H0 FAIL 'verificação estrutural exata source-cap falhou'; exit 1; }
RAW_SHA=$(sha256sum "$APKC/apkc.c" | cut -d' ' -f1)
HARD_SHA=$(sha256sum "$HARD_SRC" | cut -d' ' -f1)
printf 'raw_source_sha256=%s\nhardened_source_sha256=%s\n' "$RAW_SHA" "$HARD_SHA" >> "$OUT/source-cap-verify.txt"
status H0 PASS "transformação + falsificador + verificador estrutural exato PASS; hardened_sha256=$HARD_SHA"

# F0: canonical input.
[ -s "$APKC/hello.s.txt" ] || { status F0 FAIL 'Apkc/hello.s.txt ausente/vazio'; exit 1; }
status F0 PASS 'Apkc/hello.s.txt presente'

# F1: command shape physically proven on Termux ARM32.
need cc F1
cc -std=c11 -Oz -Wno-unused-function -nostartfiles -Wl,-e,_start \
  "$HARD_SRC" -o "$EXE" > "$OUT/apkc-compile.txt" 2>&1 || { status F1 FAIL 'compilação nativa com -nostartfiles falhou'; exit 1; }
chmod 700 "$EXE"
[ -x "$EXE" ] || { status F1 FAIL 'binário apkc não executável'; exit 1; }
status F1 PASS 'apkc hardened compilado com caminho comprovado -nostartfiles'

# F2: generation with parameters from observed proof.
"$EXE" "$APKC/hello.s.txt" -o "$APK" -p com.rafael.teste -l RafaelTeste -n hello -32 \
  > "$OUT/apkc-generate.txt" 2>&1 || { status F2 FAIL 'geração hello.apk falhou'; exit 1; }
[ -s "$APK" ] || { status F2 FAIL 'hello.apk ausente/vazio após exit 0'; exit 1; }
status F2 PASS 'hello.apk gerado pelo apkc hardened'

# F2D: same binary + same input + same arguments must reproduce byte-identical APK.
"$EXE" "$APKC/hello.s.txt" -o "$APK_REPRO" -p com.rafael.teste -l RafaelTeste -n hello -32 \
  > "$OUT/apkc-generate-repro.txt" 2>&1 || { status F2D FAIL 'segunda geração falhou'; exit 1; }
[ -s "$APK_REPRO" ] || { status F2D FAIL 'hello.repro.apk ausente/vazio'; exit 1; }
if cmp -s "$APK" "$APK_REPRO"; then
  sha256sum "$APK" "$APK_REPRO" > "$OUT/apk-repro.sha256"
  status F2D PASS 'duas gerações byte-idênticas no mesmo ambiente/processo de validação'
else
  sha256sum "$APK" "$APK_REPRO" > "$OUT/apk-repro.sha256"
  status F2D FAIL 'saídas divergentes; determinismo não demonstrado'
  exit 1
fi

# F3: ZIP/APK structure must contain the three minimum artifacts.
need unzip F3
unzip -t "$APK" > "$OUT/unzip-test.txt" 2>&1 || { status F3 FAIL 'unzip -t falhou'; exit 1; }
unzip -l "$APK" > "$OUT/unzip-list.txt" 2>&1 || { status F3 FAIL 'unzip -l falhou'; exit 1; }
for p in AndroidManifest.xml classes.dex lib/armeabi-v7a/libhello.so; do
  unzip -Z1 "$APK" | grep -Fxq "$p" || { status F3 FAIL "entrada obrigatória ausente: $p"; exit 1; }
done
status F3 PASS 'ZIP íntegro e entradas mínimas presentes'

# F4: AXML evidence exists only when aapt is available.
if command -v aapt >/dev/null 2>&1; then
  aapt dump xmltree "$APK" AndroidManifest.xml > "$OUT/aapt-xmltree.txt" 2>&1 || { status F4 FAIL 'aapt não parseou AndroidManifest.xml'; exit 1; }
  grep -q 'com.rafael.teste' "$OUT/aapt-xmltree.txt" || { status F4 FAIL 'package esperado não encontrado no AXML'; exit 1; }
  grep -q 'android.app.NativeActivity' "$OUT/aapt-xmltree.txt" || { status F4 FAIL 'NativeActivity não encontrada no AXML'; exit 1; }
  status F4 PASS 'AXML parseado por aapt; package + NativeActivity encontrados'
else
  printf 'TOKEN_VAZIO: aapt ausente\n' > "$OUT/aapt-xmltree.txt"
  status F4 TOKEN_VAZIO 'aapt ausente; claim AXML não elevado nesta execução'
fi

# F5: DEX signature boundary.
if python3 - "$APK" > "$OUT/dex-sha1.txt" <<'PY'
import hashlib, sys, zipfile
with zipfile.ZipFile(sys.argv[1]) as z:
    d=z.read('classes.dex')
if len(d) < 32 or d[:8] != b'dex\n035\x00':
    print('FAIL: DEX magic/length'); raise SystemExit(1)
h=d[12:32]; c=hashlib.sha1(d[32:]).digest()
print('header_sha1='+h.hex()); print('computed_sha1='+c.hex())
if h != c:
    print('FAIL'); raise SystemExit(1)
print('PASS')
PY
then
  status F5 PASS 'classes.dex magic + SHA-1 interno conferem'
else
  status F5 FAIL 'classes.dex magic/SHA-1 interno não conferem'
  exit 1
fi

# F6: native library must be ELF32 ARM with Android entry symbols.
need readelf F6
unzip -p "$APK" lib/armeabi-v7a/libhello.so > "$EXEC_ROOT/libhello.so" || { status F6 FAIL 'extração libhello.so falhou'; exit 1; }
readelf -h "$EXEC_ROOT/libhello.so" > "$OUT/readelf-header.txt" 2>&1 || { status F6 FAIL 'readelf -h falhou'; exit 1; }
readelf -s "$EXEC_ROOT/libhello.so" > "$OUT/readelf-symbols.txt" 2>&1 || { status F6 FAIL 'readelf -s falhou'; exit 1; }
grep -q 'Class:.*ELF32' "$OUT/readelf-header.txt" || { status F6 FAIL 'ELF32 não confirmado'; exit 1; }
grep -q 'Machine:.*ARM' "$OUT/readelf-header.txt" || { status F6 FAIL 'Machine ARM não confirmada'; exit 1; }
grep -q 'ANativeActivity_onCreate' "$OUT/readelf-symbols.txt" || { status F6 FAIL 'ANativeActivity_onCreate ausente'; exit 1; }
grep -q 'android_main' "$OUT/readelf-symbols.txt" || { status F6 FAIL 'android_main ausente'; exit 1; }
status F6 PASS 'libhello.so = ELF32 ARM; símbolos NativeActivity presentes'

# F7 is deliberately not PASS before the EXIT finalizer actually emits and
# verifies the receipt. The observable proof is receipt-verify.txt plus
# finalization-status.txt after process termination.
status F7 TOKEN_VAZIO 'receipt ainda não emitido/verificado; finalizador EXIT decidirá'
{
  echo
  echo '## Claim gate'
  echo
  echo '- claim_allowed: false'
  echo '- determinism_scope: F2D mede duas gerações com mesmo binário/entrada/args no mesmo ambiente.'
  echo '- cross-build/cross-device determinism: TOKEN_VAZIO até reprodução independente.'
  echo '- permitido: build/generate/ZIP/DEX/ELF e AXML somente se F4=PASS.'
  echo '- TOKEN_VAZIO: assinatura, instalação, abertura e comportamento runtime/logcat.'
  echo '- resultado negativo: gate_exit_code fica congelado no receipt; final_exit_code fica no verifier externo.'
  echo '- receipt válido: somente quando finalization-status.txt=receipt_status=PASS e receipt-verify.txt confirma todos os hashes.'
  echo '- próximo gate: assinatura + instalação + logcat em aparelho com receipt separado.'
} >> "$SUMMARY"

printf '%s\n' "PASS estrutural pré-finalização: $OUT; receipt/runtime/cross-device permanecem não elevados até evidência"
