#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_fr_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_FR_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_FR.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 French DOS archive is not staged'
    exit 77
fi

# The French retail GDAT has its own authenticated identity.  It shares the
# DOS dungeon bytes with the English release but must retain the PC French
# GAME_LOAD owner all the way through the first input, rather than borrowing
# an English startup path.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform pc --data-dir "$archive" --boot-probe \
    --boot-probe-frames 5000 --script 'key:enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=b4d733576ea60c41737f79f212faf528'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 French DOS ZIP reaches runtime and moves in memory'
