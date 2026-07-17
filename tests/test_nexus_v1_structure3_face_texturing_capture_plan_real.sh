#!/usr/bin/env bash
set -euo pipefail
list=${NEXUS_V1_STRUCTURE3_FACE_CAPTURE_LIST:-}
if [[ -z "$list" ]]; then echo "SKIP: set Structure3 face capture list"; exit 77; fi
[[ -f "$list" ]]
awk 'NF == 5 && $1 >= 0 && $1 <= 15 && $2 ~ /^[0-9]+$/ && $3 ~ /^[0-9]+$/ && $4 ~ /^[0-9]+$/ && $5 ~ /^\// { next } { exit 1 } END { if(NR==0)exit 1 }' "$list"
while read -r level descriptor mesh face trace; do [[ -s "$trace" ]]; head -n 1 "$trace" | grep -Fx 'NEXUS_PRS3_SH2_VDP1_TRACE_V10' >/dev/null; done < "$list"
echo "nexus Structure3 face capture external inputs: PASS"
