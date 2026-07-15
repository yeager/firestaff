#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

cat >"$trace" <<'EOF'
pce_cd_fifo_origin_main_ram_receipt generation=8 source_lba=4859 source_offset=31 fifo_sequence=16384 reader_pc=ea9c logical_destination=3000 physical_destination=1f1000 writer_pc=ea52 writer_physical_pc=000a52 value=42
pce_cd_fifo_origin_main_ram_consumer sequence=0 generation=8 source_lba=4859 source_offset=31 fifo_sequence=16384 logical_address=3000 physical_address=1f1000 value=42 reader_pc=1840 reader_physical_pc=1f1840
EOF
perl "$repo/scripts/verify_theron_fifo_origin_main_ram_consumer.pl" "$trace"

sed 's/reader_physical_pc=1f1840/reader_physical_pc=000a52/' "$trace" >"$bad"
if perl "$repo/scripts/verify_theron_fifo_origin_main_ram_consumer.pl" "$bad"; then
    printf 'FAIL: verifier accepted a System Card reader as game-owned\n' >&2
    exit 1
fi
