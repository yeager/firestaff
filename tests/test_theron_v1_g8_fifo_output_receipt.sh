#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT
cat >"$trace" <<'EOF'
main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1
g8_fifo_output_capture generation=8 source_lba=4859 dispatch_sequence=4
pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 source_offset=0 fifo_sequence=1 reader_pc=ea50 logical_destination=2200 physical_destination=1f0200 writer_pc=ea52 writer_physical_pc=1f0252 value=ab
EOF
perl "$repo/scripts/verify_theron_g8_fifo_output_receipt.pl" "$trace"
sed 's/generation=8 source_lba=4859/generation=9 source_lba=4859/' "$trace" >"$bad"
if perl "$repo/scripts/verify_theron_g8_fifo_output_receipt.pl" "$bad"; then
  echo 'FAIL: accepted mutated G8 output' >&2; exit 1
fi
printf 'PASS: G8 FIFO output schema is fail-closed\n'
