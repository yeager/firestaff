#!/bin/sh
set -eu

runtime_test="${1:?Atari M11 runtime test executable is required}"
data_dir="${FIRESTAFF_CSB_ATARI_DATA:-$HOME/.firestaff/data/csb}"
archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_dir/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"

if [ ! -x "$runtime_test" ]; then
    echo "SKIP: Atari M11 runtime test executable is unavailable"
    exit 0
fi
if [ ! -f "$archive" ]; then
    echo "SKIP: local CSB Atari archive is unavailable: $archive"
    exit 0
fi

if command -v 7zz >/dev/null 2>&1; then
    seven_zip=7zz
elif command -v 7z >/dev/null 2>&1; then
    seven_zip=7z
else
    echo "SKIP: 7z/7zz is unavailable for local CSB archive verification"
    exit 0
fi

tmp_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-runtime-XXXXXX")"
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

# This is the original hard-disk package, not a fixture.  ANIM.C needs its
# two startup files; FTLCODE then loads the source GRAPHICS/DUNGEON pair and
# MINI.DAT supplies the authenticated GAMEBLOCK for the F0435/F0433 roundtrip.
if ! "$seven_zip" e -y -o"$tmp_dir" "$archive" \
    'HardDisk/2009-02-22 PP/ANIMATE.DAT' \
    'HardDisk/2009-02-22 PP/ANIMATE.SCR' \
    'HardDisk/2009-02-22 PP/DUNGEON.DAT' \
    'HardDisk/2009-02-22 PP/GRAPHICS.DAT' \
    'HardDisk/2009-02-22 PP/MINI.DAT' >/dev/null; then
    echo "FAIL: could not extract original CSB Atari runtime members" >&2
    exit 1
fi

for member in ANIMATE.DAT ANIMATE.SCR DUNGEON.DAT GRAPHICS.DAT MINI.DAT; do
    if [ ! -f "$tmp_dir/$member" ]; then
        echo "FAIL: CSB Atari archive is missing $member" >&2
        exit 1
    fi
done

FIRESTAFF_CSB_DATA_DIR="$tmp_dir" \
FIRESTAFF_CSB_ATARI_MINI="$tmp_dir/MINI.DAT" \
FIRESTAFF_CSB_TEST_QUICKSAVE_PATH="$tmp_dir/CSBGAME.DAT" \
    "$runtime_test"
echo "PASS: original CSB Atari MINI.DAT runtime/save archive corpus"
