#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
launcher="$root/probes/nexus/firestaff_nexus_v1_mednafen_structure3_topology_capture_launcher.sh"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

printf local-bios > "$tmp/bios.bin"
printf local-disc > "$tmp/game.cue"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}')
disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
args=(--mednafen /usr/bin/true --bios "$tmp/bios.bin" --bios-sha256 "$bios_hash" --bios-region us --disc "$tmp/game.cue" --disc-sha256 "$disc_hash" --capture "$tmp/capture.top" --manifest "$tmp/plan.txt" --route-epoch 7 --package-fnv 1020304050607080 --card-fnv 8877665544332211 --dgn-fnv 0123456789abcdef --dgn-size 4096 --structure1f-index 2 --structure3-index 3 --face-ordinal 4 --vertex-offset 128 --vertex-length 48 --vertex-fnv 1111111111111111 --vertex-rows-fnv 2222222222222222 --normal-offset 256 --normal-length 12 --normal-fnv 3333333333333333)

out=$(bash "$launcher" "${args[@]}")
grep -F -- '-ss.bios_na_eu' <<<"$out" >/dev/null
grep -Fx 'FIRESTAFF_NEXUS_MEDNAFEN_STRUCTURE3_TOPOLOGY_CAPTURE_PLAN_V1' "$tmp/plan.txt" >/dev/null
grep -Fx 'capture_magic=NXS3TOP1' "$tmp/plan.txt" >/dev/null
grep -Fx "bios_sha256=$bios_hash" "$tmp/plan.txt" >/dev/null
grep -Fx 'vertex_table_fnv1a64=1111111111111111' "$tmp/plan.txt" >/dev/null
[[ ! -e "$tmp/capture.top" ]]

if bash "$launcher" --launch "${args[@]}"; then exit 1; fi
[[ ! -e "$tmp/capture.top" ]]

bad_hash=(${args[@]})
bad_hash[5]="${bios_hash/a/b}"
bad_hash[13]="$tmp/bad.top"
bad_hash[15]="$tmp/bad-plan.txt"
if bash "$launcher" "${bad_hash[@]}"; then exit 1; fi

bad_span=(${args[@]})
bad_span[43]=0
bad_span[13]="$tmp/span.top"
bad_span[15]="$tmp/span-plan.txt"
if bash "$launcher" "${bad_span[@]}"; then exit 1; fi

echo "nexus mednafen Structure3 topology capture launcher: PASS"
