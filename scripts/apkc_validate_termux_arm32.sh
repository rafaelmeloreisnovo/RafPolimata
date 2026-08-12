#!/data/data/com.termux/files/usr/bin/sh
set -eu

# ApkC/RAFAELIA — evidence-first Termux ARM32 validator.
# Purpose: reproduce the already observed working path from
# docs/APKC_TERMUX_ARM32_PROOF.md while preserving fail-closed behavior.
# This script never upgrades runtime claims: claim_allowed remains false
# until runtime/logcat evidence is captured separately.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
OUT="$APKC/proofs/termux-arm32"
EXEC_ROOT=${APKC_EXEC_ROOT:-"${TMPDIR:-$PREFIX/tmp}/apkc_termux_arm32_$$"}
mkdir -p "$OUT" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM

SUMMARY="$OUT/validation-summary.md"
RECEIPT="$OUT/receipt.sha256"
HARD_SRC="$EXEC_ROOT/apkc_hardened.c"
EXE="$EXEC_ROOT/apkc"
APK="$OUT/hello.apk"

: > "$SUMMARY"
status(){ printf '| %s | %s | %s |\n' "$1" "$2" "$3" >> "$SUMMARY"; }
need(){ command -v "$1" >/dev/null 2>&1 || { status "$2" TOKEN_VAZIO "$1 ausente"; printf 'TOKEN_VAZIO: %s ausente\n' "$1" >&2; exit 1; }; }

DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
COMMIT=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf TOKEN_VAZIO)
{
  echo '# ApkC Termux ARM32 validation'
  echo
  echo "- date_utc: $DATE"
  echo "- commit: $COMMIT"
  echo '- evidence_model: measured/local'
  echo '- claim_allowed: false'
  echo '- runtime_claim: TOKEN_VAZIO'
  echo
  echo '| Gate | Status | Evidência |'
  echo '|---|---|---|'
} > "$SUMMARY"

# H0: source-cap hardening is mandatory before compilation.
need python3 H0
[ -f "$ROOT/scripts/patch_apkc_source_cap.py" ] || { status H0 FAIL 'patch_apkc_source_cap.py ausente'; exit 1; }
[ -f "$ROOT/tests/test_apkc_source_cap_patch.py" ] || { status H0 FAIL 'test_apkc_source_cap_patch.py ausente'; exit 1; }
python3 "$ROOT/tests/test_apkc_source_cap_patch.py" > "$OUT/source-cap-test.txt" 2>&1 || { status H0 FAIL 'falsificador source-cap falhou'; exit 1; }
python3 "$ROOT/scripts/patch_apkc_source_cap.py" "$APKC/apkc.c" "$HARD_SRC" > "$OUT/source-cap-transform.txt" 2>&1 || { status H0 FAIL 'transformação source-cap falhou'; exit 1; }
grep -q 'source exceeds SRC_CAP' "$HARD_SRC" || { status H0 FAIL 'guard SRC_CAP ausente'; exit 1; }
! grep -Fq 'if (n<=0) break;' "$HARD_SRC" || { status H0 FAIL 'âncora legada insegura presente'; exit 1; }
status H0 PASS 'transformação + falsificador source-cap PASS'

# F0: canonical input.
[ -s "$APKC/hello.s.txt" ] || { status F0 FAIL 'Apkc/hello.s.txt ausente/vazio'; exit 1; }
status F0 PASS 'Apkc/hello.s.txt presente'

# F1: use the command shape physically proven on Termux ARM32.
need cc F1
cc -std=c11 -Oz -Wno-unused-function -nostartfiles -Wl,-e,_start \
  "$HARD_SRC" -o "$EXE" > "$OUT/apkc-compile.txt" 2>&1 || { status F1 FAIL 'compilação nativa com -nostartfiles falhou'; exit 1; }
chmod 700 "$EXE"
[ -x "$EXE" ] || { status F1 FAIL 'binário apkc não executável'; exit 1; }
status F1 PASS 'apkc hardened compilado nativamente com caminho comprovado -nostartfiles'

# F2: generation with the package/name/ABI parameters from the observed proof.
"$EXE" "$APKC/hello.s.txt" -o "$APK" -p com.rafael.teste -l RafaelTeste -n hello -32 \
  > "$OUT/apkc-generate.txt" 2>&1 || { status F2 FAIL 'geração hello.apk falhou'; exit 1; }
[ -s "$APK" ] || { status F2 FAIL 'hello.apk ausente/vazio após exit 0'; exit 1; }
status F2 PASS 'hello.apk gerado pelo apkc hardened'

# F3: ZIP/APK structure must contain the three minimum artifacts.
need unzip F3
unzip -t "$APK" > "$OUT/unzip-test.txt" 2>&1 || { status F3 FAIL 'unzip -t falhou'; exit 1; }
unzip -l "$APK" > "$OUT/unzip-list.txt" 2>&1 || { status F3 FAIL 'unzip -l falhou'; exit 1; }
for p in AndroidManifest.xml classes.dex lib/armeabi-v7a/libhello.so; do
  unzip -Z1 "$APK" | grep -Fxq "$p" || { status F3 FAIL "entrada obrigatória ausente: $p"; exit 1; }
done
status F3 PASS 'ZIP íntegro e entradas mínimas presentes'

# F4: AXML is evidence only when aapt is available; absence remains TOKEN_VAZIO.
if command -v aapt >/dev/null 2>&1; then
  aapt dump xmltree "$APK" AndroidManifest.xml > "$OUT/aapt-xmltree.txt" 2>&1 || { status F4 FAIL 'aapt não parseou AndroidManifest.xml'; exit 1; }
  grep -q 'com.rafael.teste' "$OUT/aapt-xmltree.txt" || { status F4 FAIL 'package esperado não encontrado no AXML'; exit 1; }
  grep -q 'android.app.NativeActivity' "$OUT/aapt-xmltree.txt" || { status F4 FAIL 'NativeActivity não encontrada no AXML'; exit 1; }
  status F4 PASS 'AXML parseado por aapt; package + NativeActivity encontrados'
else
  printf 'TOKEN_VAZIO: aapt ausente\n' > "$OUT/aapt-xmltree.txt"
  status F4 TOKEN_VAZIO 'aapt ausente; claim AXML não elevado nesta execução'
fi

# F5: DEX signature and checksum boundary.
python3 - "$APK" > "$OUT/dex-sha1.txt" <<'PY'
import hashlib, sys, zipfile
apk=sys.argv[1]
with zipfile.ZipFile(apk) as z:
    d=z.read('classes.dex')
if len(d) < 32 or d[:8] != b'dex\n035\x00':
    print('FAIL: DEX magic/length')
    raise SystemExit(1)
h=d[12:32]
c=hashlib.sha1(d[32:]).digest()
print('header_sha1='+h.hex())
print('computed_sha1='+c.hex())
if h != c:
    print('FAIL')
    raise SystemExit(1)
print('PASS')
PY
status F5 PASS 'classes.dex magic + SHA-1 interno conferem'

# F6: native library must be real ELF32 ARM and export the Android entry symbols.
need readelf F6
unzip -p "$APK" lib/armeabi-v7a/libhello.so > "$EXEC_ROOT/libhello.so" || { status F6 FAIL 'extração libhello.so falhou'; exit 1; }
readelf -h "$EXEC_ROOT/libhello.so" > "$OUT/readelf-header.txt" 2>&1 || { status F6 FAIL 'readelf -h falhou'; exit 1; }
readelf -s "$EXEC_ROOT/libhello.so" > "$OUT/readelf-symbols.txt" 2>&1 || { status F6 FAIL 'readelf -s falhou'; exit 1; }
grep -q 'Class:.*ELF32' "$OUT/readelf-header.txt" || { status F6 FAIL 'ELF32 não confirmado'; exit 1; }
grep -q 'Machine:.*ARM' "$OUT/readelf-header.txt" || { status F6 FAIL 'Machine ARM não confirmada'; exit 1; }
grep -q 'ANativeActivity_onCreate' "$OUT/readelf-symbols.txt" || { status F6 FAIL 'ANativeActivity_onCreate ausente'; exit 1; }
grep -q 'android_main' "$OUT/readelf-symbols.txt" || { status F6 FAIL 'android_main ausente'; exit 1; }
status F6 PASS 'libhello.so = ELF32 ARM; símbolos NativeActivity presentes'

# F7: deterministic receipt over generated evidence. Signing/install/runtime are separate gates.
(
  cd "$OUT"
  find . -maxdepth 1 -type f ! -name 'receipt.sha256' -print | LC_ALL=C sort | while IFS= read -r f; do sha256sum "$f"; done
) > "$RECEIPT"
sha256sum -c "$RECEIPT" > "$OUT/receipt-verify.txt" 2>&1 || { status F7 FAIL 'receipt SHA-256 não revalida'; exit 1; }
status F7 PASS 'receipt SHA-256 criado e revalidado localmente'

{
  echo
  echo '## Claim gate'
  echo
  echo '- claim_allowed: false'
  echo '- permitido nesta execução: build/generate/ZIP/DEX/ELF e AXML somente se F4=PASS.'
  echo '- TOKEN_VAZIO: assinatura, instalação, abertura e comportamento runtime/logcat.'
  echo '- próximo gate: executar assinatura + instalação + logcat em aparelho e anexar receipt separado.'
} >> "$SUMMARY"

printf '%s\n' "PASS: validação estrutural ApkC ARM32 concluída; runtime permanece TOKEN_VAZIO / claim_allowed=false"
