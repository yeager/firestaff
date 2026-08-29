#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_mac_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Macintosh retail archive is not staged'
    exit 77
fi

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac --data-dir "$archive" \
    --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60' --duration 1000 >/dev/null 2>&1

# Macintosh is the first card on the second platform row.  This remains a
# launcher-only pointer sequence; the native movie and mirror clicks are
# separately covered below at their original 320x200 coordinate space.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform mac --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:410:679,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# Retail Mac owns a title movie before its source New Game action.  The first
# Enter dismisses that movie; the second is the authenticated title-menu
# action.  Keep the host window at 320x200: --script pointer coordinates are
# mapped through the current presentation rectangle, so this preserves the
# original GDAT/mirror coordinates.  The viewport click selects the
# File_header-backed mirror through the native preselection owner, and Up is
# then normal runtime movement.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac --data-dir "$archive" --boot-probe \
    --boot-probe-frames 2000 --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'champions=2'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 Macintosh ZIP start menu, title, mirror selection, and movement run in memory'
