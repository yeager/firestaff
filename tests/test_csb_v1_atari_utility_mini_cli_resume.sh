#!/bin/sh
# Native Atari ST resume from the supplied CSB campaign and Utility STX
# images.  Both paths remain virtual; Firestaff must not extract MINI.DAT.
set -eu

firestaff=${1:?Firestaff executable is required}
data_root=${FIRESTAFF_CSB_REAL_MEDIA_ROOT:-"$HOME/.firestaff/data/csb"}
campaign=${FIRESTAFF_CSB_ATARI_STX:-"$data_root/Chaos Strikes Back.stx"}
utility=${FIRESTAFF_CSB_ATARI_UTILITY_STX:-"$data_root/Chaos Strikes Back Utility.stx"}
save=${FIRESTAFF_CSB_ATARI_MINI:-"$utility::MINI.DAT"}
media_temp_dir=""
menu_temp_dir=""

cleanup() {
    if [ -n "$media_temp_dir" ]; then rm -rf "$media_temp_dir"; fi
    if [ -n "$menu_temp_dir" ]; then rm -rf "$menu_temp_dir"; fi
}
trap cleanup EXIT HUP INT TERM

if [ -x "$firestaff" ] && { [ ! -f "$campaign" ] || [ ! -f "$utility" ]; }; then
    media_archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_root/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"
    extractor=""
    if command -v 7zz >/dev/null 2>&1; then
        extractor=$(command -v 7zz)
    elif command -v 7z >/dev/null 2>&1; then
        extractor=$(command -v 7z)
    fi
    if [ -f "$media_archive" ] && [ -n "$extractor" ]; then
        media_temp_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-utility.XXXXXX")
        if "$extractor" e -y "-o$media_temp_dir" "$media_archive" \
            'Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx' \
            'Floppy Disks STX/Chaos Strikes Back for Atari ST Utility Disk v2.1 (English).stx' \
            >/dev/null 2>&1; then
            extracted_campaign="$media_temp_dir/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx"
            extracted_utility="$media_temp_dir/Chaos Strikes Back for Atari ST Utility Disk v2.1 (English).stx"
            if [ -f "$extracted_campaign" ] && [ -f "$extracted_utility" ]; then
                campaign=$extracted_campaign
                utility=$extracted_utility
                if [ -z "${FIRESTAFF_CSB_ATARI_MINI:-}" ]; then
                    save="$utility::MINI.DAT"
                fi
            fi
        fi
    fi
fi

if [ ! -x "$firestaff" ] || [ ! -f "$campaign" ] || [ ! -f "$utility" ]; then
    printf '%s\n' 'SKIP: authentic CSB Atari campaign and Utility STX media are unavailable'
    exit 77
fi

for mode in 0 1 2; do
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 45s "$firestaff" \
        --presentation-mode "$mode" --game csb --platform atari-st \
        --data-dir "$campaign" --save "$save" --boot-probe \
        --boot-probe-frames 10 --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 --duration 0 2>&1)

    printf '%s\n' "$output"
    printf '%s\n' "$output" | grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb'
    printf '%s\n' "$output" | grep -Fq 'route=f0435-resume'
    printf '%s\n' "$output" | grep -Fq "presentationMode=$mode"
    printf '%s\n' "$output" | grep -Fq 'levelLoaded=1 map=4 party=22,18,2 champions=1'
    printf '%s\n' "$output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'
done

# The CLI probes above bypass M12. Also exercise the real Quick Resume row
# with the same original Atari ST campaign and Utility STX MINI.DAT.
menu_temp_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-menu-resume.XXXXXX")
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_temp_dir/runtime.json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 45s "$firestaff" \
    --menu --game csb --platform atari-st --data-dir "$campaign" \
    --save "$save" --script enter,enter,enter --duration 10000 \
    >/dev/null 2>&1

python3 - "$menu_temp_dir/runtime.json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or startup["receiptReady"] != 1 or
        startup["phase"] != "inactive" or startup["active"] != 1 or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (4, 22, 18, 2, 1)):
    raise SystemExit(f"FAIL: authentic CSB Atari ST M12 Quick Resume failed: {probe}")
print("PASS: authentic CSB Atari ST M12 Quick Resume reached its MINI.DAT runtime pose")
PY
