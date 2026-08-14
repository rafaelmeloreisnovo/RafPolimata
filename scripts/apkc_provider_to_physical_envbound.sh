#!/usr/bin/env bash
set -euo pipefail
ARTIFACT=${1:-}
OUT_DIR=${2:-}
MODE=${3:-probe-only}
[ -n "$ARTIFACT" ] && [ -n "$OUT_DIR" ] || {
  echo "usage: $0 ARTIFACT_ZIP OUT_DIR [extract-only|probe-only|adb-shell-pm|pm-local]" >&2
  exit 201
}
case "$MODE" in extract-only|probe-only|adb-shell-pm|pm-local) ;; *) echo "envbound_status=FAIL reason=UNKNOWN_MODE claim_allowed=false" >&2; exit 202;; esac

ROOT=${APKC_ENVBOUND_ROOT:-${APKC_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}}
CHAIN=${APKC_ENVBOUND_CHAIN:-$ROOT/scripts/apkc_provider_to_physical_codebound.sh}
SEAL=${APKC_ENVBOUND_SEALER:-$ROOT/scripts/apkc_seal_receipt_tree.sh}
CC_BIN=${APKC_ENV_CC_BIN:-${CC:-cc}}
ADB_BIN=${APKC_ENV_ADB_BIN:-${APKC_ADB_BIN:-adb}}

if [ -e "$OUT_DIR" ] && [ -n "$(find "$OUT_DIR" -mindepth 1 -print -quit 2>/dev/null)" ]; then
  echo "envbound_status=FAIL reason=OUT_DIR_NOT_EMPTY claim_allowed=false" >&2
  exit 203
fi
mkdir -p "$OUT_DIR"
STATUS="$OUT_DIR/envbound.status"
ENVF="$OUT_DIR/measurement-environment.txt"
TOOL_MANIFEST="$OUT_DIR/measurement-tools.sha256"
TOOL_VERIFY="$OUT_DIR/measurement-tools-verify.txt"
EXEC_DIR="$OUT_DIR/execution"

fail(){ printf 'envbound_status=FAIL\nmode=%s\nreason=%s\nclaim_allowed=false\n' "$MODE" "$1" > "$STATUS"; exit "$2"; }
command -v sha256sum >/dev/null 2>&1 || fail SHA256SUM_MISSING 204
[ -f "$CHAIN" ] || fail CODEBOUND_CHAIN_MISSING 205
[ -f "$SEAL" ] || fail SEALER_MISSING 206

resolve_tool(){
  local name=$1 p
  p=$(command -v "$name" 2>/dev/null || true)
  [ -n "$p" ] || return 1
  if command -v readlink >/dev/null 2>&1; then p=$(readlink -f "$p" 2>/dev/null || printf '%s' "$p"); fi
  [ -f "$p" ] || return 1
  printf '%s' "$p"
}
one_line(){ "$@" 2>&1 | sed -n '1{s/[[:cntrl:]]/ /g;p;q;}'; }

BASH_PATH=$(resolve_tool bash) || fail BASH_BINARY_UNRESOLVED 207
SHA_PATH=$(resolve_tool sha256sum) || fail SHA256SUM_BINARY_UNRESOLVED 208
CC_PATH=$(resolve_tool "$CC_BIN") || fail COMPILER_BINARY_UNRESOLVED 209
ADB_PATH=TOKEN_VAZIO
if [ "$MODE" = adb-shell-pm ]; then ADB_PATH=$(resolve_tool "$ADB_BIN") || fail ADB_BINARY_UNRESOLVED 210; fi

: > "$TOOL_MANIFEST"
for p in "$BASH_PATH" "$SHA_PATH" "$CC_PATH"; do sha256sum "$p" >> "$TOOL_MANIFEST"; done
if [ "$ADB_PATH" != TOKEN_VAZIO ]; then sha256sum "$ADB_PATH" >> "$TOOL_MANIFEST"; fi
sha256sum -c "$TOOL_MANIFEST" > "$TOOL_VERIFY" 2>&1 || fail TOOL_BINARY_VERIFY_FAILED 211

machine=$(uname -m 2>/dev/null || printf TOKEN_VAZIO)
uname_full=$(uname -a 2>/dev/null || printf TOKEN_VAZIO)
bash_version=$(one_line "$BASH_PATH" --version || printf TOKEN_VAZIO)
sha_version=$(one_line "$SHA_PATH" --version || printf TOKEN_VAZIO)
cc_version=$(one_line "$CC_PATH" --version || printf TOKEN_VAZIO)
adb_version=TOKEN_VAZIO
if [ "$ADB_PATH" != TOKEN_VAZIO ]; then adb_version=$(one_line "$ADB_PATH" version || printf TOKEN_VAZIO); fi
repo_commit=TOKEN_VAZIO
repo_dirty=TOKEN_VAZIO
if command -v git >/dev/null 2>&1 && git -C "$ROOT" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  repo_commit=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || printf TOKEN_VAZIO)
  if [ -n "$(git -C "$ROOT" status --porcelain --untracked-files=no 2>/dev/null)" ]; then repo_dirty=true; else repo_dirty=false; fi
fi
{
  printf 'environment_provenance_schema=raf.apkc.measurement-environment.v1\n'
  printf 'mode=%s\n' "$MODE"
  printf 'machine=%s\n' "$machine"
  printf 'uname=%s\n' "$uname_full"
  printf 'bash_path=%s\n' "$BASH_PATH"
  printf 'bash_version=%s\n' "$bash_version"
  printf 'sha256sum_path=%s\n' "$SHA_PATH"
  printf 'sha256sum_version=%s\n' "$sha_version"
  printf 'compiler_path=%s\n' "$CC_PATH"
  printf 'compiler_version=%s\n' "$cc_version"
  printf 'adb_path=%s\n' "$ADB_PATH"
  printf 'adb_version=%s\n' "$adb_version"
  printf 'repo_commit=%s\n' "$repo_commit"
  printf 'repo_dirty=%s\n' "$repo_dirty"
  printf 'claim_allowed=false\n'
} > "$ENVF"

printf 'envbound_status=IN_PROGRESS\nmode=%s\nclaim_allowed=false\n' "$MODE" > "$STATUS"
set +e
APKC_ROOT="$ROOT" APKC_ADB_BIN="$ADB_BIN" CC="$CC_BIN" bash "$CHAIN" "$ARTIFACT" "$EXEC_DIR" "$MODE"
chain_rc=$?
set -e
if [ "$chain_rc" -eq 0 ]; then
  printf 'envbound_status=PASS\nmode=%s\nchain_exit=0\nclaim_allowed=false\n' "$MODE" > "$STATUS"
else
  printf 'envbound_status=FAIL\nmode=%s\nchain_exit=%s\nreason=CHAIN_REJECTED\nclaim_allowed=false\n' "$MODE" "$chain_rc" > "$STATUS"
fi

bash "$SEAL" "$OUT_DIR" environment-bound-tree.sha256 || exit 212
(cd "$OUT_DIR" && sha256sum -c environment-bound-tree.sha256 >/dev/null) || exit 213
[ "$chain_rc" -eq 0 ] || exit "$chain_rc"
exit 0
