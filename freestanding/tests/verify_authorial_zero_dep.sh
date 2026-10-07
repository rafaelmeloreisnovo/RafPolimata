#!/bin/sh
set -eu
CC=${CC:-clang}
NM=${NM:-nm}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
PROBE="$ROOT/tools/rafbbs/tests/rafbbs_authorial_probe.c"
OUT=${TMPDIR:-/tmp}/rafbbs-authorial-zero-dep
mkdir -p "$OUT"
cd "$ROOT"
sh "$ROOT/freestanding/tests/verify_contract.sh"
PURE_FILES="
tools/rafbbs/rafbbs_types.h
tools/rafbbs/rafbbs_status.h
tools/rafbbs/rafbbs_time.h
tools/rafbbs/rafbbs_freestanding.h
tools/rafbbs/rafbbs_core.h
tools/rafbbs/rafbbs_context_core.h
tools/rafbbs/rafbbs_command_core.h
tools/rafbbs/rafbbs_manifest_core.h
tools/rafbbs/rafbbs_filepicker_core.h
tools/rafbbs/rafbbs_theme.h
tools/rafbbs/rafbbs_log_core.h
tools/rafbbs/rafbbs_format_core.h
tools/rafbbs/rafbbs_git_core.h
tools/rafbbs/rafbbs_runlog_core.h
tools/rafbbs/rafbbs_pipeline_core.h
tools/rafbbs/rafbbs_cli_core.h
tools/rafbbs/rafbbs_tui_core.h
tools/rafbbs/rafbbs_baremetal.h
tools/rafbbs/rafbbs_manifest_bin_core.h
tools/rafbbs/rafbbs_crc32_core.h
tools/rafbbs/rafbbs_sha256_core.h
"
for command in "$CC" "$NM"; do command -v "$command" >/dev/null 2>&1 || { echo "FAIL: required tool unavailable: $command" >&2; exit 127; }; done
for file in $PURE_FILES; do test -f "$ROOT/$file" || { echo "FAIL: declared authorial file is missing: $file" >&2; exit 1; }; done
test -f "$PROBE" || { echo "FAIL: declared authorial probe is missing: $PROBE" >&2; exit 1; }
fail_source() { label=$1; pattern=$2; if grep -n -E "$pattern" $PURE_FILES "$PROBE"; then echo "FAIL: $label" >&2; exit 1; fi; }
fail_source "hosted/system header in authorial RafBBS slice" '^[[:space:]]*#include[[:space:]]*<'
fail_source "host/runtime primitive in authorial RafBBS slice" '(^|[^A-Za-z0-9_])(malloc|calloc|realloc|free|system|fopen|fread|fwrite|fclose)[[:space:]]*[(]'
fail_source "syscall instruction in authorial RafBBS slice" '"(syscall|ecall|svc[[:space:]]*#?0|int[[:space:]]+[$]0x80)"'
fail_source "inline assembly in authorial RafBBS slice" '(^|[^A-Za-z0-9_])(__asm__|asm)[[:space:]]*'
fail_source "external declaration in authorial RafBBS slice" '^[[:space:]]*extern[[:space:]]'
sh "$ROOT/tools/rafbbs/rafbbs_build.sh" freestanding
COMMON="-std=c11 -Wall -Wextra -Werror -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-ident -fno-optimize-sibling-calls -fvisibility=hidden -ffunction-sections -fdata-sections -nostdinc -I$ROOT/tools/rafbbs"
compile() { target=$1; obj="$OUT/${target}.o"; "$CC" -target "$target" $COMMON -c "$PROBE" -o "$obj"; if "$NM" -u "$obj" | grep -q '[^[:space:]]'; then echo "FAIL: unresolved helper(s) in authorial RafBBS $target" >&2; "$NM" -u "$obj" >&2; exit 1; fi; if ! "$NM" -g --defined-only "$obj" | grep -Eq '[[:space:]][Tt][[:space:]]+rafbbs_authorial_probe$'; then echo "FAIL: authorial probe symbol missing in $target" >&2; exit 1; fi; }
compile x86_64-unknown-none
compile i686-unknown-none
compile armv7a-none-eabi
compile aarch64-none-elf
compile riscv32-unknown-elf
compile riscv64-unknown-elf
echo "RAFAELIA RafBBS authorial zero-dependency gate: 21 pure modules; command-policy + context-seed + time + text/binary manifest + filepicker + theme + log-line + typed-format + git-provenance-parse + persisted-runlog-byte + pipeline-spec + cli-route + tui-route cores included; 6/6 ISA objects; unresolved helpers=0"
