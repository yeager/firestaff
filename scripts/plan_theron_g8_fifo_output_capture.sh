#!/usr/bin/env bash
set -euo pipefail
if [ "$#" -ne 3 ]; then
  printf 'usage: %s MEDNAFEN CUE TRACE\n' "$0" >&2
  exit 2
fi
mednafen=$1
cue=$2
trace=$3
if [ ! -x "$mednafen" ] || [ ! -f "$cue" ] || [ -e "$trace" ]; then
  printf 'FAIL: require executable Mednafen, regular CUE, and absent trace target\n' >&2
  exit 1
fi
printf 'FIRESTAFF_THERON_G8_FIFO_OUTPUT_TRACE=1 FIRESTAFF_THERON_IRQ2_TRACE=%q %q %q\n' \
  "$trace" "$mednafen" "$cue"
