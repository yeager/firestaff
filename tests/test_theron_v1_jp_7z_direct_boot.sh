#!/usr/bin/env bash
set -euo pipefail

firestaff=${1:?usage: test_theron_v1_jp_7z_direct_boot.sh FIRESTAFF}
archive=${FIRESTAFF_THERON_JP_7Z:-}
wrong_named_candidate=${FIRESTAFF_THERON_JP_WRONG_NAMED_CANDIDATE:-}
if [[ -z "$archive" ]]; then
    archive="$HOME/.firestaff/data/theron/Dungeon Master - Theron's Quest (Japan).7z"
fi
if [[ -z "$wrong_named_candidate" ]]; then
    wrong_named_candidate="$HOME/.firestaff/data/theron/TQJP02End.iso"
fi
if [[ ! -f "$archive" ]]; then
    printf 'test_theron_v1_jp_7z_direct_boot: SKIP (authentic Japanese 7z not found)\n'
    exit 77
fi
if ! command -v 7zz >/dev/null 2>&1 && ! command -v 7z >/dev/null 2>&1; then
    printf 'test_theron_v1_jp_7z_direct_boot: SKIP (7zz/7z not installed)\n'
    exit 77
fi

data_root=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-jp-7z.XXXXXX")
trap 'rm -rf "$data_root"' EXIT
ln -s "$archive" "$data_root/japan.7z"
# This is a real but different Japanese disc image. Giving it the expected
# filename proves that native selection is content-hash-driven, not name-led.
if [[ -f "$wrong_named_candidate" ]]; then
    ln -s "$wrong_named_candidate" "$data_root/TQJP02.bin"
fi
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff" --theron-native jp \
    --enable-external-archive-tools \
    --data-dir "$data_root" \
    --boot-probe --duration 0 2>&1)
printf '%s\n' "$output"
grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output"
grep -Fq 'assetMd5=b7afb338ad31be1025b53f9aff12d73a' <<<"$output"
grep -Fq 'japan.7z::Dungeon Master - Theron' <<<"$output"
printf 'test_theron_v1_jp_7z_direct_boot: PASS (original media, no extraction)\n'
