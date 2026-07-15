#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp); track=$(mktemp)
trap 'rm -f "$trace" "$trace.bad" "$track"' EXIT
truncate -s $((1157 * 2352)) "$track"
for offset in $(seq 0 31); do printf 'pce_cd_data_origin sequence=%s cpu_pc=ea9c port=1808 source_generation=2 source_lba=4165 source_offset=%s data=00\n' "$offset" "$offset" >>"$trace"; done
perl "$repo/scripts/verify_theron_lba4165_fifo_origin_receipt.pl" "$trace" "$track"
sed 's/source_offset=0 data=00/source_offset=0 data=ff/' "$trace" >"$trace.bad"
if perl "$repo/scripts/verify_theron_lba4165_fifo_origin_receipt.pl" "$trace.bad" "$track"; then exit 1; fi
