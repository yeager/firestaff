#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_mac_reject_foreign_dos_resume.sh <firestaff>}
mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-}
dos_archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-}
scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}

if [ ! -x "$app" ] || [ ! -f "$mac_archive" ] || [ ! -f "$dos_archive" ]; then
    echo 'SKIP: authentic DM2 Macintosh and DOS archives are required'
    exit 77
fi

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
mkdir -p "$scratch"
config_path="$scratch/dm2-mac-resume-rejection-$$.cfg"
trap 'rm -f "$config_path" "$config_path.tmp"' EXIT HUP INT TERM

if output=$(FIRESTAFF_CONFIG_PATH="$config_path" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac --data-dir "$mac_archive" \
    --save "$dos_archive::data/sksave1.dat" --script enter --duration 3000 2>&1); then
    printf '%s\n' "$output" >&2
    echo 'FAIL: DM2 Macintosh accepted a foreign DOS SKSave' >&2
    exit 1
fi
case "$output" in
    *'DM2 Macintosh Resume is unavailable until its complete original GAME_LOAD path is supported'*) ;;
    *)
        printf '%s\n' "$output" >&2
        echo 'FAIL: Mac Resume rejection did not identify the unsupported load path' >&2
        exit 1
        ;;
esac

echo 'PASS: authentic DM2 Macintosh launch rejects an original DOS SKSave Resume request'
