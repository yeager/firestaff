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
require 'instruction=NOP'
require 'boot_pc=e985'
require 'instruction=JSR $EA27'
require 'boot_pc=e988'
require 'instruction=BRA $E95E'
require 'boot_pc=e96a'
require 'instruction=AND #$B8'

if ! awk '
    /boot_pc=e981 .*instruction=NOP/ { after = NR }
    after && /boot_pc=e985 .*instruction=JSR \$EA27/ { call = NR }
    call && /boot_pc=e988 .*instruction=BRA \$E95E/ { returned = NR }
    returned && /boot_pc=e96a .*instruction=AND #\$B8/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: post-latch controller transition trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves the next post-e981 controller transition; no CD_READ, record, transfer, or payload semantics assigned\n'
