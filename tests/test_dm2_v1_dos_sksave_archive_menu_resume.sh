#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_sksave_archive_menu_resume.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 DOS archive is not staged'
    exit 77
fi

# Keep both the game and its original SKSAVE member in the same read-only ZIP.
# The menu hands --save to DM2's source GAME_LOAD path; no user media may be
# materialized beside the archive.
save_path="$archive::data/sksave1.dat"
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
    --boot-probe --boot-probe-frames 5000 \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-map 11 --boot-probe-expect-party 15,10,2 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=25247ede4dabb6a71e5dabdfbcd5907d'*'phase=dm2-runtime'*'levelLoaded=1'*'map=11'*'party=15,10,2'*'startedFromLauncher=1'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 DOS ZIP start menu resumes archive::SKSAVE in memory'
