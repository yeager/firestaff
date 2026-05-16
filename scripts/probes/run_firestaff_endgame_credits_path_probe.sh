#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
cc -std=c99 -Wall -Wextra -pedantic -I"$HERE" "$HERE/test_endgame_credits_path_pc34_compat_integration.c" "$HERE/endgame_credits_path_pc34_compat.c" -o "$HERE/test_endgame_credits_path_pc34_compat_integration"
"$HERE/test_endgame_credits_path_pc34_compat_integration"
