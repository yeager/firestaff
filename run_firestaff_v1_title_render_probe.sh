#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"

cc -std=c99 -Wall -Wextra -pedantic -O2 -I"$HERE" \
  "$HERE/probes/v1/firestaff_v1_pass57_title_render_probe.c" \
  "$HERE/title_dat_loader_v1.c" \
  -o "$HERE/firestaff_v1_pass57_title_render_probe"

"$HERE/firestaff_v1_pass57_title_render_probe" "$@"
