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
require 'e100_transfer_source_pc=e109'
require 'next_pc=c860'
require 'c860_window_pc=c860'
require 'instruction=LDA #$7B'
require 'c860_window_pc=c868'
require 'instruction=JSR $C950'
require 'c860_window_pc=c86b'
require 'instruction=JSR $FE92'
require 'c860_transfer_source_pc=c86b'
require 'next_pc=fe92'

if ! awk '
    /e100_transfer_source_pc=e109.*next_pc=c860/ { saw_e109 = NR }
    saw_e109 && /c860_window_pc=c860.*instruction=LDA #\$7B/ { saw_c860 = NR }
    saw_c860 && /c860_window_pc=c868.*instruction=JSR \$C950/ { saw_c950 = NR }
    saw_c950 && /c860_window_pc=c86b.*instruction=JSR \$FE92/ { saw_fe92_call = NR }
    saw_fe92_call && /c860_transfer_source_pc=c86b.*next_pc=fe92/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: c860 post-return trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves e109 -> c860 -> fe92; no CD-record semantics assigned\n'
