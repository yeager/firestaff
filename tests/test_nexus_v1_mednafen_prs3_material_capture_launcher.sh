#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd); launcher="$root/probes/nexus/firestaff_nexus_v1_mednafen_prs3_material_capture_launcher.sh"; tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
printf bios > "$tmp/b"; printf disc > "$tmp/d"; bh=$(shasum -a 256 "$tmp/b"|awk '{print $1}'); dh=$(shasum -a 256 "$tmp/d"|awk '{print $1}')
args=(--mednafen /usr/bin/true --bios "$tmp/b" --bios-sha256 "$bh" --bios-region us --disc "$tmp/d" --disc-sha256 "$dh" --capture "$tmp/c" --manifest "$tmp/m" --route-epoch 7 --package-fnv 1020304050607080 --card-fnv 8877665544332211 --entry-index 4 --compressed-offset 288 --compressed-length 52 --compressed-fnv 1122334455667788 --declared-output 128)
bash "$launcher" "${args[@]}"; grep -Fx 'NXSPRS3M' "$tmp/m" >/dev/null; [[ ! -e "$tmp/c" ]]; if bash "$launcher" --launch "${args[@]}"; then exit 1; fi
echo "nexus mednafen PRS3 material capture launcher: PASS"
