#!/bin/sh
set -eu
# Uso: sh scripts/apkc_validate.sh
# Valida compilação, geração e artefatos básicos do ApkC sem afirmar instalação/runtime.
# Contrato anti-retração: nenhum caminho de compilação usa Apkc/apkc.c cru; o
# transformador source-cap deve ser aplicado e verificado antes de qualquer build.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
OUT="$APKC/proofs/out"
EXEC_ROOT=${APKC_EXEC_ROOT:-"${TMPDIR:-${TMP:-/tmp}}/apkc_validate_$$"}
mkdir -p "$OUT" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM
SUMMARY="$OUT/validation-summary.md"
COMPILE_LOG="$OUT/apkc-compile.txt"
HARD_LOG="$OUT/source-cap-hardening.txt"
HARD_SRC="$EXEC_ROOT/apkc_hardened.c"
: > "$SUMMARY"
: > "$COMPILE_LOG"
: > "$HARD_LOG"
log(){ printf '%s\n' "$*"; printf '%s\n' "$*" >> "$SUMMARY"; }
status(){ log "| $1 | $2 | $3 |"; }
cmd_status(){ command -v "$1" >/dev/null 2>&1; }

COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)
DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
{
  echo '# ApkC validation summary'
  echo
  echo "- Data UTC: $DATE"
  echo "- Commit: $COMMIT"
  echo "- Exec root: $EXEC_ROOT"
  echo '- claim_allowed: false'
  echo
  echo '| Gate | Status | Evidência/observação |'
  echo '|---|---|---|'
} >> "$SUMMARY"

cd "$APKC"
if test -f hello.s.txt; then
  status F0 PASS 'Apkc/hello.s.txt presente'
else
  status F0 FAIL 'Apkc/hello.s.txt ausente'
  exit 1
fi

# H0 — source-cap hardening obrigatório e fail-closed.
# Se a transformação/falsificador não puder ser executado, nenhum build segue.
if ! cmd_status python3; then
  echo 'TOKEN_VAZIO: python3 ausente; source-cap hardening não pode ser estabelecido.' > "$HARD_LOG"
  status H0 TOKEN_VAZIO 'python3 ausente; validação bloqueada antes da compilação'
  exit 1
fi
if ! test -f "$ROOT/scripts/patch_apkc_source_cap.py" || ! test -f "$ROOT/tests/test_apkc_source_cap_patch.py"; then
  echo 'FAIL: transformer/test source-cap ausente.' > "$HARD_LOG"
  status H0 FAIL 'transformer/test source-cap ausente; validação bloqueada'
  exit 1
fi

if python3 "$ROOT/tests/test_apkc_source_cap_patch.py" > "$HARD_LOG" 2>&1 \
   && python3 "$ROOT/scripts/patch_apkc_source_cap.py" "$APKC/apkc.c" "$HARD_SRC" >> "$HARD_LOG" 2>&1 \
   && grep -q 'source exceeds SRC_CAP' "$HARD_SRC" \
   && ! grep -Fq 'if (n<=0) break;' "$HARD_SRC"; then
  RAW_SHA=$(sha256sum "$APKC/apkc.c" 2>/dev/null | cut -d' ' -f1 || echo TOKEN_VAZIO)
  HARD_SHA=$(sha256sum "$HARD_SRC" 2>/dev/null | cut -d' ' -f1 || echo TOKEN_VAZIO)
  {
    echo "raw_source_sha256: $RAW_SHA"
    echo "hardened_source_sha256: $HARD_SHA"
    echo 'guard: source exceeds SRC_CAP'
    echo 'legacy_anchor_present: no'
  } >> "$HARD_LOG"
  status H0 PASS "source-cap transform+falsifier; hardened_sha256=$HARD_SHA"
else
  status H0 FAIL 'source-cap hardening/falsifier/anchor gate falhou; ver source-cap-hardening.txt'
  exit 1
fi

BUILD_MODE=TOKEN_VAZIO
EXE="$EXEC_ROOT/apkc"

try_build(){
  label=$1
  shift
  {
    echo "--- attempt: $label ---"
    echo "command: $*"
  } >> "$COMPILE_LOG"
  if "$@" >> "$COMPILE_LOG" 2>&1; then
    echo 'exit_code: 0' >> "$COMPILE_LOG"
    return 0
  else
    rc=$?
    echo "exit_code: $rc" >> "$COMPILE_LOG"
    return "$rc"
  fi
}

# 1) Preferir executável nativo: caminho correto em Termux/ARM32/ARM64.
#    SEMPRE compila HARD_SRC; Apkc/apkc.c cru nunca entra no compilador aqui.
if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  for c in cc gcc clang; do
    if command -v "$c" >/dev/null 2>&1; then
      if try_build native-freestanding "$c" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start -I"$APKC" "$HARD_SRC" -lgcc -o "$EXE"; then
        BUILD_MODE=native_runnable
        break
      fi
      if try_build native-freestanding-no-lgcc "$c" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start -I"$APKC" "$HARD_SRC" -o "$EXE"; then
        BUILD_MODE=native_runnable
        break
      fi
    fi
  done
fi

# 2) Cross executável ARM64, quando o toolchain completo existe.
if [ "$BUILD_MODE" = TOKEN_VAZIO ] && command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
  if try_build cross-aarch64-runnable aarch64-linux-gnu-gcc -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -static -Wl,-e,_start -I"$APKC" "$HARD_SRC" -o "$EXE"; then
    BUILD_MODE=cross_runnable
  fi
fi

# 3) Objeto ARM64: prova sintática/objeto, mas não permite gerar APK.
if [ "$BUILD_MODE" = TOKEN_VAZIO ] && command -v clang >/dev/null 2>&1; then
  if try_build arm64-object clang --target=aarch64-linux-android21 -std=c11 -Wall -Wextra -Wno-unused-function -I"$APKC" -c "$HARD_SRC" -o "$OUT/apkc.o"; then
    BUILD_MODE=object_only
  fi
fi

# 4) Fallback mínimo: sintaxe host, ainda contra HARD_SRC.
if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  for c in cc gcc clang; do
    if command -v "$c" >/dev/null 2>&1; then
      if try_build syntax-only "$c" -std=c11 -Wall -Wextra -Wno-unused-function -I"$APKC" -fsyntax-only "$HARD_SRC"; then
        BUILD_MODE=syntax_only
        break
      fi
    fi
  done
fi

case "$BUILD_MODE" in
  native_runnable)
    cp "$EXE" "$OUT/apkc" 2>/dev/null || true
    chmod +x "$EXE" "$OUT/apkc" 2>/dev/null || true
    status F1 PASS 'apkc hardened compilado como binário nativo executável; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  cross_runnable)
    cp "$EXE" "$OUT/apkc" 2>/dev/null || true
    chmod +x "$EXE" "$OUT/apkc" 2>/dev/null || true
    status F1 PASS 'apkc hardened compilado como binário cross executável; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  object_only)
    status F1 PASS 'apkc hardened compilado como objeto ARM64; linker executável ausente; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  syntax_only)
    status F1 PASS 'apkc hardened verificado por sintaxe; toolchain executável ausente; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  *)
    status F1 FAIL 'falha de compilação/verificação básica do hardened source; log: Apkc/proofs/out/apkc-compile.txt'
    exit 1
    ;;
esac

if [ "$BUILD_MODE" != native_runnable ] && [ "$BUILD_MODE" != cross_runnable ]; then
  echo 'TOKEN_VAZIO: não há binário apkc executável neste ambiente; instale linker/toolchain ARM compatível ou rode em ARM/Linux.' > "$OUT/apkc-generate.txt"
  status F2 TOKEN_VAZIO 'geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável'
  echo 'TOKEN_VAZIO: hello.apk ausente; unzip não executado.' > "$OUT/unzip.txt"; status F3 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; aapt não executado.' > "$OUT/aapt-xmltree.txt"; status F4 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; DEX SHA-1 não executado.' > "$OUT/dex-sha1.txt"; status F5 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; readelf arm64 não executado.' > "$OUT/readelf-arm64.txt"; status F6A TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; readelf arm32 não executado.' > "$OUT/readelf-arm32.txt"; status F6B TOKEN_VAZIO 'hello.apk ausente'
  log ''
  log 'Conclusão: hardening + compilação/verificação básica arquivados; geração de APK exige binário executável do ApkC.'
  exit 0
fi

RUN_EXE="$EXE"
if ! [ -x "$RUN_EXE" ] && [ -x "$OUT/apkc" ]; then RUN_EXE="$OUT/apkc"; fi

if "$RUN_EXE" hello.s.txt -o "$OUT/hello.apk" >"$OUT/apkc-generate.txt" 2>&1; then
  status F2 PASS 'Apkc/proofs/out/hello.apk gerado pelo apkc hardened'
else
  status F2 FAIL 'falha ao gerar hello.apk; ver Apkc/proofs/out/apkc-generate.txt'
  exit 1
fi

if cmd_status unzip; then
  if unzip -l "$OUT/hello.apk" >"$OUT/unzip.txt" 2>&1; then status F3 PASS 'unzip parse registrado'; else status F3 FAIL 'unzip falhou'; exit 1; fi
else
  echo 'TOKEN_VAZIO: unzip ausente; instale unzip.' > "$OUT/unzip.txt"
  status F3 TOKEN_VAZIO 'unzip ausente'
fi

if cmd_status aapt; then
  if aapt dump xmltree "$OUT/hello.apk" AndroidManifest.xml >"$OUT/aapt-xmltree.txt" 2>&1; then
    status F4 PASS 'aapt xmltree registrado'
  else
    status F4 FAIL 'aapt falhou'
    exit 1
  fi
else
  echo 'TOKEN_VAZIO: aapt ausente; instale Android build-tools/aapt.' > "$OUT/aapt-xmltree.txt"
  status F4 TOKEN_VAZIO 'aapt ausente'
fi

if cmd_status python3; then
  python3 - "$OUT/hello.apk" > "$OUT/dex-sha1.txt" <<'PY'
import hashlib, sys, zipfile
apk=sys.argv[1]
try:
    data=zipfile.ZipFile(apk).read('classes.dex')
    hdr=data[12:32]
    calc=hashlib.sha1(data[32:]).digest()
    print('command: internal dex sha1 validation')
    print('status:', 'PASS' if hdr==calc else 'FAIL')
    print('header_sha1:', hdr.hex())
    print('computed_sha1:', calc.hex())
    sys.exit(0 if hdr==calc else 1)
except Exception as e:
    print('status: FAIL')
    print('error:', e)
    sys.exit(1)
PY
  if [ $? -eq 0 ]; then status F5 PASS 'DEX SHA-1 confere'; else status F5 FAIL 'DEX SHA-1 não confere'; exit 1; fi
else
  echo 'TOKEN_VAZIO: python3 ausente; DEX SHA-1 não executado.' > "$OUT/dex-sha1.txt"
  status F5 TOKEN_VAZIO 'python3 ausente'
fi

extract_and_readelf(){
  gate_id=$1; abi=$2; outfile=$3; pattern=$4
  if ! cmd_status readelf; then
    echo 'TOKEN_VAZIO: readelf ausente; instale binutils.' > "$outfile"
    status "$gate_id" TOKEN_VAZIO "readelf ausente para $abi"
    return
  fi
  tmp=$(mktemp -d)
  if unzip -p "$OUT/hello.apk" "$pattern" > "$tmp/lib.so" 2>/dev/null && [ -s "$tmp/lib.so" ]; then
    { echo "command: readelf -h/-s $pattern"; readelf -h "$tmp/lib.so"; readelf -s "$tmp/lib.so"; } > "$outfile" 2>&1
    status "$gate_id" PASS "readelf $abi registrado em ${outfile#$ROOT/}"
  else
    echo "SKIP: biblioteca $pattern não encontrada no APK." > "$outfile"
    status "$gate_id" SKIP "biblioteca $abi ausente"
  fi
  rm -rf "$tmp"
}
extract_and_readelf F6A arm64 "$OUT/readelf-arm64.txt" 'lib/arm64-v8a/*.so'
extract_and_readelf F6B arm32 "$OUT/readelf-arm32.txt" 'lib/armeabi-v7a/*.so'

log ''
log 'Conclusão: validação técnica hardened arquivada; assinatura, instalação e runtime exigem scripts próprios e evidência real.'
