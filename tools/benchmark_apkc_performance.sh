#!/bin/bash
# tools/benchmark_apkc_performance.sh
#
# Phase C Component 6: Benchmark & Performance Baseline
# Captures: compilation time, memory usage, throughput
# Outputs: Structured receipt with metrics and SLA compliance check
#
# Usage:
#   ./tools/benchmark_apkc_performance.sh <source_file> [iterations]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
APKC_BIN="${REPO_ROOT}/Apkc/apkc"
SOURCE_FILE="${1:?Usage: $0 <source_file> [iterations]}"
ITERATIONS="${2:-5}"
TIMESTAMP=$(date -u +"%Y%m%d_%H%M%S")
RECEIPT_DIR="${REPO_ROOT}/docs/proofs"
RECEIPT_FILE="${RECEIPT_DIR}/BENCHMARK_${TIMESTAMP}.json"

mkdir -p "$RECEIPT_DIR"

echo "[INFO] Starting performance benchmark..."
echo "[INFO] Source: $SOURCE_FILE"
echo "[INFO] Iterations: $ITERATIONS"

# Verify source
if [ ! -f "$SOURCE_FILE" ]; then
    echo "[ERROR] Source file not found: $SOURCE_FILE"
    exit 1
fi

# Run benchmarks
declare -a TIMES
declare -a SIZES

for i in $(seq 1 $ITERATIONS); do
    output_file="/tmp/benchmark_${i}.apk"

    # Time the compilation
    start_time=$(date +%s%N)
    if "$APKC_BIN" -i "$SOURCE_FILE" -o "$output_file" >/dev/null 2>&1; then
        end_time=$(date +%s%N)
        duration_ms=$(( (end_time - start_time) / 1000000 ))

        size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

        TIMES[$i]=$duration_ms
        SIZES[$i]=$size

        echo "[RUN $i] Time: ${duration_ms}ms, Size: $size bytes"

        rm -f "$output_file"
    else
        echo "[ERROR] Build $i failed"
        exit 1
    fi
done

# Calculate statistics
total_time=0
min_time=${TIMES[1]}
max_time=${TIMES[1]}

for time in "${TIMES[@]}"; do
    total_time=$((total_time + time))
    [ $time -lt $min_time ] && min_time=$time
    [ $time -gt $max_time ] && max_time=$time
done

avg_time=$((total_time / ITERATIONS))

# Create JSON receipt
cat > "$RECEIPT_FILE" << EOF
{
  "metadata": {
    "timestamp": "$(date -u +'%Y-%m-%dT%H:%M:%SZ')",
    "source_file": "$(basename $SOURCE_FILE)",
    "apkc_version": "$("$APKC_BIN" --version 2>/dev/null || echo unknown)"
  },
  "benchmark": {
    "iterations": $ITERATIONS,
    "compilation_time_ms": {
      "min": $min_time,
      "max": $max_time,
      "average": $avg_time,
      "total": $total_time
    },
    "throughput": {
      "files_per_second": $((1000 * ITERATIONS / total_time))
    },
    "sla_check": {
      "target_ms_per_build": 1000,
      "actual_avg_ms": $avg_time,
      "compliant": $([ $avg_time -le 1000 ] && echo true || echo false)
    }
  }
}
EOF

echo "[INFO] Benchmark complete"
echo "[INFO] Receipt: $RECEIPT_FILE"
echo "[INFO] Average: ${avg_time}ms"
echo "[INFO] SLA compliant: $([ $avg_time -le 1000 ] && echo YES || echo NO)"
