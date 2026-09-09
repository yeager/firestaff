#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$repo/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/firestaff-theron-lba4165.XXXXXX")
trace="$scratch/trace"
track="$scratch/track"
trap 'rm -rf "$scratch"' EXIT

truncate -s $((1160 * 2352)) "$track"
cat >"$trace" <<'EOF'
scsi_read_command generation=2 opcode=08 cdb=080010450400 start_lba=4165 sector_count=4
cd_interface_raw_sector_read lba=4165 bytes=2352 sector_fnv1a=291bd385 span_offset=0 span_bytes=32 span_fnv1a=0b2ae445
cd_interface_raw_sector_read lba=4166 bytes=2352 sector_fnv1a=291bd385 span_offset=0 span_bytes=32 span_fnv1a=0b2ae445
cd_interface_raw_sector_read lba=4167 bytes=2352 sector_fnv1a=291bd385 span_offset=0 span_bytes=32 span_fnv1a=0b2ae445
cd_interface_raw_sector_read lba=4168 bytes=2352 sector_fnv1a=291bd385 span_offset=0 span_bytes=32 span_fnv1a=0b2ae445
EOF
perl "$repo/scripts/verify_theron_lba4165_track02_receipt.pl" "$trace" "$track"

sed 's/sector_fnv1a=291bd385/sector_fnv1a=00000000/' "$trace" >"$trace.bad"
if perl "$repo/scripts/verify_theron_lba4165_track02_receipt.pl" "$trace.bad" "$track"; then
    echo "FAIL: accepted a mismatched raw Track 02 sector" >&2
    exit 1
fi
