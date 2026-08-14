#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-record-XXXXXX")
trap 'rm -rf "$tmp"' EXIT
ram="$tmp/ram-provenance"
watch="$tmp/record-watch"
registers="$tmp/spawn-registers"

printf '%s\n' source=mednafen-pce-instrumented-ram-provenance >"$ram"
printf '%s\n' source=mednafen-pce-instrumented-record-watch >"$watch"
printf '%s\n' source=mednafen-pce-instrumented-spawn-registers-v3 >"$registers"
for offset in $(seq 301 310); do
    logical=$(printf '%04x' $((0x611d + offset - 301)))
    physical=$(printf '%06x' $((0x0f811d + offset - 301)))
    value=$(printf '%02x' $((offset - 300)))
    printf 'theron_ram_provenance_direct source_lba=4880 source_offset=%s fifo_sequence=1 destination_physical=%s destination_logical=%s reader_pc=f406 reader_physical_pc=001406 writer_pc=f427 writer_physical_pc=001427 value=%s\n' \
        "$offset" "$physical" "$logical" "$value" >>"$ram"
    printf 'theron_record_watch op=write logical_address=%s physical_address=%s value=%s pc=f427 physical_pc=001427\n' \
        "$logical" "$physical" "$value" >>"$watch"
done
printf '%s\n' 'spawn_consumer_registers sequence=0 pc=c3a0 physical_pc=000d23a0 a=00 x=00 y=00 sp=ff p=00 mpr0=ff mpr_pc=69 b3=00 b4=00 b5=00 b6=00 b8=00 ba=00 bb=00 c96b_window=0 cc4c_window=0 preconsumer_4644=0 helper_4667=0 spawn_entry_b0e5=0 record_c3a0_window=1' >>"$registers"

python3 "$repo/scripts/verify_theron_record_table_provenance.py" \
    "$ram" "$watch" --spawn-registers "$registers" --minimum-records 1
