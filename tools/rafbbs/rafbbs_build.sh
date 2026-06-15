#!/bin/sh
set -eu
cc -std=c11 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L -I tools/rafbbs tools/rafbbs/rafbbs.c -o tools/rafbbs/rafbbs
