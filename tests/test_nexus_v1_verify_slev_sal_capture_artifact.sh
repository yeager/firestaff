#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.."&&pwd);tmp=$(mktemp -d);trap 'rm -rf "$tmp"' EXIT
printf '%s\n' route_epoch=7 package_fnv1a64=1020304050607080 card_fnv1a64=8877665544332211 task_trace_fnv1a64=1111111111111111 sal_descriptor_fnv1a64=2222222222222222 map_table_fnv1a64=3333333333333333 sddrvs_fnv1a64=4444444444444444 > "$tmp/p"
perl -e 'open F,">",shift;binmode F;print F "NXSLSC01",pack("NN",1,96),pack("Q>*",7,0x1020304050607080,0x8877665544332211,0x1111111111111111,0x2222222222222222,0x3333333333333333,0x4444444444444444),pack("NNQ>Q>",96,1,1,1,1),"x"' "$tmp/c"
bash "$root/probes/nexus/firestaff_nexus_v1_verify_slev_sal_capture_artifact.sh" --plan "$tmp/p" --capture "$tmp/c"
