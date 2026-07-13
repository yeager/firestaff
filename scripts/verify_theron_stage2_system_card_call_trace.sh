#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 || ! -f $1 ]]; then
    printf 'usage: %s TRACE\n' "$0" >&2
    exit 2
fi

trace=$1

require_row() {
    local row=$1
    if ! grep -Fqx "$row" "$trace"; then
        printf 'FAIL: missing authentic stage-two receipt: %s\n' "$row" >&2
        exit 1
    fi
}

# These rows establish only the observed System Card call/return boundary and
# loader table bytes. They intentionally do not assign game-data semantics.
require_row 'stage2_system_card_call pc=40cd return_pc=40d0 target=e009 a=01 x=03 y=03 p=00 mpr0=ff table=00e30302 fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff'
require_row 'stage2_system_card_return pc=40d0 call_pc=40cd a=00 x=01 y=ff p=03 mpr0=ff fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff'
require_row 'stage2_system_card_call pc=40a4 return_pc=40a7 target=e00f a=01 x=03 y=ff p=01 mpr0=ff table=00e70311 fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff'
require_row 'stage2_system_card_return pc=40a7 call_pc=40a4 a=78 x=00 y=03 p=00 mpr0=ff fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff'

printf 'PASS: authenticated stage-two System Card call/return receipt\n'
