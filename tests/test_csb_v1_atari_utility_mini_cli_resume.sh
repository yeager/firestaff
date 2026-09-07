#!/bin/sh
# Native Atari ST resume from the supplied CSB campaign and Utility STX
# images.  Both paths remain virtual; Firestaff must not extract MINI.DAT.
set -eu

firestaff=${1:?Firestaff executable is required}
data_root=${FIRESTAFF_CSB_REAL_MEDIA_ROOT:-"$HOME/.firestaff/data/csb"}
campaign=${FIRESTAFF_CSB_ATARI_STX:-"$data_root/Chaos Strikes Back.stx"}
utility=${FIRESTAFF_CSB_ATARI_UTILITY_STX:-"$data_root/Chaos Strikes Back Utility.stx"}
save=${FIRESTAFF_CSB_ATARI_MINI:-"$utility::MINI.DAT"}

if [ ! -x "$firestaff" ] || [ ! -f "$campaign" ] || [ ! -f "$utility" ]; then
    printf '%s\n' 'SKIP: authentic CSB Atari campaign and Utility STX media are unavailable'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 45s "$firestaff" \
    --game csb --platform atari-st --data-dir "$campaign" --save "$save" \
    --boot-probe --boot-probe-frames 10 --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --duration 0 2>&1)

printf '%s\n' "$output"
printf '%s\n' "$output" | grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb'
printf '%s\n' "$output" | grep -Fq 'route=f0435-resume'
printf '%s\n' "$output" | grep -Fq 'levelLoaded=1 map=4 party=22,18,2 champions=1'
printf '%s\n' "$output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'
