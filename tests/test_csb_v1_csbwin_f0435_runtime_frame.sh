#!/bin/sh
set -eu

test_bin=${1:?missing CSB M11 HUD test binary}
data_dir=${FIRESTAFF_CSBWIN_REAL_DATA_DIR:-}
save_path=${FIRESTAFF_CSBWIN_REAL_SAVE:-}

if [ -z "$data_dir" ] || [ -z "$save_path" ]; then
    printf '%s\n' 'SKIP: FIRESTAFF_CSBWIN_REAL_DATA_DIR and FIRESTAFF_CSBWIN_REAL_SAVE are not staged'
    exit 0
fi

if [ ! -f "$data_dir/graphics.dat" ] || [ ! -f "$data_dir/Dungeon.dat" ] ||
   [ ! -f "$save_path" ]; then
    printf '%s\n' 'SKIP: stock CSBWin GRAPHICS.DAT/Dungeon.dat/save corpus is incomplete'
    exit 0
fi

FIRESTAFF_CSBWIN_REAL_DATA_DIR=$data_dir \
FIRESTAFF_CSBWIN_REAL_SAVE=$save_path \
"$test_bin"
