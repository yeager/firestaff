#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

cat >"$trace" <<'EOF'
post_generation7_main_ram_write writer_pc=384d writer_physical_pc=1f184d logical_destination=3837 physical_destination=1f1837 value=1e
post_generation7_main_ram_write writer_pc=3852 writer_physical_pc=1f1852 logical_destination=3838 physical_destination=1f1838 value=20
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e7 physical_destination=1f01e7 value=04
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e6 physical_destination=1f01e6 value=20
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e5 physical_destination=1f01e5 value=ff
main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1
EOF

perl "$repo/scripts/verify_theron_post_g7_parameter_thunk_receipt.pl" "$trace"
cp "$trace" "$bad"
sed -i '' '4s/value=20/value=21/' "$bad"
if perl "$repo/scripts/verify_theron_post_g7_parameter_thunk_receipt.pl" "$bad"; then
  echo 'expected parameter thunk value rejection' >&2
  exit 1
fi
