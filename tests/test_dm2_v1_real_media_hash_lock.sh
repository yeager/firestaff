#!/bin/sh
set -eu

root=${FIRESTAFF_DM2_REAL_MEDIA_ROOT:-"$HOME/.firestaff/data/dm2"}

check_media() {
    name=$1
    shift
    identities=$*
    path="$root/$name"
    duplicate_path="${path%.zip} (1).zip"
    found=0
    for candidate in "$path" "$duplicate_path"; do
        if [ ! -f "$candidate" ]; then
            continue
        fi
        found=1
        actual_sha256=$(sha256sum "$candidate" | awk '{print $1}')
        actual_size=$(wc -c < "$candidate" | tr -d ' ')
        set -- $identities
        while [ "$#" -ge 2 ]; do
            expected_size=$1
            expected_sha256=$2
            shift 2
            if [ "$actual_size" = "$expected_size" ] &&
               [ "$actual_sha256" = "$expected_sha256" ]; then
                return 0
            fi
        done
    done
    if [ "$found" -eq 0 ]; then
        echo "SKIP: DM2 real-media corpus is not staged: $path (or duplicate-suffixed archive)"
        exit 77
    fi
    echo "FAIL: DM2 real-media identity not found as $path or $duplicate_path" >&2
    exit 1
}

# Exact local original-media inputs used by the native launch tests. Archives
# remain in place and are read directly: Firestaff must not substitute a
# sibling edition or synthetic assets. The English Macintosh source is the
# original StuffIt archive; its extracted catalog-authenticated files are used
# by the native launch tests.
# Keep the established container identity and the exact installed archive
# identity. The installed archive's GRAPHICS.DAT and DUNGEON.DAT match the
# original-media catalog; unknown repacks remain rejected.
check_media 'Dungeon-Master-II-Skullkeep_DOS_EN.zip' \
    13203537 d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929 \
    17624319 29fa346b85c1be5a636c5e59159b49b230f28e344c64dbbf19e6659c717c13bf
check_media 'Dungeon-Master-II-Skullkeep_DOS_FR.zip' 17745368 \
    265ea1564334735212d1460b277aece0ce719a5875222ab6a8bd7b3e37981923
check_media 'Dungeon-Master-II-Skullkeep_Amiga_EN.zip' 13616735 \
    fb3bdcb19039f449389c5872aedcf08e53a1765ebd57fd72e60a1e50dbe74768
# The installed CloneCD ZIP contains the catalog-authenticated original
# track01.iso and FM Towns GRAPHICS.DAT; unknown repacks remain rejected.
check_media 'Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip' \
    148912573 ed68fd6dc0072d7da9e29f87049fecf50da4414b938b140bcfdda5b9d5e69708 \
    223151745 3db899e6dcf4328e31ec4c154d224667c68e83d2c24b31e0d026986c757a7cb1
check_media 'Dungeon Master II for Macintosh English.sit' 23693732 \
    bcb4153bfad6c72a78f5b1c9c191b9aa3fcff552dfea215fb922514a15fae9d3

echo 'PASS: DM2 real-media DOS, Amiga, FM Towns, and Macintosh identities are locked'
