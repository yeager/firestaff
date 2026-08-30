#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_amiga_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_AMIGA_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"}

# ZIP -> native Amiga media admission must not depend on an external tool.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Amiga archive is not staged'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform amiga --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# DM2's third platform card is Amiga; retain the real archive through a
# pointer-only game -> platform -> Original selection.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform amiga --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform amiga --data-dir "$archive" --boot-probe \
    --boot-probe-frames 2000 --script 'key:enter,key:enter,key:enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=1c940ea95703eaea0ecdf84d17e954b9'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 Amiga archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 Amiga ZIP start menu reaches runtime and moves in memory'
