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
require 'post_latch_cd_change_pc=ea34'
require 'before_1800=90 before_1801=ff before_1802=80'
require 'cd_1800=d0 cd_1801=ff cd_1802=00'
require 'boot_pc=e97a'
require 'instruction=LDA $224C,X'
require 'boot_pc=e97e'
require 'instruction=STA $1801'
require 'boot_pc=e981'
require 'instruction=NOP'

if ! awk '
    /post_latch_cd_change_pc=ea34 .*before_1800=90 before_1801=ff before_1802=80 .*cd_1800=d0 cd_1801=ff cd_1802=00/ { exchange = NR }
    exchange && /boot_pc=e97a .*instruction=LDA \$224C,X/ { before = NR }
    before && /boot_pc=e97e .*instruction=STA \$1801/ { write = NR }
    write && /boot_pc=e981 .*instruction=NOP/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-latch controller dispatch trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the first post-latch controller dispatch register transition; no CD_READ or payload semantics assigned\n'
