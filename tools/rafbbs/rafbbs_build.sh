#!/bin/sh
set -eu
case "${1:-host}" in
  host)
    cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/rafbbs.c -o tools/rafbbs/rafbbs
    ;;
  freestanding)
    cc -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -I tools/rafbbs -c tools/rafbbs/tests/rafbbs_freestanding_core_test.c -o /tmp/rafbbs_freestanding_core_test.o
    cc -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -I tools/rafbbs -c tools/rafbbs/tests/rafbbs_baremetal_test.c -o /tmp/rafbbs_baremetal_test.o
    cc -std=c11 -Wall -Wextra -Werror -ffreestanding -fno-builtin -I tools/rafbbs -c tools/rafbbs/tests/rafbbs_baremetal_overflow_test.c -o /tmp/rafbbs_baremetal_overflow_test.o
    ;;
  commandless)
    cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -DRAFBBS_FREESTANDING_MODE -I tools/rafbbs -c tools/rafbbs/rafbbs.c -o /tmp/rafbbs_commandless.o
    ;;
  *)
    echo "usage: sh tools/rafbbs/rafbbs_build.sh [host|freestanding|commandless]" >&2
    exit 2
    ;;
esac
