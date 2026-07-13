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
require 'c860_transfer_source_pc=c86b'
require 'next_pc=fe92'
require 'fe90_window_pc=fe92'
require 'instruction=LDA $18C5'
require 'fe90_window_pc=fea5'
require 'instruction=LDA $18C1'
require 'fe90_window_pc=febe'
require 'instruction=RTS'
require 'fe90_transfer_source_pc=febe'
require 'next_pc=c86e'

if ! awk '
    /c860_transfer_source_pc=c86b.*next_pc=fe92/ { enter = NR }
    enter && /fe90_window_pc=fe92.*instruction=LDA \$18C5/ { status = NR }
    status && /fe90_window_pc=fea5.*instruction=LDA \$18C1/ { handshake = NR }
    handshake && /fe90_window_pc=febe.*instruction=RTS/ { return_instruction = NR }
    return_instruction && /fe90_transfer_source_pc=febe.*next_pc=c86e/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: fe92 System Card status trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves fe92 status return; no CD record or payload semantics assigned\n'
