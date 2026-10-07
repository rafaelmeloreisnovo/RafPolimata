#!/bin/sh
set -eu

FS_CFLAGS="-std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostdinc -I tools/rafbbs"

case "${1:-host}" in
  host)
    cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/rafbbs.c -o tools/rafbbs/rafbbs
    ;;
  freestanding)
    cc $FS_CFLAGS -c tools/rafbbs/tests/rafbbs_freestanding_core_test.c -o /tmp/rafbbs_freestanding_core_test.o
    cc $FS_CFLAGS -c tools/rafbbs/tests/rafbbs_baremetal_test.c -o /tmp/rafbbs_baremetal_test.o
    cc $FS_CFLAGS -c tools/rafbbs/tests/rafbbs_baremetal_overflow_test.c -o /tmp/rafbbs_baremetal_overflow_test.o
    cc $FS_CFLAGS -c tools/rafbbs/tests/rafbbs_zero_dependency_test.c -o /tmp/rafbbs_zero_dependency_test.o

    for obj in       /tmp/rafbbs_freestanding_core_test.o       /tmp/rafbbs_baremetal_test.o       /tmp/rafbbs_baremetal_overflow_test.o       /tmp/rafbbs_zero_dependency_test.o
    do
      if nm -u "$obj" | grep -q .; then
        echo "FAIL: unresolved helper in $obj" >&2
        nm -u "$obj" >&2
        exit 1
      fi
    done
    ;;
  commandless)
    cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -DRAFBBS_FREESTANDING_MODE -I tools/rafbbs -c tools/rafbbs/rafbbs.c -o /tmp/rafbbs_commandless.o
    ;;
  *)
    echo "usage: sh tools/rafbbs/rafbbs_build.sh [host|freestanding|commandless]" >&2
    exit 2
    ;;
esac
