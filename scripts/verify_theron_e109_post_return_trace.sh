#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    printf 'usage: %s TRACE\n' "$0" >&2
    exit 2
fi

trace=$1
if [ ! -s "$trace" ]; then
    printf 'FAIL: missing Mednafen trace: %s\n' "$trace" >&2
    exit 1
fi

require() {
    if ! grep -Fq "$1" "$trace"; then
        printf 'FAIL: missing trace fact: %s\n' "$1" >&2
        exit 1
    fi
}

require 'source=mednafen-pce-instrumented'
require 'cb20_transfer_source_pc=cb2f'
require 'next_pc=e109'
require 'e100_window_pc=e109'
require 'instruction=JSR $C860'
require 'cd_1800=00 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00'
require 'e100_transfer_source_pc=e109'
require 'next_pc=c860'

if ! awk '
    /cb20_transfer_source_pc=cb2f.*next_pc=e109/ { saw_cb2f = NR }
    saw_cb2f && /e100_window_pc=e109.*instruction=JSR \$C860/ { saw_e109 = NR }
    saw_e109 && /e100_transfer_source_pc=e109.*next_pc=c860/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: e109 post-return trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves cb2f -> e109 -> c860; no CD-record semantics assigned\n'
