#!/bin/sh
set -eu
sh tools/rafbbs/rafbbs_build.sh host
sh tools/rafbbs/rafbbs_build.sh freestanding
tools/rafbbs/rafbbs --help >/tmp/rafbbs-help.txt
tools/rafbbs/rafbbs list >/tmp/rafbbs-list.txt
tools/rafbbs/rafbbs files >/tmp/rafbbs-files.txt
printf 'q\n' | tools/rafbbs/rafbbs >/tmp/rafbbs-tui.txt
cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/tests/rafbbs_failsafe_test.c -o /tmp/rafbbs_failsafe_test
/tmp/rafbbs_failsafe_test
cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/tests/rafbbs_baremetal_test.c -o /tmp/rafbbs_baremetal_test
/tmp/rafbbs_baremetal_test
cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/tests/rafbbs_watchdog_negative_test.c -o /tmp/rafbbs_watchdog_negative_test
/tmp/rafbbs_watchdog_negative_test
if rg -n "\b(malloc|calloc|realloc|free)\s*\(" tools/rafbbs --glob '*.h' --glob '*.c'; then
  echo "heap symbol found" >&2
  exit 1
fi
