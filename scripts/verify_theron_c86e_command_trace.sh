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
require 'fe90_transfer_source_pc=febe'
require 'next_pc=c86e'
require 'c860_window_pc=c86e'
require 'instruction=BCS $C88C'
require 'c860_window_pc=c870'
require 'instruction=TXA'
require 'c860_window_pc=c873'
require 'instruction=BEQ $C87F'
require 'c860_window_pc=c875'
require 'instruction=LDA #$AA'
require 'c860_window_pc=c877'
require 'instruction=STA $18C0'
require 'c860_window_pc=c87a'
require 'cd_1800=d0'
require 'c860_window_pc=c87c'
require 'c860_window_pc=c887'
require 'instruction=JSR $C950'
require 'c860_window_pc=c88a'
require 'instruction=BRA $C897'
require 'c860_window_pc=c897'
require 'instruction=LDA $2241'

if ! awk '
    /fe90_transfer_source_pc=febe.*next_pc=c86e/ { returned = NR }
    returned && /c860_window_pc=c86e.*instruction=BCS \$C88C/ { branch = NR }
    branch && /c860_window_pc=c870.*instruction=TXA/ { carry_clear = NR }
    carry_clear && /c860_window_pc=c875.*instruction=LDA #\$AA/ { command_a = NR }
    command_a && /c860_window_pc=c877.*instruction=STA \$18C0/ { write_a = NR }
    write_a && /c860_window_pc=c87a.*cd_1800=d0/ { observed_command = NR }
    observed_command && /c860_window_pc=c87c.*instruction=STA \$18C0/ { write_b = NR }
    write_b && /c860_window_pc=c887.*instruction=JSR \$C950/ { helper = NR }
    helper && /c860_window_pc=c88a.*instruction=BRA \$C897/ { handoff = NR }
    handoff && /c860_window_pc=c897.*instruction=LDA \$2241/ { proven = 1 }
    END { exit proven ? 0 : 1 }
' "$trace"; then
    printf 'FAIL: c86e command trace order is incomplete\n' >&2
    exit 1
fi

printf 'PASS: original Mednafen trace proves c86e command-latch route; no CD record or payload semantics assigned\n'
