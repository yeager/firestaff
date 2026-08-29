#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_fmtowns_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 FM Towns archive is not staged'
    exit 77
fi

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform fm-towns --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# AUTOEXEC hands SWOOSH/TITLE to SKULL before the source New Game event.  Keep
# the probe window at the original 320x200 coordinate space: --script click
# coordinates are host-window points and are mapped through the presentation
# rectangle before reaching SKULL's source GDAT rectangles.  The first click
# reaches the verified NEW GAME rectangle; the second selects the authentic
# dungeon mirror after GAME_LOAD has prepared it.  Up then proves normal
# runtime input.  Do not replace either click with a host row selection.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform fm-towns --data-dir "$archive" --boot-probe \
    --boot-probe-frames 12000 --width 320 --height 200 \
    --script 'click:100:60,click:100:60,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=027ff3b8ddc2c4c4cdda7ada0b0bc46c'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 FM Towns ZIP start menu presents a real GDAT runtime frame'
