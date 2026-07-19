#!/usr/bin/env bash
set -euo pipefail

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD="${TMPDIR:-/tmp}/rafpolimata-runtime-truth-$$"
trap 'rm -rf "$BUILD"' EXIT
mkdir -p "$BUILD"
cd "$ROOT"

printf '%s\n' '[1/7] strict host compiler build'
cc -std=c11 -Wall -Wextra -Werror \
  raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c \
  -o "$BUILD/raf_compile"

printf '%s\n' '[2/7] segment v1 header, fixed records and bounded-reader tests'
make -C runtime/conversation_indexer test-segment audit

printf '%s\n' '[3/7] raw native output contract'
printf 'int main(void){return 0;}\n' > "$BUILD/input.c"
"$BUILD/raf_compile" "$BUILD/input.c" "$BUILD/out" O2 --native
for suffix in s hex bin ops; do
  test -s "$BUILD/out.$suffix"
done
grep -qx 'ops_schema=3' "$BUILD/out.ops"
grep -qx 'native_requested=1' "$BUILD/out.ops"
grep -qx 'native_written=1' "$BUILD/out.ops"

printf '%s\n' '[4/7] optional output-base parsing'
(
  cd "$BUILD"
  "$BUILD/raf_compile" input.c --native
  test -s raf_out.bin
  grep -qx 'native_written=1' raf_out.ops
)

printf '%s\n' '[5/7] unknown extension and oversized source rejection'
printf 'not a C source\n' > "$BUILD/input.unknown"
if "$BUILD/raf_compile" "$BUILD/input.unknown" "$BUILD/unknown"; then
  echo 'FAIL unknown extension was accepted as C' >&2
  exit 1
fi
grep -qx 'rollback_code=-6' "$BUILD/unknown.ops"

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

printf '%s\n' '[6/7] source-level honesty invariants'
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

printf '%s\n' '[7/7] ecosystem evidence-state validation'
python3 scripts/validate_ecosystem_runtime_state.py

printf '%s\n' 'PASS rafpolimata-runtime-truth-local'
