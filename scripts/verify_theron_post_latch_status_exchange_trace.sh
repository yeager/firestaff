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
require 'boot_pc=e981'
require 'boot_pc=e9d3'
require 'instruction=JSR $E9DC'
require 'boot_pc=e9dc'
require 'instruction=STZ $227B'
require 'boot_pc=e9eb'
require 'instruction=LDA $1801'
require 'boot_pc=e9f3'
require 'instruction=TST #$40, $1800'
require 'boot_pc=ea15'
require 'boot_pc=ea1d'
require 'instruction=TST #$80, $1800'
require 'boot_pc=ea26'
require 'instruction=RTS'

if ! awk '
    /boot_pc=e981 .*instruction=NOP/ { entry = NR }
    entry && /boot_pc=e9d3 .*instruction=JSR \$E9DC/ { call = NR }
    call && /boot_pc=e9dc .*instruction=STZ \$227B/ { clear = NR }
    clear && /boot_pc=e9eb .*instruction=LDA \$1801 .*cd_1801=01/ { reply = NR }
    reply && /boot_pc=e9f3 .*instruction=TST #\$40, \$1800 .*cd_1802=80/ { first_wait = NR }
    first_wait && /boot_pc=ea15 .*instruction=TST #\$40, \$1800 .*cd_1802=80/ { second_wait = NR }
    second_wait && /boot_pc=ea1d .*instruction=TST #\$80, \$1800 .*cd_1800=00/ { completion = NR }
    completion && /boot_pc=ea26 .*instruction=RTS/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-latch controller status exchange trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the post-e981 controller status exchange; no CD_READ, record, transfer, or payload semantics assigned\n'
