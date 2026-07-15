#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd); trace=$(mktemp); trap 'rm -f "$trace"' EXIT
cat >"$trace" <<'EOF'
scsi_read_command generation=4 opcode=08 cdb=080010891100 start_lba=4233 sector_count=17
pce_cd_data_origin sequence=1 cpu_pc=ea9c port=1808 source_generation=4 source_lba=4233 source_offset=0 data=78
main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=0 logical_destination=2256 physical_destination=1f0256 value=38 writer_pc=ea52 writer_physical_pc=000a52
main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=1 logical_destination=2257 physical_destination=1f0257 value=50 writer_pc=ea52 writer_physical_pc=000a52
main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=2 logical_destination=2258 physical_destination=1f0258 value=37 writer_pc=ea52 writer_physical_pc=000a52
main_ram_e009_fifo_destination dispatch_sequence=0 generation=4 fifo_sequence=3 logical_destination=2259 physical_destination=1f0259 value=04 writer_pc=ea52 writer_physical_pc=000a52
EOF
perl "$repo/scripts/verify_theron_generation4_system_card_receipt.pl" "$trace"
