#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

dispatch_patch=$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_dispatch_trace.patch
grep -Fq 'if(lastop == 0x20 && first == 0xe009)' "$dispatch_patch"

cat >"$trace" <<'EOF'
main_ram_loader_e009_dispatch sequence=3 logical_pc=3840 physical_pc=1f1840 a=20 x=00 y=04
scsi_read_command generation=7 opcode=08 cdb=080012ef0800 start_lba=4847 sector_count=8
main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
scsi_read_command generation=8 opcode=08 cdb=080012fb0100 start_lba=4859 sector_count=1
main_ram_loader_e009_dispatch sequence=5 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
scsi_read_command generation=9 opcode=08 cdb=080012f70300 start_lba=4855 sector_count=3
main_ram_loader_e009_dispatch sequence=6 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
scsi_read_command generation=10 opcode=08 cdb=080012fa0100 start_lba=4858 sector_count=1
EOF

perl "$repo/scripts/verify_theron_post_generation7_loader_route_receipt.pl" "$trace"
cp "$trace" "$bad"
sed -i '' '8s/start_lba=4858/start_lba=4857/' "$bad"
if perl "$repo/scripts/verify_theron_post_generation7_loader_route_receipt.pl" "$bad"; then
  echo 'expected incorrect post-G7 route rejection' >&2
  exit 1
fi
