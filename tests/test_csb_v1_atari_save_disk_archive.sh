#!/bin/sh
set -eu

msa_test="${1:?Atari MSA test executable is required}"
data_dir="${FIRESTAFF_CSB_ATARI_DATA:-$HOME/.firestaff/data/csb}"
archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_dir/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"

if [ ! -x "$msa_test" ]; then
    echo "SKIP: Atari MSA test executable is unavailable"
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

tmp_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-save-disk-XXXXXX")"
trap 'rm -rf -- "$tmp_dir"' EXIT HUP INT TERM

# The retail save disk is intentionally blank.  Its MSA container and FAT12
# root must be validated as original media before Firestaff can report that no
# saved session is present; no replacement CSBGAME.DAT is generated here.
if ! "$seven_zip" e -y -o"$tmp_dir" "$archive" \
    'Floppy Disks MSA/Chaos Strikes Back for Atari ST Save Disk.msa' >/dev/null; then
    echo "FAIL: could not extract original CSB Atari save disk MSA" >&2
    exit 1
fi

msa="$tmp_dir/Chaos Strikes Back for Atari ST Save Disk.msa"
if [ ! -f "$msa" ]; then
    echo "FAIL: CSB Atari archive is missing its original save disk MSA" >&2
    exit 1
fi

FIRESTAFF_CSB_ATARI_MSA="$msa" "$msa_test"
echo "PASS: original CSB Atari save disk archive remains blank and admitted"
