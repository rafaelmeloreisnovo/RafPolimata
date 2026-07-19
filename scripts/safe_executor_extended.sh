#!/usr/bin/env sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
MODE=${1:-ci}
OUT_BASE=${SAFE_EXTENDED_OUT:-"$ROOT/build/safe-extended"}
KEEP_RUNS=${SAFE_EXTENDED_KEEP_RUNS:-8}
ALLOW_ROOT=${SAFE_EXTENDED_ALLOW_ROOT:-0}
CC=${CC:-}
[ -n "$CC" ] || { command -v clang >/dev/null 2>&1 && CC=clang || CC=cc; }

export LC_ALL=C TZ=UTC
umask 077

usage() {
cat <<'TXT'
Safe Executor Extended — CI local/offline do RafPolimata

  sh safe-extended plan
  sh safe-extended build
  sh safe-extended run
  sh safe-extended ci
  sh safe-extended ci-full
  sh safe-extended status
  sh safe-extended clean

plan     mostra o grafo fixo
build    audita e compila o Omega para a ABI Android atual
run      compila e executa no Termux/Android
ci       executa Omega + compilador RafPolimata
ci-full  executa ci + runtime-truth local
status   mostra o último relatório
clean    remove saídas e lock

Sem rede, GitHub, eval, sudo, root ou YAML executável.
TXT
}
die(){ printf 'SAFE_EXTENDED FAIL: %s\n' "$1" >&2; exit "${2:-1}"; }
epoch(){ date +%s; }
arch(){ uname -m 2>/dev/null || printf unknown; }
termux(){ [ -n "${TERMUX_VERSION:-}" ] || [ -d /data/data/com.termux/files/usr ]; }

case "$MODE" in
 plan|build|run|ci|ci-full|status|clean) ;;
 help|-h|--help) usage; exit 0 ;;
 *) usage >&2; exit 2 ;;
esac

case "$MODE" in
 clean)
   rm -rf "$OUT_BASE" "$ROOT/build/.safe-extended.lock"
   echo 'SAFE_EXTENDED CLEAN PASS'; exit 0 ;;
 status)
   [ -s "$OUT_BASE/latest-report.json" ] || die 'nenhum relatório local' 4
   cat "$OUT_BASE/latest-report.json"; exit 0 ;;
 plan)
   cat <<TXT
schema=rafpolimata.safe_executor_extended.v1
root=$ROOT
host_arch=$(arch)
network=DENY
github=NOT_USED
eval=DENY
root_execution=DENY
build=preflight,source_audit,omega_host_gate,omega_termux_build
run=build,omega_termux_run
ci=run,compiler_strict_build,compiler_smoke
ci-full=ci,runtime_truth
TXT
   exit 0 ;;
esac

mkdir -p "$ROOT/build" "$OUT_BASE"
LOCK="$ROOT/build/.safe-extended.lock"
mkdir "$LOCK" 2>/dev/null || die "CI local já ativa; verifique $LOCK" 5
trap 'rm -rf "$LOCK"' EXIT HUP INT TERM

RUN_ID="$(date -u '+%Y%m%dT%H%M%SZ')-$$"
RUN="$OUT_BASE/$RUN_ID"
LOG="$RUN/logs"
ART="$RUN/artifacts"
STAGES="$RUN/stages.jsonl"
REPORT="$RUN/report.json"
mkdir -p "$LOG" "$ART"; : > "$STAGES"
START=$(epoch)
LAST_RC=0

record(){
 printf '{"name":"%s","status":"%s","rc":%s,"started":%s,"ended":%s,"log":"logs/%s.log"}\n' \
   "$1" "$2" "$3" "$4" "$5" "$1" >> "$STAGES"
}
stage(){
 n=$1; shift; l="$LOG/$n.log"; a=$(epoch)
 printf '\n=== %s ===\n' "$n"
 set +e; ( set -e; "$@" ) >"$l" 2>&1; r=$?; set -e
 b=$(epoch); cat "$l"
 if [ "$r" -eq 0 ]; then
   record "$n" PASS 0 "$a" "$b"; echo "SAFE_EXTENDED STAGE PASS $n"; LAST_RC=0; return 0
 fi
 record "$n" FAIL "$r" "$a" "$b"; echo "SAFE_EXTENDED STAGE FAIL $n rc=$r" >&2
 LAST_RC=$r; return "$r"
}
finish(){
 result=$1; end=$(epoch)
 {
   printf '{"schema":"rafpolimata.safe_executor_extended.report.v1",'
   printf '"run_id":"%s","mode":"%s","host_arch":"%s",' "$RUN_ID" "$MODE" "$(arch)"
   printf '"network":"DENY","github":"NOT_USED","started":%s,"ended":%s,' "$START" "$end"
   printf '"result":"%s","stages":[' "$result"
   awk 'NR>1{printf ","}{printf "%s",$0}END{printf "\n"}' "$STAGES"
   printf ']}\n'
 } > "$REPORT"
 cp "$REPORT" "$OUT_BASE/latest-report.json"
 echo "SAFE_EXTENDED RESULT $result report=$REPORT"
}
required(){
 n=$1; shift
 if ! stage "$n" "$@"; then r=$LAST_RC; finish FAIL; exit "$r"; fi
}

preflight(){
 [ -f "$ROOT/raf_main.c" ] || { echo "raiz inválida: $ROOT" >&2; return 10; }
 if [ "$ALLOW_ROOT" != 1 ] && [ "$(id -u 2>/dev/null || echo x)" = 0 ]; then
   echo 'root negado; use o usuário normal do Termux' >&2; return 11
 fi
 for f in freestanding/omega/omega_core.c freestanding/omega/omega_core.h \
          scripts/validate_omega_freestanding.sh scripts/termux_freestanding_build.sh; do
   [ -f "$ROOT/$f" ] || { echo "arquivo ausente: $f" >&2; return 12; }
 done
 command -v "$CC" >/dev/null 2>&1 || { echo 'instale clang no Termux' >&2; return 13; }
 printf 'root=%s\narch=%s\ntermux=%s\nnetwork=DENY\ngithub=NOT_USED\n' \
   "$ROOT" "$(arch)" "$(termux && echo yes || echo no)"
}
source_audit(){
 if grep -REn '^[[:space:]]*(curl|wget|git[[:space:]]+(clone|fetch|pull)|gh[[:space:]]|sudo|su[[:space:]])' \
   "$ROOT/scripts/safe_executor_extended.sh" \
   "$ROOT/scripts/validate_omega_freestanding.sh" \
   "$ROOT/scripts/termux_freestanding_build.sh" >"$ART/forbidden-ops.txt" 2>/dev/null; then
   cat "$ART/forbidden-ops.txt"; return 20
 fi
 sh -n "$ROOT/scripts/safe_executor_extended.sh"
 sh -n "$ROOT/scripts/validate_omega_freestanding.sh"
 sh -n "$ROOT/scripts/termux_freestanding_build.sh"
 echo source_audit=PASS
}
omega_host(){
 OUT_DIR="$ART/omega-host" REPORT_PATH="$ART/omega-freestanding.md" CC="$CC" \
   sh "$ROOT/scripts/validate_omega_freestanding.sh"
}
omega_build(){
 termux || { echo 'Termux/Android obrigatório' >&2; return 30; }
 OUT_DIR="$ART/omega-termux" CC="$CC" sh "$ROOT/scripts/termux_freestanding_build.sh" --build
}
omega_run(){
 termux || { echo 'Termux/Android obrigatório' >&2; return 31; }
 OUT_DIR="$ART/omega-termux" CC="$CC" sh "$ROOT/scripts/termux_freestanding_build.sh" --run
}
compiler_build(){
 mkdir -p "$ART/compiler"
 "$CC" -std=c11 -Wall -Wextra -Werror \
   "$ROOT/raf_main.c" "$ROOT/raf_frontend.c" "$ROOT/raf_cpu.c" \
   "$ROOT/raf_asm_emit.c" "$ROOT/raf_precomp.c" -o "$ART/compiler/raf_compile"
}
compiler_smoke(){
 b="$ART/compiler/raf_compile"; [ -x "$b" ] || return 40
 "$b" --help
 printf 'int main(void){return 0;}\n' >"$ART/compiler/input.c"
 "$b" "$ART/compiler/input.c" "$ART/compiler/out" O2 --native
 for x in s hex bin ops; do [ -s "$ART/compiler/out.$x" ] || return 41; done
 grep -qx 'native_requested=1' "$ART/compiler/out.ops"
 grep -qx 'native_written=1' "$ART/compiler/out.ops"
}
runtime_truth(){
 command -v bash >/dev/null 2>&1 || return 50
 command -v make >/dev/null 2>&1 || return 51
 command -v python3 >/dev/null 2>&1 || return 52
 bash "$ROOT/scripts/validate_runtime_truth_local.sh"
}
prune(){
 case "$KEEP_RUNS" in ''|*[!0-9]*) return ;; esac
 [ "$KEEP_RUNS" -gt 0 ] || return
 find "$OUT_BASE" -mindepth 1 -maxdepth 1 -type d 2>/dev/null |
   sort -r | awk -v n="$KEEP_RUNS" 'NR>n' |
   while IFS= read -r d; do rm -rf "$d"; done
}

required preflight preflight
required source_audit source_audit
required omega_host_gate omega_host

case "$MODE" in
 build) required omega_termux_build omega_build ;;
 run)
   required omega_termux_build omega_build
   required omega_termux_run omega_run ;;
 ci|ci-full)
   required omega_termux_build omega_build
   required omega_termux_run omega_run
   required compiler_strict_build compiler_build
   required compiler_smoke compiler_smoke
   [ "$MODE" = ci-full ] && required runtime_truth runtime_truth ;;
esac

finish PASS
prune
