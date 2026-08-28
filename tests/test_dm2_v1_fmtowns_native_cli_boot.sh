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
    --script 'key:enter' --duration 1000 >/dev/null 2>&1

# AUTOEXEC hands SWOOSH/TITLE to SKULL before the source New Game event.  The
# viewport click is a source-space mirror selection, followed by normal Up.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform fm-towns --data-dir "$archive" --boot-probe \
    --boot-probe-frames 12000 --script 'key:enter,click:100:100,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=027ff3b8ddc2c4c4cdda7ada0b0bc46c'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 FM Towns ZIP start menu presents a real GDAT runtime frame'
