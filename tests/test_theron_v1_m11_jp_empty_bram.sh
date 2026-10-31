#!/bin/sh
set -eu

probe=$1
data_root=${FIRESTAFF_WORKSPACE_DATA_DIR:-"$HOME/.firestaff/data"}
theron_root=${FIRESTAFF_THERON_DATA_DIR:-"$data_root/theron"}
track02=${FIRESTAFF_THERON_JP_TRACK02_BIN:-"$theron_root/TQJP02.bin"}
empty_bram=${FIRESTAFF_THERON_JP_EMPTY_BRAM:-}

[ -f "$track02" ] && [ "$(wc -c < "$track02" | tr -d ' ')" = 8102640 ] &&
    [ -f "$empty_bram" ] && [ "$(wc -c < "$empty_bram" | tr -d ' ')" = 2048 ] || exit 77
if command -v md5 >/dev/null 2>&1; then
    track02_md5=$(md5 -q "$track02")
    bram_md5=$(md5 -q "$empty_bram")
else
    track02_md5=$(md5sum "$track02" | awk '{print $1}')
    bram_md5=$(md5sum "$empty_bram" | awk '{print $1}')
fi
[ "$track02_md5" = b7afb338ad31be1025b53f9aff12d73a ] || {
    echo "FAIL: JP Track 02 is not the authenticated original" >&2
    exit 1
}
[ "$bram_md5" = dbdedb0ec809227b289c2bc5b18b9c9d ] || {
    echo "FAIL: JP BRAM is not the captured authentic empty save" >&2
    exit 1
}

original_home=$HOME
mkdir -p "$original_home/.cache"
test_home=$(mktemp -d "$original_home/.cache/firestaff-theron-jp-empty-bram.XXXXXX")
trap 'rm -rf "$test_home"' EXIT HUP INT TERM
HOME=$test_home
XDG_DATA_HOME=$test_home/.local/share
APPDATA=$test_home/AppData/Roaming
FIRESTAFF_THERON_BRAM_PATH=$empty_bram
export HOME XDG_DATA_HOME APPDATA FIRESTAFF_THERON_BRAM_PATH
"$probe" "$data_root" "$track02" "$empty_bram"

if command -v md5 >/dev/null 2>&1; then
    track02_after_md5=$(md5 -q "$track02")
    bram_after_md5=$(md5 -q "$empty_bram")
else
    track02_after_md5=$(md5sum "$track02" | awk '{print $1}')
    bram_after_md5=$(md5sum "$empty_bram" | awk '{print $1}')
fi
[ "$track02_after_md5" = "$track02_md5" ] || {
    echo "FAIL: JP Track 02 changed during the startup test" >&2
    exit 1
}
[ "$bram_after_md5" = "$bram_md5" ] || {
    echo "FAIL: native startup modified the authentic empty JP BRAM" >&2
    exit 1
}
