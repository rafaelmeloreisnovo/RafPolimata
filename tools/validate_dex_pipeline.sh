#!/usr/bin/env bash
# L4 Java/Kotlin -> JVM class -> DEX validation.
# Exit 0=observed PASS, 1=falsified with available toolchain, 2=TOKEN_VAZIO external capability.
set -u -o pipefail

SOURCE_FILE="${1:-}"
[ -n "$SOURCE_FILE" ] || { echo "Usage: $0 <java_or_kt_file>" >&2; exit 64; }
[ -f "$SOURCE_FILE" ] || { echo "L4 FAIL source_missing=$SOURCE_FILE" >&2; exit 1; }

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
TIMESTAMP="$(date -u +%Y%m%dT%H%M%SZ)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
OUT_DIR="$TMP/dex"
CLASS_DIR="$TMP/classes"
mkdir -p "$OUT_DIR" "$CLASS_DIR" docs/proofs
RECEIPT="docs/proofs/L4_DEX_VALIDATION_${TIMESTAMP}.json"

has() { command -v "$1" >/dev/null 2>&1; }
find_sdk_tool() {
  local name="$1"
  if has "$name"; then command -v "$name"; return 0; fi
  local sdk="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
  [ -n "$sdk" ] || return 1
  [ -d "$sdk/build-tools" ] || return 1
  find "$sdk/build-tools" -type f -name "$name" -perm -u+x 2>/dev/null | sort -V | tail -n 1
}

EXT="${SOURCE_FILE##*.}"
COMPILER=""
case "$EXT" in
  java)
    if ! has javac; then
      cat >"$RECEIPT" <<JSON
{"schema":"rafpolimata.l4.dex.v2","state":"TOKEN_VAZIO_TOOLCHAIN","missing":"javac","source":"$(basename "$SOURCE_FILE")","claim_allowed":false}
JSON
      echo "L4 TOKEN_VAZIO_TOOLCHAIN missing=javac receipt=$RECEIPT"
      exit 2
    fi
    COMPILER="javac"
    ;;
  kt)
    if ! has kotlinc; then
      cat >"$RECEIPT" <<JSON
{"schema":"rafpolimata.l4.dex.v2","state":"TOKEN_VAZIO_TOOLCHAIN","missing":"kotlinc","source":"$(basename "$SOURCE_FILE")","claim_allowed":false}
JSON
      echo "L4 TOKEN_VAZIO_TOOLCHAIN missing=kotlinc receipt=$RECEIPT"
      exit 2
    fi
    COMPILER="kotlinc"
    ;;
  *)
    echo "L4 FAIL unsupported_source_extension=$EXT" >&2
    exit 1
    ;;
esac

DEX="$(find_sdk_tool d8 2>/dev/null || find_sdk_tool dx 2>/dev/null || true)"
if [ -z "$DEX" ]; then
  cat >"$RECEIPT" <<JSON
{"schema":"rafpolimata.l4.dex.v2","state":"TOKEN_VAZIO_TOOLCHAIN","missing":"d8_or_dx","source":"$(basename "$SOURCE_FILE")","claim_allowed":false}
JSON
  echo "L4 TOKEN_VAZIO_TOOLCHAIN missing=d8_or_dx receipt=$RECEIPT"
  exit 2
fi

if [ "$COMPILER" = javac ]; then
  if ! javac -d "$CLASS_DIR" "$SOURCE_FILE"; then
    echo "L4 FAIL javac_compile" >&2; exit 1
  fi
else
  if ! kotlinc "$SOURCE_FILE" -d "$CLASS_DIR"; then
    echo "L4 FAIL kotlinc_compile" >&2; exit 1
  fi
fi

mapfile -t CLASSES < <(find "$CLASS_DIR" -type f -name '*.class' -print | sort)
if [ ${#CLASSES[@]} -eq 0 ]; then
  echo "L4 FAIL no_class_files_generated" >&2
  exit 1
fi

DEX_FILE="$OUT_DIR/classes.dex"
case "$(basename "$DEX")" in
  d8*)
    if ! "$DEX" --output "$OUT_DIR" "${CLASSES[@]}"; then echo "L4 FAIL d8_compile" >&2; exit 1; fi
    ;;
  dx*)
    if ! "$DEX" --dex --output="$DEX_FILE" "${CLASSES[@]}"; then echo "L4 FAIL dx_compile" >&2; exit 1; fi
    ;;
  *) echo "L4 FAIL unknown_dex_compiler=$DEX" >&2; exit 1;;
esac

[ -f "$DEX_FILE" ] || { echo "L4 FAIL dex_not_generated" >&2; exit 1; }
# DEX header is: 'dex\n' + three ASCII version bytes + NUL.
MAGIC_HEX="$(od -An -tx1 -N8 "$DEX_FILE" | tr -d ' \n')"
case "$MAGIC_HEX" in
  6465780a3033[0-9a-f][0-9a-f]00|6465780a3034[0-9a-f][0-9a-f]00)
    ;;
  *) echo "L4 FAIL invalid_dex_magic=$MAGIC_HEX" >&2; exit 1;;
esac

SIZE="$(wc -c <"$DEX_FILE" | tr -d ' ')"
SHA="$(sha256sum "$DEX_FILE" | awk '{print $1}')"
CLASS_COUNT="${#CLASSES[@]}"
cat >"$RECEIPT" <<JSON
{
  "schema":"rafpolimata.l4.dex.v2",
  "timestamp_utc":"$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "source":"$(basename "$SOURCE_FILE")",
  "source_language":"$EXT",
  "class_compiler":"$COMPILER",
  "dex_compiler":"$(basename "$DEX")",
  "class_files":$CLASS_COUNT,
  "dex_bytes":$SIZE,
  "dex_sha256":"$SHA",
  "dex_magic_hex":"$MAGIC_HEX",
  "state":"PASS_GENERATED_DEX",
  "device_execution":"TOKEN_VAZIO_NOT_PART_OF_L4_STATIC_PIPELINE",
  "claim_allowed":false
}
JSON

echo "L4 PASS generated_dex bytes=$SIZE sha256=$SHA receipt=$RECEIPT"
exit 0
