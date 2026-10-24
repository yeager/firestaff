#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 PC-34 media is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$(dirname "$app")/dm1-debug-receipt-test"}
mkdir -p "$scratch_root"
tmp="$scratch_root/run-$$"
mkdir -p "$tmp"
trap 'rm -rf "$tmp"' EXIT
output=$(HOME="$tmp" XDG_CONFIG_HOME="$tmp" APPDATA="$tmp" \
    FIRESTAFF_CONFIG_PATH="$tmp/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 0 --script up --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        exit 1
    }

if ! grep -Fq 'selected game=dm1 platform=PC edition=pc34-en' <<<"$output" ||
   ! grep -Fq 'game-action game=dm1 tick=1 input=forward result=1' <<<"$output" ||
   ! grep -Fq 'game-debug-capabilities game=dm1 source=reDMCSB-derived-simulation events=sequenced-runtime-receipts' <<<"$output" ||
   grep -Fq 'source=compatibility-world' <<<"$output" ||
   grep -Eq 'game-event game=dm1 tick=.*type=(player-attack|monster-moved|monster-attack|champion-damaged)' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 debug provenance mislabeled or fabricated a combat event' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic PC-34 DM1 debug labels the ReDMCSB-derived runtime and emits no unobserved combat event'
