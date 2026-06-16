#!/bin/sh
set -eu
# Closes checklist item S25 — "Criar teste de consumo energético".
#
# This host has no real watt-meter hardware. Per evidence_policy:
# token_vazio_over_false_claim (configs/operational_excellence.yml), this
# script NEVER fabricates a wattage number:
#   - if RAPL (/sys/class/powercap/intel-rapl) energy counters are readable,
#     it reports MEASURED joules/watts around the fixed workload.
#   - else if `perf stat` is usable, it reports MEASURED IPC and cache-miss
#     rate, explicitly labeled "PROXY — not real wattage".
#   - else it emits TOKEN_VAZIO: no energy/power claim of any kind.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT_DIR="$ROOT/ci/reports"
SUMMARY="$OUT_DIR/watt-proxy.md"
EXEC_ROOT="${TMPDIR:-${TMP:-/tmp}}/raf_watt_$$"
mkdir -p "$OUT_DIR" "$EXEC_ROOT"
trap 'rm -rf "$EXEC_ROOT"' EXIT HUP INT TERM

COMMIT=$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)
DATE=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
{
  echo '# Processing-per-watt proxy (S25)'
  echo ''
  echo "- Data UTC: $DATE"
  echo "- Commit: $COMMIT"
  echo ''
} > "$SUMMARY"

log(){ printf '%s\n' "$*" >> "$SUMMARY"; printf '%s\n' "$*"; }

EXE="$EXEC_ROOT/raf_watt_proxy"
BUILD_OK=0
for c in cc gcc clang; do
  if command -v "$c" >/dev/null 2>&1; then
    if "$c" -std=c11 -O2 -I"$ROOT" "$ROOT/tools/raf_watt_proxy.c" -o "$EXE" 2>"$EXEC_ROOT/build.txt"; then
      BUILD_OK=1; break
    fi
  fi
done

if [ "$BUILD_OK" -ne 1 ]; then
  log 'TOKEN_VAZIO: raf_watt_proxy não compilou neste host; nenhuma medição realizada.'
  exit 0
fi
chmod +x "$EXE"

RAPL_BASE=""
for d in /sys/class/powercap/intel-rapl:0 /sys/class/powercap/intel-rapl/intel-rapl:0; do
  if [ -r "$d/energy_uj" ]; then RAPL_BASE="$d"; break; fi
done

PERF_OK=0
if [ -z "$RAPL_BASE" ] && command -v perf >/dev/null 2>&1; then
  if perf stat -e instructions,cycles,cache-misses -- "$EXE" >"$EXEC_ROOT/perf_probe.txt" 2>&1; then
    PERF_OK=1
  fi
fi

if [ -n "$RAPL_BASE" ]; then
  E0=$(cat "$RAPL_BASE/energy_uj")
  "$EXE" > "$EXEC_ROOT/run.txt"
  E1=$(cat "$RAPL_BASE/energy_uj")
  DELTA_UJ=$((E1 - E0))
  log '## MEASURED — RAPL energy counters'
  log ''
  log "- energy_delta_uj: $DELTA_UJ"
  log "- workload: $(cat "$EXEC_ROOT/run.txt")"
  log ''
  log 'Status: MEASURED (real energy delta from intel-rapl). Não é watts diretos'
  log '(falta duração precisa de wall-clock isolada do overhead do script);'
  log 'use junto com elapsed_ns reportado pelo workload para derivar watts médios.'
elif [ "$PERF_OK" -eq 1 ]; then
  log '## PROXY — not real wattage (IPC / cache-miss rate via perf stat)'
  log ''
  log '```'
  cat "$EXEC_ROOT/perf_probe.txt" >> "$SUMMARY"
  cat "$EXEC_ROOT/perf_probe.txt"
  log '```'
  log ''
  log 'Status: PROXY. instructions/cycles (IPC) e cache-misses são um proxy de'
  log 'eficiência computacional, NÃO uma medição de watts. Nenhum valor de'
  log 'consumo energético real é reportado por este host.'
else
  log 'TOKEN_VAZIO: nem RAPL (/sys/class/powercap/intel-rapl) nem `perf stat`'
  log 'disponíveis neste host; nenhuma medição de energia ou proxy de IPC foi'
  log 'realizada. Nenhum número de watts é fabricado.'
fi
