#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"

cc -std=c99 -Wall -Wextra -pedantic \
  -I"$HERE" \
  "$HERE/probes/v1/firestaff_v1_pass61_title_menu_handoff_probe.c" \
  "$HERE/title_frontend_v1.c" \
  "$HERE/title_dat_loader_v1.c" \
  -o "$HERE/firestaff_v1_pass61_title_menu_handoff_probe"

"$HERE/firestaff_v1_pass61_title_menu_handoff_probe" "$@"
