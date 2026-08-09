#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.."&&pwd);tmp=$(mktemp -d);trap 'rm -rf "$tmp"' EXIT
printf '%s\n' route_epoch=7 package_fnv1a64=1020304050607080 card_fnv1a64=8877665544332211 task_trace_fnv1a64=0123456789abcdef sal_descriptor_fnv1a64=13579bdf2468ace0 map_table_fnv1a64=0f1e2d3c4b5a6978 sddrvs_fnv1a64=89abcdef01234567 > "$tmp/p"
perl -e 'open F,">",shift;binmode F;print F "NXSLSC01",pack("NN",1,96),pack("Q>*",7,0x1020304050607080,0x8877665544332211,0x0123456789abcdef,0x13579bdf2468ace0,0x0f1e2d3c4b5a6978,0x89abcdef01234567),pack("NNQ>Q>",96,1,1,1,1),"x"' "$tmp/c"
bash "$root/probes/nexus/firestaff_nexus_v1_verify_slev_sal_capture_artifact.sh" --plan "$tmp/p" --capture "$tmp/c"
printf '\0' | dd of="$tmp/c" bs=1 seek=56 conv=notrunc status=none
if bash "$root/probes/nexus/firestaff_nexus_v1_verify_slev_sal_capture_artifact.sh" --plan "$tmp/p" --capture "$tmp/c"; then
  exit 1
fi
