#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_mac_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Macintosh retail archive is not staged'
    exit 77
fi

# Retail Mac owns a title movie before its source New Game action.  The first
# Enter dismisses that movie; the second is the authenticated title-menu
# action.  The viewport click selects the File_header-backed mirror through
# the native preselection owner, and Up is then normal runtime movement.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$archive" --boot-probe \
    --boot-probe-frames 2000 --script 'key:enter,key:enter,click:100:100,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 Macintosh ZIP title, mirror selection, and movement run in memory'
