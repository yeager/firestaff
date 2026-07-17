#!/usr/bin/env bash
set -euo pipefail

trace=${NEXUS_V1_PRS3_V10_TRACE:-}
bpk=${NEXUS_V1_PRS3_BPK_PATH:-}
dgn=${NEXUS_V1_PRS3_DGN_PATH:-}
bpk_sha=${NEXUS_V1_PRS3_BPK_SHA256:-}
dgn_sha=${NEXUS_V1_PRS3_DGN_SHA256:-}

if [[ -z "$trace" || -z "$bpk" || -z "$dgn" || -z "$bpk_sha" || -z "$dgn_sha" ]]; then
    echo "SKIP: set NEXUS_V1_PRS3_V10_TRACE, BPK/DGN paths and SHA-256 values"
    exit 77
fi
[[ -f "$trace" && -f "$bpk" && -f "$dgn" ]]
[[ $(shasum -a 256 "$bpk" | awk '{print $1}') == "$bpk_sha" ]]
[[ $(shasum -a 256 "$dgn" | awk '{print $1}') == "$dgn_sha" ]]
head -n 1 "$trace" | grep -Fx 'NEXUS_PRS3_SH2_VDP1_TRACE_V10' >/dev/null
grep -E '^dgn_descriptor_fnv1a64=[1-9a-fA-F][0-9a-fA-F]*$' "$trace" >/dev/null
echo "nexus prs3 vdp1 capture replay local inputs: PASS"
