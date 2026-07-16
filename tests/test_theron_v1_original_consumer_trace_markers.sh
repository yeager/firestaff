#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
markers=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$markers" "$bad"' EXIT

palette_raw=$((0x2a06a0))
nonstartup_raw=$((2352 * 2000 + 17))
object_raw=$((2352 * 2001 + 23))
palette_lba=$((palette_raw / 2352 + 3009))
palette_offset=$((palette_raw % 2352))
nonstartup_lba=$((nonstartup_raw / 2352 + 3009))
nonstartup_offset=$((nonstartup_raw % 2352))
object_lba=$((object_raw / 2352 + 3009))
object_offset=$((object_raw % 2352))

cat >"$trace" <<EOF
source=mednafen-pce-instrumented-cd
main_ram_loader_e009_dispatch sequence=7 logical_pc=3840 physical_pc=1f1840 a=20 x=00 y=00
pce_cd_fifo_origin_main_ram_receipt generation=9 source_lba=$palette_lba source_offset=$palette_offset fifo_sequence=100 reader_pc=e98a logical_destination=2300 physical_destination=1f2300 writer_pc=3844 writer_physical_pc=1f1844 value=11
pce_cd_fifo_origin_main_ram_consumer sequence=0 generation=9 source_lba=$palette_lba source_offset=$palette_offset fifo_sequence=100 logical_address=2300 physical_address=1f2300 value=11 reader_pc=3900 reader_physical_pc=1f1900
pce_cd_fifo_origin_main_ram_receipt generation=9 source_lba=$nonstartup_lba source_offset=$nonstartup_offset fifo_sequence=101 reader_pc=e98a logical_destination=2301 physical_destination=1f2301 writer_pc=3844 writer_physical_pc=1f1844 value=22
pce_cd_fifo_origin_main_ram_consumer sequence=1 generation=9 source_lba=$nonstartup_lba source_offset=$nonstartup_offset fifo_sequence=101 logical_address=2301 physical_address=1f2301 value=22 reader_pc=3901 reader_physical_pc=1f1901
pce_cd_fifo_origin_main_ram_receipt generation=9 source_lba=$object_lba source_offset=$object_offset fifo_sequence=102 reader_pc=e98a logical_destination=2302 physical_destination=1f2302 writer_pc=3844 writer_physical_pc=1f1844 value=33
pce_cd_fifo_origin_main_ram_consumer sequence=2 generation=9 source_lba=$object_lba source_offset=$object_offset fifo_sequence=102 logical_address=2302 physical_address=1f2302 value=33 reader_pc=3902 reader_physical_pc=1f1902
EOF

perl "$repo/scripts/verify_theron_original_consumer_trace_markers.pl" \
  "$trace" 1156 0x00000101 0x00000202 0x00000303 \
  "$palette_raw" "$nonstartup_raw" "$object_raw" >"$markers"

grep -Fq 'theron_track02_original_consumer_trace' "$markers"
grep -Fq "palette_raw_offset=$palette_raw" "$markers"
grep -Fq "nonstartup_level_raw_offset=$nonstartup_raw" "$markers"
grep -Fq "object_table_raw_offset=$object_raw" "$markers"
grep -Fq 'synthetic_palette_promoted=0' "$markers"
grep -Fq 'fallback_visuals_allowed=0' "$markers"

grep -v "source_lba=$object_lba source_offset=$object_offset" "$trace" >"$bad"
if perl "$repo/scripts/verify_theron_original_consumer_trace_markers.pl" \
  "$bad" 1156 0x00000101 0x00000202 0x00000303 \
  "$palette_raw" "$nonstartup_raw" "$object_raw" >"$markers" 2>/dev/null; then
  echo 'expected missing object-table consumer rejection' >&2
  exit 1
fi
