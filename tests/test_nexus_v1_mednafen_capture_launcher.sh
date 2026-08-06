#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
launcher="$root/probes/nexus/firestaff_nexus_v1_mednafen_capture_launcher.sh"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
printf bios > "$tmp/bios.bin"
printf disc > "$tmp/game.cue"
printf menu > "$tmp/MENU.BPK"
printf dm > "$tmp/DM.BIN"
printf dgn > "$tmp/LEV00.DGN"
bios_hash=$(shasum -a 256 "$tmp/bios.bin" | awk '{print $1}')
disc_hash=$(shasum -a 256 "$tmp/game.cue" | awk '{print $1}')
menu_hash=$(shasum -a 256 "$tmp/MENU.BPK" | awk '{print $1}')
dm_hash=$(shasum -a 256 "$tmp/DM.BIN" | awk '{print $1}')
dgn_hash=$(shasum -a 256 "$tmp/LEV00.DGN" | awk '{print $1}')
args=(--mednafen /usr/bin/true --bios "$tmp/bios.bin" --bios-sha256 "$bios_hash" --disc "$tmp/game.cue" --disc-sha256 "$disc_hash" --trace "$tmp/trace.txt" --validator /usr/bin/true --menu-bpk "$tmp/MENU.BPK" --menu-bpk-sha256 "$menu_hash" --dm-bin "$tmp/DM.BIN" --dm-bin-sha256 "$dm_hash" --dgn "$tmp/LEV00.DGN" --dgn-sha256 "$dgn_hash" --manifest "$tmp/manifest.txt" --replay-trace-fnv 77 --replay-dgn-fnv 1234 --replay-bitmap-fnv 22 --replay-epoch 1)
out=$(bash "$launcher" "${args[@]}")
grep -F -- '-ss.bios_na_eu' <<<"$out" >/dev/null
grep -F -- "$tmp/bios.bin" <<<"$out" >/dev/null
grep -F -- "$tmp/game.cue" <<<"$out" >/dev/null
[[ ! -e "$tmp/trace.txt" ]]
grep -Fx 'FIRESTAFF_NEXUS_MEDNAFEN_PRS3_REPLAY_MANIFEST_V1' "$tmp/manifest.txt" >/dev/null
grep -Fx 'replay_epoch=1' "$tmp/manifest.txt" >/dev/null
if bash "$launcher" "${args[@]}"; then exit 1; fi
bad=(${args[@]}); bad[5]="${bios_hash/a/b}"; bad[25]="$tmp/reject.txt"; bad[35]="$tmp/reject-manifest.txt"; if bash "$launcher" "${bad[@]}"; then exit 1; fi
launch_args=(${args[@]}); launch_args[25]="$tmp/no-trace.txt"; launch_args[35]="$tmp/no-trace-manifest.txt"; if bash "$launcher" --launch "${launch_args[@]}"; then exit 1; fi
echo "nexus mednafen capture launcher: PASS"
