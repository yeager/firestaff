#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
cc -std=c99 -Wall -Wextra -pedantic -I"$HERE" "$HERE/test_interface_keyboard_routes_pc34_compat_integration.c" "$HERE/interface_keyboard_routes_pc34_compat.c" -o "$HERE/test_interface_keyboard_routes_pc34_compat_integration"
"$HERE/test_interface_keyboard_routes_pc34_compat_integration"
