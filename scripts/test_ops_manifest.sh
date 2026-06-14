#!/usr/bin/env sh
set -eu

cc="${CC:-gcc}"
build_dir="build_host_check/ops_manifest"
mkdir -p "$build_dir"

"$cc" -std=c11 -Wall -Wextra -Werror \
  raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c \
  -o "$build_dir/raf_compile"

"$build_dir/raf_compile" raf_main.c "$build_dir/a" O3 >/dev/null
"$build_dir/raf_compile" raf_main.c "$build_dir/b" O3 >/dev/null
python3 scripts/validate_ops_manifest.py "$build_dir/a.ops" --expect-rollback 0
python3 scripts/validate_ops_manifest.py "$build_dir/b.ops" --expect-rollback 0
python3 scripts/compare_ops_manifest.py "$build_dir/a.ops" "$build_dir/b.ops"

set +e
"$build_dir/raf_compile" does_not_exist.c "$build_dir/fail" O3 >/dev/null 2>&1
rc="$?"
set -e
if [ "$rc" -eq 0 ]; then
  echo "expected missing-source compile to fail"
  exit 1
fi
python3 scripts/validate_ops_manifest.py "$build_dir/fail.ops" --expect-rollback -1
