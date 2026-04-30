#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
cc -std=c99 -Wall -Wextra -pedantic -I"$HERE" "$HERE/test_action_area_routes_pc34_compat_integration.c" "$HERE/action_area_routes_pc34_compat.c" "$HERE/touch_click_zone_matrix_pc34_compat.c" -o "$HERE/test_action_area_routes_pc34_compat_integration"
"$HERE/test_action_area_routes_pc34_compat_integration"
