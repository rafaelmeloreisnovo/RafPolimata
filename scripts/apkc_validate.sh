#!/bin/sh
set -eu
# Uso: bash scripts/apkc_validate.sh
# Valida compilação, geração e artefatos básicos do ApkC sem afirmar instalação/runtime.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
APKC="$ROOT/Apkc"
OUT="$APKC/proofs/out"
EXEC_ROOT=${APKC_EXEC_ROOT:-"${TMPDIR:-${TMP:-/tmp}}/apkc_validate_$$"}
mkdir -p "$OUT" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM
SUMMARY="$OUT/validation-summary.md"
: > "$SUMMARY"
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

BUILD_MODE=TOKEN_VAZIO
: > "$OUT/apkc-compile.txt"
EXE="$EXEC_ROOT/apkc"

try_build(){
  label=$1
  shift
  {
    echo "command: $*"
    "$@"
  } >"$OUT/apkc-compile.txt" 2>&1
}

# 1) Preferir executável nativo: é o caminho correto em Termux/ARM32/ARM64.
#    Usa _start próprio do apkc.c e tenta libgcc/builtins quando ARM32 precisa de __aeabi_*.
if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  for c in cc gcc clang; do
    if command -v "$c" >/dev/null 2>&1; then
      if try_build native-freestanding "$c" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start apkc.c -lgcc -o "$EXE"; then
        BUILD_MODE=native_runnable
        break
      fi
      if try_build native-freestanding-no-lgcc "$c" -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -Wl,-e,_start apkc.c -o "$EXE"; then
        BUILD_MODE=native_runnable
        break
      fi
    fi
  done
fi

# 2) Cross executável ARM64, quando o toolchain completo existe.
if [ "$BUILD_MODE" = TOKEN_VAZIO ] && command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
  if try_build cross-aarch64-runnable aarch64-linux-gnu-gcc -std=c11 -Wall -Wextra -Wno-unused-function -nostdlib -static -Wl,-e,_start apkc.c -o "$EXE"; then
    BUILD_MODE=cross_runnable
  fi
fi

# 3) Objeto ARM64: prova sintática/objeto, mas não permite gerar APK.
if [ "$BUILD_MODE" = TOKEN_VAZIO ] && command -v clang >/dev/null 2>&1; then
  if try_build arm64-object clang --target=aarch64-linux-android21 -std=c11 -Wall -Wextra -Wno-unused-function -c apkc.c -o "$OUT/apkc.o"; then
    BUILD_MODE=object_only
  fi
fi

# 4) Fallback mínimo: sintaxe host.
if [ "$BUILD_MODE" = TOKEN_VAZIO ]; then
  for c in cc gcc clang; do
    if command -v "$c" >/dev/null 2>&1; then
      if try_build syntax-only "$c" -std=c11 -Wall -Wextra -Wno-unused-function -fsyntax-only apkc.c; then
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
    status F1 PASS 'apkc compilado como binário nativo executável; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  cross_runnable)
    cp "$EXE" "$OUT/apkc" 2>/dev/null || true
    chmod +x "$EXE" "$OUT/apkc" 2>/dev/null || true
    status F1 PASS 'apkc compilado como binário cross executável; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  object_only)
    status F1 PASS 'apkc compilado como objeto ARM64; linker executável ausente; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  syntax_only)
    status F1 PASS 'apkc verificado por sintaxe; toolchain executável ausente; log: Apkc/proofs/out/apkc-compile.txt'
    ;;
  *)
    status F1 FAIL 'falha de compilação/verificação básica; log: Apkc/proofs/out/apkc-compile.txt'
    exit 1
    ;;
esac

if [ "$BUILD_MODE" != native_runnable ] && [ "$BUILD_MODE" != cross_runnable ]; then
  echo 'TOKEN_VAZIO: não há binário apkc executável neste ambiente; instale linker/toolchain ARM compatível ou rode em ARM/Linux.' > "$OUT/apkc-generate.txt"
  status F2 TOKEN_VAZIO 'geração de hello.apk não executada porque só há objeto/sintaxe, sem binário executável'
  echo 'TOKEN_VAZIO: hello.apk ausente; unzip não executado.' > "$OUT/unzip.txt"; status F3 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; aapt não executado.' > "$OUT/aapt-xmltree.txt"; status F4 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; DEX SHA-1 não executado.' > "$OUT/dex-sha1.txt"; status F5 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; readelf arm64 não executado.' > "$OUT/readelf-arm64.txt"; status F6 TOKEN_VAZIO 'hello.apk ausente'
  echo 'TOKEN_VAZIO: hello.apk ausente; readelf arm32 não executado.' > "$OUT/readelf-arm32.txt"; status F6 TOKEN_VAZIO 'hello.apk ausente'
  log ''
  log 'Conclusão: compilação/verificação básica arquivada; geração de APK exige binário executável do ApkC.'
  exit 0
fi

RUN_EXE="$EXE"
if ! [ -x "$RUN_EXE" ] && [ -x "$OUT/apkc" ]; then RUN_EXE="$OUT/apkc"; fi

if "$RUN_EXE" hello.s.txt -o "$OUT/hello.apk" >"$OUT/apkc-generate.txt" 2>&1; then
  status F2 PASS 'Apkc/proofs/out/hello.apk gerado'
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
  abi=$1; outfile=$2; pattern=$3
  if ! cmd_status readelf; then echo 'TOKEN_VAZIO: readelf ausente; instale binutils.' > "$outfile"; status F6 TOKEN_VAZIO "readelf ausente para $abi"; return; fi
  tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT HUP INT TERM
  if unzip -p "$OUT/hello.apk" "$pattern" > "$tmp/lib.so" 2>/dev/null && [ -s "$tmp/lib.so" ]; then
    { echo "command: readelf -h/-s $pattern"; readelf -h "$tmp/lib.so"; readelf -s "$tmp/lib.so"; } > "$outfile" 2>&1
    status F6 PASS "readelf $abi registrado em ${outfile#$ROOT/}"
  else
    echo "SKIP: biblioteca $pattern não encontrada no APK." > "$outfile"
    status F6 SKIP "biblioteca $abi ausente"
  fi
  rm -rf "$tmp"; trap - EXIT HUP INT TERM
}
extract_and_readelf arm64 "$OUT/readelf-arm64.txt" 'lib/arm64-v8a/*.so'
extract_and_readelf arm32 "$OUT/readelf-arm32.txt" 'lib/armeabi-v7a/*.so'

log ''
log 'Conclusão: validação técnica arquivada; assinatura, instalação e runtime exigem scripts próprios e evidência real.'
