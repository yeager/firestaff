#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

cat >"$trace" <<'EOF'
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e7 physical_destination=1f01e7 value=04
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e6 physical_destination=1f01e6 value=20
post_generation7_main_ram_write writer_pc=3837 writer_physical_pc=1f1837 logical_destination=21e5 physical_destination=1f01e5 value=ff
main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
pce_cd_register_write cpu_pc=e981 physical=00001801 data=08
scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1
main_ram_loader_e009_dispatch sequence=5 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
pce_cd_register_write cpu_pc=e981 physical=00001801 data=08
scsi_read_command generation=9 opcode=08 cdb=080012f70300 start_lba=4855 sector_count=3
main_ram_loader_e009_dispatch sequence=6 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
pce_cd_register_write cpu_pc=e981 physical=00001801 data=08
scsi_read_command generation=10 opcode=08 cdb=080012fa0100 start_lba=4858 sector_count=1
EOF

perl "$repo/scripts/verify_theron_post_generation7_cdb_parameter_receipt.pl" "$trace"
cp "$trace" "$bad"
sed -i '' '12s/cdb=080012fa0100/cdb=080012fb0100/' "$bad"
if perl "$repo/scripts/verify_theron_post_generation7_cdb_parameter_receipt.pl" "$bad"; then
  echo 'expected direct-CDB parameter rejection' >&2
  exit 1
fi
