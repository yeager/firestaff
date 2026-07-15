#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

cat >"$trace" <<'EOF'
scsi_read_command generation=4 opcode=08 cdb=080010891100 start_lba=4233 sector_count=17
pce_cd_fifo_origin_main_ram_receipt generation=4 source_lba=4233 source_offset=17 fifo_sequence=0 reader_pc=ea50 logical_destination=2256 physical_destination=1f0256 writer_pc=ea52 writer_physical_pc=000a52 value=78
EOF
perl "$repo/scripts/verify_theron_fifo_origin_main_ram_receipt.pl" "$trace"

sed 's/source_lba=4233/source_lba=4250/' "$trace" >"$bad"
if perl "$repo/scripts/verify_theron_fifo_origin_main_ram_receipt.pl" "$bad"; then
    printf 'FAIL: verifier accepted a source LBA outside the READ(6) range\n' >&2
    exit 1
fi
