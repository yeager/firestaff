#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    printf 'usage: %s <m12-test> <gameplay-test> <title-test>\n' "$0" >&2
    exit 2
fi

fmtowns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
dos_archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}

if [[ ! -f "$fmtowns_archive" || ! -f "$dos_archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM2 FM Towns and DOS archives are not both staged'
    exit 77
fi

for test_binary in "$@"; do
    if [[ ! -x "$test_binary" ]]; then
        printf 'FAIL: test executable is unavailable: %s\n' "$test_binary" >&2
        exit 1
    fi
    FIRESTAFF_DM2_FMTOWNS_ROOT="$fmtowns_archive" \
    FIRESTAFF_DM2_ENGLISH_COMPANION="$dos_archive" \
    FIRESTAFF_DM2_ENGLISH_COMPANION_ARCHIVE="$dos_archive" \
        "$test_binary"
done

printf '%s\n' 'PASS: authentic DM2 FM Towns CUE/CCD/IMG ZIP corpus reaches M12 and M11'
