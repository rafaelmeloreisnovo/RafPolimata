#!/usr/bin/env sh
# Convert one Java source into a JAR suitable for ApkC's subsequent d8 stage.
set -eu

OUT=${1:?output jar required}
SRC=${2:?Java source required}
TMP_BASE=${TMPDIR:-/data/data/com.termux/files/usr/tmp}
WORK="$TMP_BASE/apkc-java-$$"
CLASSES="$WORK/classes"

cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT HUP INT TERM
mkdir -p "$CLASSES"

javac -encoding UTF-8 -source 8 -target 8 -d "$CLASSES" "$SRC"
jar --create --file "$OUT" -C "$CLASSES" .

[ -s "$OUT" ] || {
  echo "apkc-java: JAR vazio" >&2
  exit 1
}
