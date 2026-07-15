#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd); t=$(mktemp); trap 'rm -f "$t"' EXIT
{
  printf '%s\n' 'scsi_read_command generation=7 opcode=08 cdb=x start_lba=4847 sector_count=8'
  byte=0
  while [ "$byte" -lt 10240 ]; do
    lba=$((4847 + byte / 2048)); offset=$((byte % 2048))
    printf 'pce_cd_data_origin sequence=%s cpu_pc=eb33 port=1801 source_generation=7 source_lba=%s source_offset=%s data=00\n' "$byte" "$lba" "$offset"
    printf 'pce_cd_fifo_read generation=7 fifo_sequence=%s reader_pc=eb33 value=00\n' "$byte"
    printf 'pce_cd_fifo_destination_receipt generation=7 fifo_sequence=%s reader_pc=eb33 logical_destination=0002 physical_destination=1fe002 value=00\n' "$byte"
    byte=$((byte + 1))
  done
  printf '%s\n' 'generation7_fifo_window_complete fifo_sequence=16383' 'post_generation7_main_ram_write writer_pc=311e writer_physical_pc=1f111e logical_destination=3128 physical_destination=1f1128 value=c1'
} >"$t"
perl "$repo/scripts/verify_theron_generation7_order_receipt.pl" "$t"
