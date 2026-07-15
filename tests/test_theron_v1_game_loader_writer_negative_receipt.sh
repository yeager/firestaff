#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

for destination in 1f01f6 1f01f7 1f01f8 1f01f9 1f01fa 1f01fb 1f01f6 1f01f7 1f01f8 1f01f9 1f01fa 1f01fa; do
  printf 'main_ram_loader_write sequence=0 dispatch_sequence=0 logical_destination=0000 physical_destination=%s value=00 writer_pc=0000 writer_physical_pc=1f1185\n' "$destination" >>"$trace"
done
for i in $(seq 1 116); do
  value=00
  [ "$i" -gt 43 ] && value=ff
  printf 'main_ram_loader_write sequence=%s dispatch_sequence=0 logical_destination=0000 physical_destination=1f10%02x value=%s writer_pc=0000 writer_physical_pc=1f118b\n' "$i" "$((i - 1))" "$value" >>"$trace"
done
printf 'scsi_read_command generation=7 opcode=08 cdb=080012ef0800 start_lba=4847 sector_count=8\n' >>"$trace"

perl "$repo/scripts/verify_theron_game_loader_writer_negative_receipt.pl" "$trace"
cp "$trace" "$bad"
sed -i '' '13s/value=00/value=42/' "$bad"
if perl "$repo/scripts/verify_theron_game_loader_writer_negative_receipt.pl" "$bad"; then
  echo 'expected non-sentinel initialization rejection' >&2
  exit 1
fi
