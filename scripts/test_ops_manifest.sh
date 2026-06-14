#!/usr/bin/env sh
set -eu

cc="${CC:-gcc}"
build_dir="build_host_check/ops_manifest"
mkdir -p "$build_dir"

exec_root="${TMPDIR:-${TMP:-/tmp}}/raf_ops_manifest_$$"
mkdir -p "$exec_root"
trap 'rm -rf "$exec_root"' EXIT HUP INT TERM
exe="$exec_root/raf_compile"

"$cc" -std=c11 -Wall -Wextra -Werror \
  raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c \
  -o "$exe"
chmod +x "$exe" 2>/dev/null || true

"$exe" raf_main.c "$build_dir/a" O3 >/dev/null
"$exe" raf_main.c "$build_dir/b" O3 >/dev/null
python3 scripts/validate_ops_manifest.py "$build_dir/a.ops" --expect-rollback 0
python3 scripts/validate_ops_manifest.py "$build_dir/b.ops" --expect-rollback 0
python3 scripts/compare_ops_manifest.py "$build_dir/a.ops" "$build_dir/b.ops"

set +e
"$exe" does_not_exist.c "$build_dir/fail" O3 >/dev/null 2>&1
rc="$?"
set -e
if [ "$rc" -eq 0 ]; then
  echo "expected missing-source compile to fail"
  exit 1
fi
python3 scripts/validate_ops_manifest.py "$build_dir/fail.ops" --expect-rollback -1
bash scripts/android_build_matrix.sh --plan >/dev/null
test -s build_host_check/android_matrix/plan.txt
