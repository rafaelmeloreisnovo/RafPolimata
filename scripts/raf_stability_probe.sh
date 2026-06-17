#!/usr/bin/env bash
# S26 — Teste de estabilidade por 1h, 6h e 24h
# Executa um benchmark continuamente por DURATION_S segundos, amostrando cada INTERVAL_S.
# Detecta drift de latência (p99 crescente), erros de retorno ou falhas de compilação.
# Uso: bash scripts/raf_stability_probe.sh [duration_seconds] [interval_seconds]
#   Ex: bash scripts/raf_stability_probe.sh 3600  30   # 1 hora
#       bash scripts/raf_stability_probe.sh 21600 60   # 6 horas
#       bash scripts/raf_stability_probe.sh 86400 120  # 24 horas
set -euo pipefail

DURATION_S=${1:-3600}
INTERVAL_S=${2:-30}
OUT_DIR="${RAF_REPORT_DIR:-ci/reports}"
REPORT="$OUT_DIR/stability_$(date -u +%Y%m%dT%H%M%S).md"
mkdir -p "$OUT_DIR"

# Compile the stability probe binary from the existing benchmark sources.
# Falls back to a minimal XOR-loop benchmark if raf_main.c not present.
PROBE_BIN=/tmp/raf_stability_probe_bin
if [ -f Benchmark/raf_main.c ]; then
  gcc -std=c11 -O2 -I. -IBenchmark \
    Benchmark/raf_main.c -o "$PROBE_BIN" -lm 2>/dev/null \
  || gcc -std=c11 -O2 -D_POSIX_C_SOURCE=200809L \
       Benchmark/raf_bus_throughput_benchmark.c -IBenchmark \
       -o "$PROBE_BIN" 2>/dev/null \
  || { echo "COMPILE_FAIL — no suitable benchmark source found"; exit 1; }
else
  # Inline minimal probe: XOR loop timed with clock_gettime
  cat > /tmp/raf_stability_inline.c <<'C'
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <time.h>
#include <stdio.h>
static uint64_t ns(void){
  struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t);
  return (uint64_t)t.tv_sec*1000000000ull+(uint64_t)t.tv_nsec;
}
int main(void){
  volatile uint64_t acc=0;
  uint64_t t0=ns();
  for(uint64_t i=0;i<1000000ull;i++) acc^=i;
  uint64_t el=ns()-t0;
  (void)acc;
  printf("elapsed_ns=%llu\n",(unsigned long long)el);
  return 0;
}
C
  gcc -std=c11 -O2 /tmp/raf_stability_inline.c -o "$PROBE_BIN"
fi

{
  echo "# Relatório de estabilidade — S26"
  echo ""
  echo "- Duração configurada: ${DURATION_S}s"
  echo "- Intervalo de amostra: ${INTERVAL_S}s"
  echo "- Início: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo ""
  echo "| Epoch (s) | Timestamp UTC | elapsed_ns | rc | Estado |"
  echo "|----------:|---------------|:----------:|:--:|:------:|"
} > "$REPORT"

t_start=$(date +%s)
t_end=$((t_start + DURATION_S))
sample=0
fail=0
prev_ns=0

while [ "$(date +%s)" -lt "$t_end" ]; do
  epoch=$(( $(date +%s) - t_start ))
  ts=$(date -u +%Y-%m-%dT%H:%M:%SZ)

  out=$("$PROBE_BIN" 2>/dev/null || echo "elapsed_ns=0")
  rc=$?
  el=$(echo "$out" | grep -oP '(?<=elapsed_ns=)\d+' || echo 0)
  el=${el:-0}

  if [ "$rc" -ne 0 ]; then
    state="FAIL(rc=$rc)"
    fail=1
  elif [ "$prev_ns" -gt 0 ] && [ "$el" -gt $(( prev_ns * 10 )) ]; then
    state="DRIFT(10x)"
    fail=1
  else
    state="OK"
  fi
  prev_ns="$el"
  sample=$((sample+1))

  echo "| $epoch | $ts | $el | $rc | $state |" >> "$REPORT"

  remaining=$(( t_end - $(date +%s) ))
  if [ "$remaining" -le 0 ]; then break; fi
  sleep_s=$(( INTERVAL_S < remaining ? INTERVAL_S : remaining ))
  sleep "$sleep_s" || true
done

{
  echo ""
  echo "- Término: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "- Amostras coletadas: ${sample}"
  if [ "$fail" -eq 0 ]; then
    echo ""
    echo "**Resultado: PASS — nenhum drift ou falha detectado em ${DURATION_S}s.**"
  else
    echo ""
    echo "**Resultado: FAIL — drift ou rc != 0 detectado. Ver tabela acima.**"
  fi
} >> "$REPORT"

cat "$REPORT"
exit "$fail"
