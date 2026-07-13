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
require 'boot_pc=e96a'
require 'instruction=AND #$B8'
require 'boot_pc=e96c'
require 'instruction=CMP #$98'
require 'boot_pc=e96e'
require 'instruction=BEQ $E98A'
require 'boot_pc=e98a'
require 'instruction=LDA $22A4'

if ! awk '
    /boot_pc=e96a .*instruction=AND #\$B8/ { mask = NR }
    mask && /boot_pc=e96c .*instruction=CMP #\$98/ { compare = NR }
    compare && /boot_pc=e96e .*instruction=BEQ \$E98A/ { branch = NR }
    branch && /boot_pc=e98a .*instruction=LDA \$22A4/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-latch resume branch trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the post-e96a status branch; no CD_READ, record, transfer, or payload semantics assigned\n'
