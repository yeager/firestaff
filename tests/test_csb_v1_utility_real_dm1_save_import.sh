#!/bin/sh
set -eu

handoff_test=${1:?usage: test_csb_v1_utility_real_dm1_save_import.sh <handoff-test>}
utility_root=${FIRESTAFF_CSB_REAL_UTILITY_ROOT:-"$HOME/.firestaff/data/csb"}
dm1_save=${FIRESTAFF_DM1_REAL_SAVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_unpacked/dungeon-master-fr/dungeon_master/DMSAVE.DAT"}

if [ ! -x "$handoff_test" ] || [ ! -f "$dm1_save" ] ||
   [ ! -f "$utility_root/Chaos Strikes Back Utility.stx" ]; then
    echo 'SKIP: authentic DM1 save or CSB Utility Disk is not staged'
    exit 77
fi

# The handoff target contains the source-owned CEDTINC7.C utility flow.
# Supplying the original DM1 file is deliberate: do not fall back to its
# bounded CI fixture when real media is present.
output=$(FIRESTAFF_CSB_REAL_UTILITY_ROOT="$utility_root" \
    FIRESTAFF_DM1_REAL_SAVE="$dm1_save" \
    "$handoff_test" 2>&1) || {
    printf '%s\n' "$output" >&2
    exit 1
}

if ! printf '%s\n' "$output" | grep -Fq \
    'real DM1 save and Utility ADF complete the original import route' ||
   ! printf '%s\n' "$output" | grep -Fq 'FAILED: 0'; then
    printf '%s\n' "$output" >&2
    echo 'FAIL: authentic DM1 save did not complete the native CSB Utility import route' >&2
    exit 1
fi

echo 'PASS: authentic DM1 save completes native CSB Utility Disk import without extraction'
