#!/bin/sh
set -eu

save_root=${FIRESTAFF_DM1_DOS_FR_SAVE_DIR:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_FR_unpacked/dungeon-master-fr/dungeon_master"}
primary="$save_root/DMSAVE.DAT"
backup="$save_root/DMSAVE.BAK"
# The five fixed save parts occupy 9,408 bytes after the 512-byte header;
# four 464-byte portraits follow.  The saved DUNGEON_HEADER therefore begins
# at 11,264 (not ten bytes later).  Keep the complete 37,297-byte tail intact:
# it is the source-owned F0434 payload, including its terminal word.
tail_offset=11264
tail_size=37297

if [ ! -f "$primary" ] || [ ! -f "$backup" ]; then
    echo 'SKIP: authentic unpacked French DM1 save pair is not staged'
    exit 77
fi

if [ "$(wc -c < "$primary")" -ne 48561 ] ||
   [ "$(wc -c < "$backup")" -ne 48561 ]; then
    echo 'FAIL: unexpected French DM1 save size' >&2
    exit 1
fi

# F0435's five save sections plus four 32x29 portraits end at this offset.
# The remaining original dungeon payload must stay opaque until its real
# F0434/F0455 framing is decoded; it is identical in the supplied DAT/BAK.
header=$(od -An -tx1 -N 10 -j "$tail_offset" "$primary" | tr -d ' ' | tr -d '\n')
if [ "$header" != 6300fb2f0e00d5066108 ]; then
    echo 'FAIL: French DM1 tail does not begin with its real DUNGEON_HEADER' >&2
    exit 1
fi
primary_tail=$(dd if="$primary" bs=1 skip="$tail_offset" count="$tail_size" 2>/dev/null | sha256sum | awk '{print $1}')
backup_tail=$(dd if="$backup" bs=1 skip="$tail_offset" count="$tail_size" 2>/dev/null | sha256sum | awk '{print $1}')

if [ "$primary_tail" != "$backup_tail" ]; then
    echo 'FAIL: French DM1 save dungeon tail differs between DAT and BAK' >&2
    exit 1
fi

echo "PASS: French DM1 original save tail boundary=$tail_offset bytes=$tail_size sha256=$primary_tail"
