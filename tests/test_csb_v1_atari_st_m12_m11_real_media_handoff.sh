#!/bin/sh
set -eu

handoff_test="${1:?CSB M12/M11 handoff test executable is required}"
source_archive="${FIRESTAFF_CSB_ATARI_ST_ARCHIVE:-$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"
source_root="${FIRESTAFF_CSB_ATARI_ST_ROOT:-}"
test_scratch=${FIRESTAFF_TEST_SCRATCH:-"${TMPDIR:-$PWD/.codex-scratch}"}
staging_dir=""

cleanup() {
    if [ -n "$staging_dir" ]; then rm -rf "$staging_dir"; fi
}
trap cleanup EXIT HUP INT TERM

if [ -e "$source_archive" ]; then
    extractor=$(command -v 7zz || command -v 7z || true)
    if [ -z "$extractor" ]; then
        echo "SKIP: authentic CSB Atari source is archived and 7zz/7z is unavailable"
        exit 77
    fi
    mkdir -p "$test_scratch"
    staging_dir=$(mktemp -d "$test_scratch/firestaff-csb-atari-handoff.XXXXXX") || exit 1
    source_root="$staging_dir"
    game_member='Floppy Disks STX/Chaos Strikes Back for Atari ST Game Disk v2.1 (English).stx'
    utility_member='Floppy Disks STX/Chaos Strikes Back for Atari ST Utility Disk v2.1 (English).stx'
    if ! "$extractor" x -so "$source_archive" "$game_member" \
            > "$source_root/Chaos Strikes Back.stx" ||
        [ ! -s "$source_root/Chaos Strikes Back.stx" ]; then
        echo "FAIL: could not extract the authentic CSB Atari ST v2.1 game disk" >&2
        exit 1
    fi
    if ! "$extractor" x -so "$source_archive" "$utility_member" \
            > "$source_root/Chaos Strikes Back Utility.stx" ||
        [ ! -s "$source_root/Chaos Strikes Back Utility.stx" ]; then
        echo "FAIL: could not extract the authentic CSB Atari ST v2.1 utility disk" >&2
        exit 1
    fi
elif [ -n "$source_root" ] &&
        [ -s "$source_root/Chaos Strikes Back.stx" ] &&
        [ -s "$source_root/Chaos Strikes Back Utility.stx" ]; then
    : # Use the caller's already extracted, matching original media pair.
else
    echo "SKIP: matching authentic CSB Atari ST game and utility disks are unavailable"
    exit 77
fi

FIRESTAFF_CSB_ATARI_ST_ONLY=1 \
FIRESTAFF_CSB_ATARI_ST_ROOT="$source_root" \
    "$handoff_test"
