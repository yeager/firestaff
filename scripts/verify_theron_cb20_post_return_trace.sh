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
require 'cbc0_transfer_source_pc=cbef'
require 'next_pc=cb2f'
require 'cb20_window_pc=cb2f'
require 'instruction=RTS'
require 'cb20_transfer_source_pc=cb2f'
require 'next_pc=e109'

if ! awk '
    /cbc0_transfer_source_pc=cbef/ { saw_cbef = NR }
    saw_cbef && /cb20_window_pc=cb2f/ { saw_cb2f = NR }
    saw_cb2f && /cb20_transfer_source_pc=cb2f.*next_pc=e109/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-return trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves cbef -> cb2f -> e109; no CD-record semantics assigned\n'
