#!/bin/sh
set -eu

save_test="${1:?Atari save test executable is required}"
data_dir="${FIRESTAFF_CSB_ATARI_DATA:-$HOME/.firestaff/data/csb}"
archive="${FIRESTAFF_CSB_ATARI_ARCHIVE:-$data_dir/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"
member='HardDisk/2009-02-22 PP/MINI.DAT'

if [ ! -x "$save_test" ]; then
    echo "SKIP: Atari save test executable is unavailable"
    exit 0
fi
if [ ! -f "$archive" ]; then
    echo "SKIP: local CSB Atari archive is unavailable: $archive"
    exit 0
fi
if ! command -v 7zz >/dev/null 2>&1; then
    echo "SKIP: 7zz is unavailable for local CSB archive verification"
    exit 0
fi

tmp_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-atari-mini-XXXXXX")"
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

if ! 7zz e -y -o"$tmp_dir" "$archive" "$member" >/dev/null; then
    echo "FAIL: could not extract original CSB Atari MINI.DAT" >&2
    exit 1
fi
if [ ! -f "$tmp_dir/MINI.DAT" ]; then
    echo "FAIL: CSB Atari archive has no original MINI.DAT member" >&2
    exit 1
fi

FIRESTAFF_CSB_ATARI_SAVE_CORPUS="$tmp_dir/MINI.DAT" "$save_test"
echo "PASS: original CSB Atari MINI.DAT archive corpus"
