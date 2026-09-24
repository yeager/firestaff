#!/bin/sh
set -eu

probe=$1
data_root=${FIRESTAFF_WORKSPACE_DATA_DIR:-"$HOME/.firestaff/data"}
theron_root=${FIRESTAFF_THERON_DATA_DIR:-"$data_root/theron"}
track02=${FIRESTAFF_THERON_US_TRACK02_BIN:-"$theron_root/TQUS02.bin"}
bram=${FIRESTAFF_THERON_AUTHENTIC_PROGRESS_BRAM:-"$theron_root/theron-us-akutuba-complete-authentic.bram"}

[ -f "$track02" ] && [ "$(wc -c < "$track02" | tr -d ' ')" = 8104992 ] &&
    [ -f "$bram" ] && [ "$(wc -c < "$bram" | tr -d ' ')" = 2048 ] || exit 77
if command -v md5 >/dev/null 2>&1; then
    track02_md5=$(md5 -q "$track02")
else
    track02_md5=$(md5sum "$track02" | awk '{print $1}')
fi
[ "$track02_md5" = f23601102138f87c33025877767ebf76 ] || {
    echo "FAIL: US Track 02 is not the authenticated original" >&2
    exit 1
}
exec "$probe" "$data_root" "$track02" "$bram"
