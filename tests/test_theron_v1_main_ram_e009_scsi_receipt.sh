#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
verifier=$repo/scripts/verify_theron_main_ram_e009_scsi_receipt.pl
work=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-e009-scsi.XXXXXX")
trap 'rm -rf "$work"' EXIT

cat >"$work/valid.trace" <<'EOF'
main_ram_loader_e009_dispatch sequence=1 logical_pc=3840 physical_pc=1f1840 a=20 x=03 y=02
pce_cd_register_write cpu_pc=e90d physical=00001801 data=81
pce_cd_register_write cpu_pc=e981 physical=00001801 data=08
pce_cd_register_write cpu_pc=e981 physical=00001801 data=00
pce_cd_register_write cpu_pc=e981 physical=00001801 data=10
pce_cd_register_write cpu_pc=e981 physical=00001801 data=45
pce_cd_register_write cpu_pc=e981 physical=00001801 data=04
pce_cd_register_write cpu_pc=e981 physical=00001801 data=00
scsi_read_command generation=5 opcode=08 cdb=080010a10100 start_lba=4257 sector_count=1
EOF
perl "$verifier" "$work/valid.trace"

{ head -n1 "$work/valid.trace"; echo unrelated; tail -n8 "$work/valid.trace"; } >"$work/invalid.trace"
if perl "$verifier" "$work/invalid.trace"; then
    printf 'FAIL: verifier accepted a non-adjacent SCSI read\n' >&2
    exit 1
fi
printf 'PASS: main-RAM e009/SCSI verifier is fail-closed\n'
