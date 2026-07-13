#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
verify=$repo/scripts/verify_theron_post_e98a_track02_runtime_handoff_trace.sh
tmp=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-cd-read-registers.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

valid=$tmp/valid.trace
mismatched=$tmp/mismatched-register.trace

cat >"$valid" <<'EOF'
source=mednafen-pce-instrumented
boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90
post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27
dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=e0 record_dl=04 record_ch=00 variant=us_bin record=0004e0
dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00
EOF

"$verify" "$valid" >/dev/null
sed 's/record_cl=e0/record_cl=df/' "$valid" >"$mismatched"
if "$verify" "$mismatched" >/dev/null 2>&1; then
    printf 'FAIL: mismatched CL/DL/CH transaction reached Track 02 handoff\n' >&2
    exit 1
fi

printf 'PASS: Track 02 dynamic CD_READ register provenance is mandatory\n'
