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
require 'post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00'
require 'boot_pc=e908'
require 'instruction=LDA #$81'
require 'boot_pc=e90a'
require 'instruction=STA $1801'
require 'post_latch_cd_change_pc=e90d'
require 'cd_1801=81'
require 'boot_pc=e913'
require 'instruction=LDA #$60'
require 'boot_pc=e918'
require 'instruction=STA $1800'
require 'boot_pc=e928'
require 'instruction=LDA #$FF'
require 'boot_pc=e92a'
require 'post_latch_cd_change_pc=e92d'
require 'cd_1801=ff'
require 'boot_pc=ea37'
require 'instruction=TSB $1802'
require 'post_latch_cd_change_pc=ea3a'
require 'cd_1800=90'
require 'cd_1802=80'

if ! awk '
    /post_latch_cd_baseline_pc=c897/ { baseline = NR }
    baseline && /boot_pc=e908 .*instruction=LDA #\$81/ { setup = NR }
    setup && /boot_pc=e90a .*instruction=STA \$1801/ { write_81 = NR }
    write_81 && /post_latch_cd_change_pc=e90d .*cd_1801=81/ { observed_81 = NR }
    observed_81 && /boot_pc=e913 .*instruction=LDA #\$60/ { command = NR }
    command && /boot_pc=e918 .*instruction=STA \$1800/ { write_60 = NR }
    write_60 && /boot_pc=e928 .*instruction=LDA #\$FF/ { reply = NR }
    reply && /boot_pc=e92a .*instruction=STA \$1801/ { write_ff = NR }
    write_ff && /post_latch_cd_change_pc=e92d .*cd_1801=ff/ { observed_ff = NR }
    observed_ff && /boot_pc=ea37 .*instruction=TSB \$1802/ { status = NR }
    status && /post_latch_cd_change_pc=ea3a .*cd_1800=90 .*cd_1802=80/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-latch CD controller trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the first post-latch CD controller register exchange; no CD_READ record or payload semantics assigned\n'
