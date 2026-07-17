#!/usr/bin/env bash
set -euo pipefail
list=${NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_LIST:-}
if [[ -z "$list" ]]; then echo "SKIP: set DGN multi-level capture list"; exit 77; fi
[[ -f "$list" && $(wc -l < "$list" | tr -d ' ') == 16 ]]
awk 'NF == 5 && $1 == NR-1 && $2 ~ /^[0-9A-Fa-f]+$/ && $3 ~ /^[0-9]+$/ && $4 ~ /^[0-9]+$/ && $5 ~ /^\// { next } { exit 1 }' "$list"
while read -r level dgn descriptor command trace; do [[ -s "$trace" ]]; head -n 1 "$trace" | grep -Fx 'NEXUS_PRS3_SH2_VDP1_TRACE_V10' >/dev/null; done < "$list"
echo "nexus multi-level DGN capture external inputs: PASS"
