#!/bin/sh
set -eu
: "${CC:=cc}"
d="$(mktemp -d)"
trap 'rm -rf "$d"' EXIT HUP INT TERM
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -pedantic -ffreestanding -fno-builtin -c RAFDB/banco/rafdb.c -o "$d/core.o"
if nm -u "$d/core.o" | grep .; then
  echo "FAIL: core references external symbols" >&2; exit 1
fi
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -pedantic RAFDB/banco/rafdb.c RAFDB/gateway/rafdb_sql.c RAFDB/tests/test_rafdb.c -o "$d/rafdb_test"
"$d/rafdb_test"
