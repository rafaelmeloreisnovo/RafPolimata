#!/bin/sh
variant="$1"
lane="$2"
case "$variant" in
  reference|alternate) ;;
  *) exit 65 ;;
esac
case "$lane" in
  correctness) printf '%s\n' 'fixture-ok' ;;
  observe) : ;;
  performance)
    if [ "$variant" = reference ]; then :; else :; fi
    ;;
  *) exit 66 ;;
esac
