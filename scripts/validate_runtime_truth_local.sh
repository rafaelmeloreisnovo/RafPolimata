#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD="${TMPDIR:-/tmp}/rafpolimata-runtime-truth-$$"
trap 'rm -rf "$BUILD"' EXIT
mkdir -p "$BUILD"
cd "$ROOT"

printf '%s\n' '[1/9] strict host compiler build'
cc -std=c11 -Wall -Wextra -Werror \
  raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c \
  -o "$BUILD/raf_compile"

printf '%s\n' '[2/9] segment v1 header, fixed records and bounded-reader tests'
make -C runtime/conversation_indexer test-segment audit

printf '%s\n' '[3/9] raw native output contract'
printf 'int main(void){return 0;}\n' > "$BUILD/input.c"
"$BUILD/raf_compile" "$BUILD/input.c" "$BUILD/out" O2 --native
for suffix in s hex bin ops; do
  test -s "$BUILD/out.$suffix"
done
grep -qx 'ops_schema=4' "$BUILD/out.ops"
grep -qx 'transaction_state=COMMITTED' "$BUILD/out.ops"
grep -qx 'ir_value=0' "$BUILD/out.ops"
grep -qx 'native_requested=1' "$BUILD/out.ops"
grep -qx 'native_written=1' "$BUILD/out.ops"
python3 scripts/validate_ops_manifest.py "$BUILD/out.ops" --expect-rollback 0

printf '%s\n' '[4/9] optional output-base parsing'
(
  cd "$BUILD"
  "$BUILD/raf_compile" input.c --native
  test -s raf_out.bin
  grep -qx 'native_written=1' raf_out.ops
  grep -qx 'transaction_state=COMMITTED' raf_out.ops
)

printf '%s\n' '[5/9] unknown extension and oversized source rejection'
printf 'not a C source\n' > "$BUILD/input.unknown"
if "$BUILD/raf_compile" "$BUILD/input.unknown" "$BUILD/unknown"; then
  echo 'FAIL unknown extension was accepted as C' >&2
  exit 1
fi
grep -qx 'rollback_code=-6' "$BUILD/unknown.ops"
grep -qx 'transaction_state=ROLLED_BACK' "$BUILD/unknown.ops"
python3 scripts/validate_ops_manifest.py "$BUILD/unknown.ops" --expect-rollback -6

python3 - "$BUILD/oversized.c" <<'PY'
from pathlib import Path
import sys
Path(sys.argv[1]).write_bytes(b'x' * (1024 * 1024))
PY
if "$BUILD/raf_compile" "$BUILD/oversized.c" "$BUILD/oversized"; then
  echo 'FAIL oversized source was silently accepted' >&2
  exit 1
fi
grep -qx 'rollback_code=-5' "$BUILD/oversized.ops"
grep -qx 'transaction_state=ROLLED_BACK' "$BUILD/oversized.ops"
python3 scripts/validate_ops_manifest.py "$BUILD/oversized.ops" --expect-rollback -5

printf '%s\n' '[6/9] source-level honesty invariants'
python3 - <<'PY'
from pathlib import Path

root = Path('.')
main = (root / 'raf_main.c').read_text()
frontend = (root / 'raf_frontend.c').read_text()
cpu = (root / 'raf_cpu.c').read_text()
header = (root / 'raf_compile.h').read_text()
segment_h = (root / 'runtime/conversation_indexer/raf_segment_v1.h').read_text()
segment_c = (root / 'runtime/conversation_indexer/raf_segment_v1.c').read_text()

checks = {
    'native forwarded': 'raf_compile_file(&G, src, out, do_native)' in main,
    'optional output base': 'if (argc > 2 && !is_flag(argv[2]))' in main,
    'native no longer discarded': '(void)do_native' not in frontend,
    'canonical FNV offset': '14695981039346656037' in frontend,
    'transactional schema': '#define RAF_OPS_SCHEMA 4u' in frontend,
    'atomic manifest replacement': 'rename(tmp, ctx->out_ops)' in frontend,
    'source capacity explicit': 'RAF_SOURCE_CAP' in header,
    'unknown language explicit': 'RAF_LANG_UNKNOWN' in header and 'return RAF_LANG_UNKNOWN;' in cpu,
    'presence-only accelerator flags': 'RAF_FEAT_GPU_NODE' in header and '_linux_detect_hw_nodes' in cpu,
    'runtime core count': '_SC_NPROCESSORS_ONLN' in cpu,
    'linux x86 flags parsed': 'static const char x86_key[] = "flags"' in cpu,
    'conversation record frozen': 'RAF_SEGMENT_V1_CONVERSATION_SIZE 96u' in segment_h,
    'message record frozen': 'RAF_SEGMENT_V1_MESSAGE_SIZE 128u' in segment_h,
    'bounded reader present': 'raf_segment_reader_v1_next' in segment_h and 'range_within' in segment_c,
    'payload CRCs verified': 'content_crc32c' in segment_h and 'author_crc32c' in segment_h,
}
for name, passed in checks.items():
    if not passed:
        raise SystemExit(f'FAIL {name}')
    print(f'PASS {name}')
PY

printf '%s\n' '[7/9] ecosystem evidence-state validation'
python3 scripts/validate_ecosystem_runtime_state.py

printf '%s\n' '[8/9] toroidal research router validation'
python3 -m unittest tests/test_toroidal_research_router.py -v
python3 scripts/toroidal_research_router.py validate-contract \
  contracts/toroidal_research_router.v1.json
python3 scripts/toroidal_research_router.py validate-manifest \
  contracts/toroidal_research_router.v1.json \
  examples/toroidal_research_router.example.json
python3 scripts/toroidal_research_router.py summarize \
  contracts/toroidal_research_router.v1.json \
  examples/toroidal_research_router.example.json

printf '%s\n' '[9/9] ecosystem build doctor unit tests and self-audit'
python3 -m unittest tests/test_ecosystem_build_doctor.py -v
python3 scripts/ecosystem_build_doctor.py \
  --repo rafpolimata=. \
  --json-out "$BUILD/ecosystem-build-doctor.json" \
  --markdown-out "$BUILD/ecosystem-build-doctor.md" \
  --fail-on critical
python3 - "$BUILD/ecosystem-build-doctor.json" <<'PY'
import json
from pathlib import Path
import sys

report = json.loads(Path(sys.argv[1]).read_text())
assert report['schema'] == 'raf.ecosystem-build-doctor-report.v1'
assert report['claim_boundary']['static_analysis'] == 'VERIFIED_BY_EXECUTION'
assert report['claim_boundary']['build_execution'] == 'TOKEN_VAZIO'
assert report['claim_boundary']['runtime_correctness'] == 'TOKEN_VAZIO'
assert report['claim_boundary']['automatic_deletion'] is False
print('PASS ecosystem-build-doctor-report-envelope')
PY

printf '%s\n' 'PASS rafpolimata-runtime-truth-local'
