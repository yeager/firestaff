#!/usr/bin/env bash
set -euo pipefail
list=${NEXUS_V1_PRS3_MULTI_MODE_TRACE_LIST:-}
if [[ -z "$list" ]]; then echo "SKIP: set PRS3 multi-mode trace list"; exit 77; fi
[[ -f "$list" ]]
awk 'NF == 3 && $1 ~ /^[0-9]+$/ && $2 ~ /^(LSB|MSB)$/ && $3 ~ /^\// && !seen[$1]++ { next } { exit 1 } END { if (NR < 2) exit 1 }' "$list"
while read -r mode order trace; do [[ -s "$trace" ]]; head -n 1 "$trace" | grep -Fx 'NEXUS_PRS3_SH2_VDP1_TRACE_V10' >/dev/null; done < "$list"
echo "nexus PRS3 multi-mode external inputs: PASS"
