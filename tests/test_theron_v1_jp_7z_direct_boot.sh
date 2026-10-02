#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 || ! -x "$1" || ( "$2" != us && "$2" != jp ) ]]; then
    printf 'usage: %s <firestaff> <us|jp>\n' "$0" >&2
    exit 2
fi

firestaff=$1
region=$2
if [[ "$region" == jp ]]; then
    archive=${FIRESTAFF_THERON_JP_7Z:-"$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan).7z"}
    digest=b7afb338ad31be1025b53f9aff12d73a
    member_prefix="japan.7z::Dungeon Master - Theron's Quest"
    archive_alias=japan.7z
else
    archive=${FIRESTAFF_THERON_US_7Z:-"$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (USA).7z"}
    digest=f23601102138f87c33025877767ebf76
    member_prefix="usa.7z::Dungeon Master - Theron's Quest"
    archive_alias=usa.7z
fi
if [[ ! -f "$archive" ]]; then
    printf 'test_theron_v1_%s_7z_direct_boot: SKIP (authentic %s 7z not found)\n' \
        "$region" "$region"
    exit 77
fi
if ! command -v 7zz >/dev/null 2>&1 && ! command -v 7z >/dev/null 2>&1; then
    printf 'test_theron_v1_%s_7z_direct_boot: SKIP (7zz/7z not installed)\n' "$region"
    exit 77
fi

data_root=$(mktemp -d "$(dirname "$1")/firestaff-theron-${region}-7z.XXXXXX")
trap 'rm -rf "$data_root"' EXIT
ln -s "$archive" "$data_root/$archive_alias"
# This is a real but different Japanese disc image. Giving it the expected
# filename proves that native selection is content-hash-driven, not name-led.
if [[ "$region" == jp ]]; then
    wrong_named_candidate=${FIRESTAFF_THERON_JP_WRONG_NAMED_CANDIDATE:-"$HOME/.firestaff/data/theron/TQJP02End.iso"}
    if [[ -f "$wrong_named_candidate" ]]; then
        ln -s "$wrong_named_candidate" "$data_root/TQJP02.bin"
    fi
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff" --theron-native "$region" \
    --enable-external-archive-tools \
    --data-dir "$data_root" \
    --boot-probe --duration 0 2>&1)
printf '%s\n' "$output"
grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output"
grep -Fq "assetMd5=$digest" <<<"$output"
grep -Fq "$member_prefix" <<<"$output"
grep -Fq 'theronTrack01CddaReady=1' <<<"$output"
printf 'test_theron_v1_%s_7z_direct_boot: PASS (original media, no extraction)\n' "$region"
