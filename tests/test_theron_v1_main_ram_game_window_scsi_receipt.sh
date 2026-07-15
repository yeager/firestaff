#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
trap 'rm -f "$trace" "$trace.bad"' EXIT

for offset in 0 1 2 3 4 5 6 7; do
    printf 'main_ram_game_window_read sequence=%s logical_address=300%s physical_address=1f100%s value=00 reader_pc=2c88 reader_physical_pc=1f0c88\n' "$offset" "$offset" "$offset" >>"$trace"
done
cat >>"$trace" <<'EOF'
main_ram_loader_e009_dispatch sequence=0 logical_pc=2cc7 physical_pc=1f0cc7 a=ef x=00 y=4a
scsi_read_command generation=2 opcode=08 cdb=080010450400 start_lba=4165 sector_count=4
EOF
perl "$repo/scripts/verify_theron_main_ram_game_window_scsi_receipt.pl" "$trace"

sed 's/reader_physical_pc=1f0c88/reader_physical_pc=002b22/' "$trace" >"$trace.bad"
if perl "$repo/scripts/verify_theron_main_ram_game_window_scsi_receipt.pl" "$trace.bad"; then
    echo "FAIL: accepted a System Card reader as the game-owned window consumer" >&2
    exit 1
fi
