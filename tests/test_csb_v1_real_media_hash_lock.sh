#!/bin/sh
set -eu

root=${FIRESTAFF_CSB_REAL_MEDIA_ROOT:-}
if [ -z "$root" ]; then
    root="$HOME/.firestaff/data/csb"
fi

check_media() {
    name=$1
    expected_size=$2
    expected_sha256=$3
    path="$root/$name"

    if [ ! -f "$path" ]; then
        echo "SKIP: CSB real-media corpus is not staged: $path"
        exit 77
    fi
    if [ "$(wc -c < "$path")" -ne "$expected_size" ]; then
        echo "FAIL: CSB real-media size drift: $path" >&2
        exit 1
    fi
    actual_sha256=$(sha256sum "$path" | awk '{print $1}')
    if [ "$actual_sha256" != "$expected_sha256" ]; then
        echo "FAIL: CSB real-media identity drift: $path" >&2
        exit 1
    fi
}

# These are the exact local original-media inputs used by the native Atari ST,
# Amiga, and FM Towns launch tests. They remain archives/images in place:
# Firestaff must read them directly and may not replace them with a cache,
# extracted copy, or sibling edition.
check_media 'Chaos Strikes Back.stx' 411568 \
    d9aed23f7916d60dfef61c7b79bc3eb1995f8afbb6a6c8b7b4160ee12ada1025
check_media 'Chaos Strikes Back Utility.stx' 411568 \
    0cfa4babc34b6e2b43367e0ab4729c33885b00f9ca37282db616c41a2b8c4f13
check_media 'Chaos Strikes Back (FTL).zip' 1159589 \
    bff24bffd2e07ff8ac4c96b3c35be93ee3fdfe219d1bf833df247726b2ce3abb
check_media 'Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip' 1192538 \
    fa0296c7bf62d806e8b1c45542d8ac225b67f4080026fa7f6fb61c97905f24c0
check_media 'Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip' 429846618 \
    54b40c1fd0b18ca2df1dbcd70c6cb07fb0333d540b99f8108b1fca84aa192b88

echo 'PASS: CSB real-media Atari ST, utility, Amiga, and FM Towns identities are locked'
