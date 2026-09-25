#!/bin/sh
set -eu

root=${FIRESTAFF_DM2_REAL_MEDIA_ROOT:-"$HOME/.firestaff/data/dm2"}

check_media() {
    name=$1
    expected_size=$2
    expected_sha256=$3
    path="$root/$name"
    duplicate_path="${path%.zip} (1).zip"
    found=0
    for candidate in "$path" "$duplicate_path"; do
        if [ ! -f "$candidate" ]; then
            continue
        fi
        found=1
        if [ "$(wc -c < "$candidate")" -ne "$expected_size" ]; then
            continue
        fi
        actual_sha256=$(sha256sum "$candidate" | awk '{print $1}')
        if [ "$actual_sha256" = "$expected_sha256" ]; then
            return 0
        fi
    done
    if [ "$found" -eq 0 ]; then
        echo "SKIP: DM2 real-media corpus is not staged: $path (or duplicate-suffixed archive)"
        exit 77
    fi
    echo "FAIL: DM2 real-media identity not found as $path or $duplicate_path" >&2
    exit 1
}

# Exact local original-media inputs used by the native launch tests. Archives
# remain in place and are read directly: Firestaff must not substitute an
# extracted cache, a sibling edition, or synthetic assets. The Macintosh demo
# is deliberately excluded because it has no supported runtime route.
check_media 'Dungeon-Master-II-Skullkeep_DOS_EN.zip' 13203537 \
    d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929
check_media 'Dungeon-Master-II-Skullkeep_DOS_FR.zip' 17745368 \
    265ea1564334735212d1460b277aece0ce719a5875222ab6a8bd7b3e37981923
check_media 'Dungeon-Master-II-Skullkeep_Amiga_EN.zip' 13616735 \
    fb3bdcb19039f449389c5872aedcf08e53a1765ebd57fd72e60a1e50dbe74768
check_media 'Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip' 148912573 \
    ed68fd6dc0072d7da9e29f87049fecf50da4414b938b140bcfdda5b9d5e69708
check_media 'Dungeon-Master-II-Skullkeep_Mac_EN.zip' 561275977 \
    dfea2fe53988784bc1525d6ad90ab881d260b74cd6a855e0eb35d04001ecfe5b

echo 'PASS: DM2 real-media DOS, Amiga, FM Towns, and Macintosh identities are locked'
