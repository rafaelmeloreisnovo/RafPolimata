#!/usr/bin/env sh
# Convert one Groovy source into a JAR suitable for ApkC's subsequent d8 stage.
set -eu

OUT=${1:?output jar required}
SRC=${2:?Groovy source required}
TMP_BASE=${TMPDIR:-/data/data/com.termux/files/usr/tmp}
WORK="$TMP_BASE/apkc-groovy-$$"
CLASSES="$WORK/classes"

cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT HUP INT TERM
mkdir -p "$CLASSES"

groovyc -encoding UTF-8 -d "$CLASSES" "$SRC"
jar --create --file "$OUT" -C "$CLASSES" .

[ -s "$OUT" ] || {
  echo "apkc-groovy: JAR vazio" >&2
  exit 1
}
