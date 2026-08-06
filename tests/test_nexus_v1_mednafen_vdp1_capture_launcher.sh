#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
launcher="$root/probes/nexus/firestaff_nexus_v1_mednafen_vdp1_capture_launcher.sh"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

printf local-bios > "$tmp/bios.bin"
printf local-disc > "$tmp/game.cue"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}')
disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
args=(--mednafen /usr/bin/true --bios "$tmp/bios.bin" --bios-sha256 "$bios_hash" --bios-region us --disc "$tmp/game.cue" --disc-sha256 "$disc_hash" --capture "$tmp/capture.vdp1" --manifest "$tmp/plan.txt" --route-epoch 7 --package-fnv 1020304050607080 --card-fnv 8877665544332211 --dgn-fnv 0123456789abcdef --dgn-size 4096 --face-fnv 1111111111111111 --descriptor-fnv 2222222222222222 --image-fnv 3333333333333333 --palette-fnv 4444444444444444)

out=$(bash "$launcher" "${args[@]}")
grep -F -- '-ss.bios_na_eu' <<<"$out" >/dev/null
grep -Fx 'FIRESTAFF_NEXUS_MEDNAFEN_VDP1_CAPTURE_PLAN_V1' "$tmp/plan.txt" >/dev/null
grep -Fx "bios_sha256=$bios_hash" "$tmp/plan.txt" >/dev/null
grep -Fx 'bios_region=us' "$tmp/plan.txt" >/dev/null
grep -Fx 'descriptor_fnv1a64=2222222222222222' "$tmp/plan.txt" >/dev/null
[[ ! -e "$tmp/capture.vdp1" ]]

if bash "$launcher" --launch "${args[@]}"; then exit 1; fi
[[ ! -e "$tmp/capture.vdp1" ]]

bad_hash=(${args[@]})
bad_hash[5]="${bios_hash/a/b}"
bad_hash[13]="$tmp/bad.vdp1"
bad_hash[15]="$tmp/bad-plan.txt"
if bash "$launcher" "${bad_hash[@]}"; then exit 1; fi

bad_region=(${args[@]})
bad_region[7]=xx
bad_region[13]="$tmp/region.vdp1"
bad_region[15]="$tmp/region-plan.txt"
if bash "$launcher" "${bad_region[@]}"; then exit 1; fi

echo "nexus mednafen VDP1 capture launcher: PASS"
