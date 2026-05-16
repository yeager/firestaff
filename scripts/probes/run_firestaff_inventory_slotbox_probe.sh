#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
cc -std=c99 -Wall -Wextra -pedantic -I"$HERE" \
  "$HERE/test_inventory_slotbox_pc34_compat_integration.c" \
  "$HERE/inventory_slotbox_pc34_compat.c" \
  -o "$HERE/test_inventory_slotbox_pc34_compat_integration"
"$HERE/test_inventory_slotbox_pc34_compat_integration"
