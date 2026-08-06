#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
launcher="$root/probes/nexus/firestaff_nexus_v1_mednafen_slev_sal_capture_launcher.sh"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

printf local-bios > "$tmp/bios.bin"
printf local-disc > "$tmp/game.cue"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}')
disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
args=(--mednafen /usr/bin/true --bios "$tmp/bios.bin" --bios-sha256 "$bios_hash" --bios-region us --disc "$tmp/game.cue" --disc-sha256 "$disc_hash" --capture "$tmp/capture.slevsal" --manifest "$tmp/plan.txt" --route-epoch 7 --package-fnv 1020304050607080 --card-fnv 8877665544332211 --task-trace-fnv 1111111111111111 --task-source-fnv 2222222222222222 --sal-descriptor-fnv 3333333333333333 --map-table-fnv 4444444444444444 --sddrvs-fnv 5555555555555555)

out=$(bash "$launcher" "${args[@]}")
grep -F -- '-ss.bios_na_eu' <<<"$out" >/dev/null
grep -Fx 'FIRESTAFF_NEXUS_MEDNAFEN_SLEV_SAL_CAPTURE_PLAN_V1' "$tmp/plan.txt" >/dev/null
grep -Fx 'capture_magic=NXSLSC01' "$tmp/plan.txt" >/dev/null
grep -Fx "bios_sha256=$bios_hash" "$tmp/plan.txt" >/dev/null
grep -Fx 'task_source_fnv1a64=2222222222222222' "$tmp/plan.txt" >/dev/null
[[ ! -e "$tmp/capture.slevsal" ]]

launch_args=(${args[@]})
launch_args[13]="$tmp/launch.slevsal"
launch_args[15]="$tmp/launch-plan.txt"
if bash "$launcher" --launch --operator-only "${launch_args[@]}"; then exit 1; fi
[[ ! -e "$tmp/launch.slevsal" ]]

bad_hash=(${args[@]})
bad_hash[5]="${bios_hash/a/b}"
bad_hash[13]="$tmp/bad.slevsal"
bad_hash[15]="$tmp/bad-plan.txt"
if bash "$launcher" "${bad_hash[@]}"; then exit 1; fi

bad_route=(${args[@]})
bad_route[17]=0
bad_route[13]="$tmp/route.slevsal"
bad_route[15]="$tmp/route-plan.txt"
if bash "$launcher" "${bad_route[@]}"; then exit 1; fi

echo "nexus mednafen SLEV/SAL capture launcher: PASS"
