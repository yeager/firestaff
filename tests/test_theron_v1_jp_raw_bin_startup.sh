#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_JP_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQJP02.bin"
expected_md5=b7afb338ad31be1025b53f9aff12d73a

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron JP raw Track 02 BIN is not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-jp-raw-bin.XXXXXX")
audio_cache=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-jp-audio.XXXXXX")
trap 'rm -f "$output"; rm -rf "$audio_cache"' EXIT
jp_archive="$data_root/Dungeon Master - Theron's Quest (Japan).7z"
if [[ ! -f "$jp_archive" ]]; then
    jp_archive="$data_root/Dungeon Master - Theron's Quest (Japan) (1).7z"
fi
if [[ ! -f "$jp_archive" ]]; then
    printf 'SKIP: authentic Theron JP full-disc archive is not staged\n'
    exit 77
fi
FIRESTAFF_THERON_MEDNAFEN_CACHE="$audio_cache" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --theron-native jp \
    --data-dir "$data_root" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,enter,action,tab' \
    --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 1,0,0 \
    --boot-probe-expect-champions 2 \
    --boot-probe-expect-asset-md5 "$expected_md5" \
    --boot-probe-expect-startup-active 0 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-runtime' "$output" ||
   ! grep -Fq 'levelLoaded=1' "$output" ||
   ! grep -Fq 'party=1,0,0 champions=2' "$output" ||
   grep -Fq 'theronSourceObjects=0' "$output" ||
   ! grep -Eq 'theronSourceObjects=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronDungeonLevelsLoaded=4' "$output" ||
   ! grep -Fq 'theronDungeonSourceHeaders=4' "$output" ||
   ! grep -Eq 'theronDungeonSourceNonzeroTiles=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronActiveChampion=1' "$output" ||
   # The selected source is the loose, hash-verified Track 02 BIN.  A staged
   # archive elsewhere in the directory must not be inferred as its Track 01
   # companion; CDDA is admitted only through an exact CUE pairing.
   ! grep -Fq 'theronTrack01CddaReady=0' "$output" ||
   ! grep -Fq 'theronSpawnSourceAuthenticated=1' "$output" ||
   ! grep -Fq 'theronSpawnSourceVariant=1' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameBanks=7' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameVariant=1' "$output" ||
   ! grep -Fq 'theronTrack19NameBankReady=1' "$output" ||
   ! grep -Fq 'theronTrack19NameVariant=1' "$output" ||
   ! grep -Fq 'theronTrack19ItemMappingProven=1' "$output" ||
   grep -Fq 'deterministic fallback assets' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron JP raw BIN did not reach the source-backed runtime route' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron JP raw BIN reaches runtime with source map, party and object records'
