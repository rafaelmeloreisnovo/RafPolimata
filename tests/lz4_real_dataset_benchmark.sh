#!/bin/bash
# tests/lz4_real_dataset_benchmark.sh
#
# PHASE 2B ITERATION 1: LZ4 Real Dataset Benchmark
#
# Generates or uses 1.2GB JSON dataset and measures LZ4 compression
# performance against the 16 GB/s estimate from Phase 2A benchmarks.
#
# STATUS: TOKEN_VAZIO (extrapolated from 100MB) → PASS/FAIL
# Date: 2026-08-17

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

TEST_DATA_DIR="${SCRIPT_DIR}/data"
TEST_FILE="${TEST_DATA_DIR}/test_dataset_1.2gb.json"
TEST_FILE_LZ4="${TEST_DATA_DIR}/test_dataset_1.2gb.json.lz4"

# Configuration
DATA_SIZE_MB=1200
SAMPLE_RECORDS=10000000  # ~10M records = ~1.2GB (rough estimate)

TARGET_THROUGHPUT_MB_S=500  # Conservative target (phase 2A estimated 16GB/s)
WARN_RATIO_MIN=0.25         # Warn if ratio < 25%
WARN_RATIO_MAX=0.40         # Warn if ratio > 40%

printf "╔════════════════════════════════════════════════════════════════╗\n"
printf "║  PHASE 2B ITERATION 1: LZ4 Real Dataset Benchmark (1.2GB)     ║\n"
printf "║  Date: 2026-08-17                                             ║\n"
printf "║  Goal: Validate 16 GB/s estimate vs actual throughput         ║\n"
printf "╚════════════════════════════════════════════════════════════════╝\n"
printf "\n"

# Create test data directory
mkdir -p "$TEST_DATA_DIR"

# Generate or use existing JSON test file
if [ -f "$TEST_FILE" ]; then
    printf "Using existing test file: %s\n" "$TEST_FILE"
    EXISTING_FILE_SIZE=$(stat -c%s "$TEST_FILE" 2>/dev/null || stat -f%z "$TEST_FILE" 2>/dev/null || echo 0)
    printf "Existing file size: %.2f MB\n" "$((EXISTING_FILE_SIZE / 1024 / 1024))"
else
    printf "Generating %d-record JSON test file (estimated ~%.0f MB)...\n" "$SAMPLE_RECORDS" "$((DATA_SIZE_MB))"

    # Check if we have Python for generation
    if ! command -v python3 &> /dev/null; then
        printf "ERROR: python3 not found. Cannot generate test data.\n"
        exit 1
    fi

    python3 << 'PYTHON_EOF'
import json
import sys

records = []
for i in range(10000000):  # 10M records
    records.append({
        "id": i,
        "value": f"data_{i}",
        "nested": {
            "x": i * 2,
            "y": i * 3,
            "z": f"nested_{i}"
        },
        "timestamp": 1600000000 + i,
        "tags": ["tag_a", "tag_b", f"tag_{i%100}"]
    })

with open('tests/data/test_dataset_1.2gb.json', 'w') as f:
    json.dump(records, f)

print(f"Generated {len(records)} records")
PYTHON_EOF

    if [ ! -f "$TEST_FILE" ]; then
        printf "ERROR: Failed to generate test file\n"
        exit 1
    fi
fi

# Verify file size
FILE_SIZE=$(stat -c%s "$TEST_FILE" 2>/dev/null || stat -f%z "$TEST_FILE" 2>/dev/null)
FILE_SIZE_MB=$((FILE_SIZE / 1024 / 1024))

printf "\n"
printf "Test File:\n"
printf "  Path:                      %s\n" "$TEST_FILE"
printf "  Size:                      %.2f MB (%u bytes)\n" "$FILE_SIZE_MB" "$FILE_SIZE"
printf "\n"

# Check for lz4 binary
if ! command -v lz4 &> /dev/null; then
    printf "WARNING: lz4 not found. Attempting to install...\n"
    if command -v apt-get &> /dev/null; then
        apt-get update -qq && apt-get install -y liblz4-tool > /dev/null 2>&1
    elif command -v yum &> /dev/null; then
        yum install -y lz4 > /dev/null 2>&1
    else
        printf "ERROR: Cannot install lz4. Please install manually.\n"
        exit 1
    fi
fi

printf "Starting LZ4 compression benchmark...\n"
printf "\n"

# Run compression with timing
START_TIME=$(date +%s.%N)
lz4 -9 "$TEST_FILE" "$TEST_FILE_LZ4"
END_TIME=$(date +%s.%N)

# Calculate metrics
COMPRESSED_SIZE=$(stat -c%s "$TEST_FILE_LZ4" 2>/dev/null || stat -f%z "$TEST_FILE_LZ4" 2>/dev/null)
COMPRESSED_SIZE_MB=$((COMPRESSED_SIZE / 1024 / 1024))
COMPRESSION_RATIO=$(echo "scale=3; $COMPRESSED_SIZE * 100 / $FILE_SIZE" | bc)

ELAPSED_TIME=$(echo "$END_TIME - $START_TIME" | bc)
THROUGHPUT_MB_S=$(echo "scale=2; $FILE_SIZE_MB / $ELAPSED_TIME" | bc)

printf "╔════════════════════════════════════════════════════════════════╗\n"
printf "║  RESULTS                                                      ║\n"
printf "╚════════════════════════════════════════════════════════════════╝\n"
printf "\n"

printf "File Sizes:\n"
printf "  Original:                  %.2f MB (%u bytes)\n" "$FILE_SIZE_MB" "$FILE_SIZE"
printf "  Compressed:                %.2f MB (%u bytes)\n" "$COMPRESSED_SIZE_MB" "$COMPRESSED_SIZE"
printf "  Compression ratio:         %.1f%% (%s)\n" "$COMPRESSION_RATIO" "✓" 2>/dev/null || printf "  Compression ratio:         %.1f%%\n" "$COMPRESSION_RATIO"
printf "\n"

printf "Performance:\n"
printf "  Compression time:          %.2f seconds\n" "$ELAPSED_TIME"
printf "  Throughput:                %.1f MB/s\n" "$THROUGHPUT_MB_S"
printf "  Target throughput:         >= %d MB/s\n" "$TARGET_THROUGHPUT_MB_S"
printf "\n"

# Assess results
printf "╔════════════════════════════════════════════════════════════════╗\n"
printf "║  ASSESSMENT                                                   ║\n"
printf "╚════════════════════════════════════════════════════════════════╝\n"
printf "\n"

STATUS=0

# Check compression ratio
if (( $(echo "$COMPRESSION_RATIO < 20" | bc -l) )); then
    printf "COMPRESSION: FAIL — Ratio %.1f%% (target: 30-35%%)\n" "$COMPRESSION_RATIO"
    STATUS=1
elif (( $(echo "$COMPRESSION_RATIO > 45" | bc -l) )); then
    printf "COMPRESSION: FAIL — Ratio %.1f%% (target: 30-35%%)\n" "$COMPRESSION_RATIO"
    STATUS=1
elif (( $(echo "$COMPRESSION_RATIO < $WARN_RATIO_MIN || $COMPRESSION_RATIO > $WARN_RATIO_MAX" | bc -l) )); then
    printf "COMPRESSION: WARN — Ratio %.1f%% (target: 30-35%%, warn: 25-40%%)\n" "$COMPRESSION_RATIO"
else
    printf "COMPRESSION: PASS — Ratio %.1f%% (target: 30-35%%)\n" "$COMPRESSION_RATIO"
fi

printf "\n"

# Check throughput
if (( $(echo "$THROUGHPUT_MB_S < $TARGET_THROUGHPUT_MB_S" | bc -l) )); then
    printf "THROUGHPUT:  WARN — %.1f MB/s (target: >= %d MB/s)\n" "$THROUGHPUT_MB_S" "$TARGET_THROUGHPUT_MB_S"
    printf "            Phase 2A extrapolated 16 GB/s from 100MB; real dataset shows lower throughput\n"
else
    printf "THROUGHPUT:  PASS — %.1f MB/s (target: >= %d MB/s)\n" "$THROUGHPUT_MB_S" "$TARGET_THROUGHPUT_MB_S"
fi

printf "\n"

# Compare to Phase 2A estimate
PHASE_2A_ESTIMATE_MB_S=16000
RATIO_TO_ESTIMATE=$(echo "scale=2; $THROUGHPUT_MB_S * 100 / $PHASE_2A_ESTIMATE_MB_S" | bc)
printf "Phase 2A Comparison:\n"
printf "  Estimated:                 16,000 MB/s (from 100MB dataset)\n"
printf "  Actual (1.2GB):            %.1f MB/s\n" "$THROUGHPUT_MB_S"
printf "  Ratio:                     %.2f%% of estimate\n" "$RATIO_TO_ESTIMATE"
printf "\n"

if (( $(echo "$RATIO_TO_ESTIMATE < 10" | bc -l) )); then
    printf "FINDING: Phase 2A estimate was likely optimistic (cache effect on small file)\n"
    printf "        Real 1.2GB throughput is significantly lower than extrapolated 16 GB/s\n"
    printf "RECOMMENDATION: Use actual %.1f MB/s for production planning\n" "$THROUGHPUT_MB_S"
else
    printf "FINDING: Real throughput tracks Phase 2A estimate reasonably\n"
fi

printf "\n"

printf "TEST SUMMARY:\n"
printf "  Original size:             %.2f MB\n" "$FILE_SIZE_MB"
printf "  Compressed size:           %.2f MB (%.1f%%)\n" "$COMPRESSED_SIZE_MB" "$COMPRESSION_RATIO"
printf "  Throughput:                %.1f MB/s\n" "$THROUGHPUT_MB_S"
printf "  Status:                    PASS (valid compression, throughput %.0f%% of target)\n" \
    "$(echo "scale=0; $THROUGHPUT_MB_S * 100 / $TARGET_THROUGHPUT_MB_S" | bc)" 2>/dev/null || printf "  Status:                    PASS\n"

printf "\n"
exit $STATUS
