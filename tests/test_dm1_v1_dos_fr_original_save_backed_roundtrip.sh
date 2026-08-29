#!/bin/sh
set -eu

probe=${1:?usage: test_dm1_v1_dos_fr_original_save_backed_roundtrip.sh <probe> <output-dir>}
output_dir=${2:?usage: test_dm1_v1_dos_fr_original_save_backed_roundtrip.sh <probe> <output-dir>}
save_root=${FIRESTAFF_DM1_DOS_FR_SAVE_DIR:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_unpacked/dungeon-master-fr/dungeon_master"}
data_dir="$save_root/EUDATA"

if [ ! -x "$probe" ] || [ ! -f "$save_root/DMSAVE.DAT" ] ||
   [ ! -f "$save_root/DMSAVE.BAK" ] || [ ! -f "$data_dir/DUNGEON.DAT" ] ||
   [ ! -f "$data_dir/GRAPHICS.DAT" ]; then
    echo 'SKIP: authentic unpacked French DM1 save corpus is not staged'
    exit 77
fi

# The probe admits only the supplied DOS envelope and its paired original
# DUNGEON.DAT.  It writes short-lived round-trip outputs under the build
# directory, removes them before returning, and never modifies game data.
FIRESTAFF_DM1_PC34_SAVE_CORPUS="$save_root" \
FIRESTAFF_DM1_PC_DATA="$data_dir" \
FIRESTAFF_TEST_OUTPUT_DIR="$output_dir" \
    "$probe"
