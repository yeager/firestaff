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
require 'boot_pc=e98a'
require 'instruction=LDA $22A4'
require 'post_e98a_controller_transfer_source_pc='

if ! awk '
    /boot_pc=e98a .*instruction=LDA \$22A4/ { entry = NR }
    entry && /post_e98a_controller_transfer_source_pc=e[89a][0-9a-f] .*next_pc=[0-9a-f][0-9a-f][0-9a-f][0-9a-f]/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-e98a controller transfer trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the first post-e98a controller transfer; no CD_READ, record, transfer payload, or destination semantics assigned\n'
