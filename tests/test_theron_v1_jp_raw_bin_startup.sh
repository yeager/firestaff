#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_JP_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQJP02.bin"
expected_track01_cdda=0
if [[ ! -f "$track02" ]]; then
    track02="$data_root/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin"
    if [[ -f "$track02" ]]; then
        expected_track01_cdda=1
    fi
fi
expected_md5=b7afb338ad31be1025b53f9aff12d73a
track19_raw="$data_root/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 19).bin"
track19_iso="$data_root/TQJP19.iso"

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron JP Rev. 1 Track 02 BIN is not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-jp-raw-bin.XXXXXX")
audio_cache=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-jp-audio.XXXXXX")
trap 'rm -f "$output"; rm -rf "$audio_cache"' EXIT
if [[ ! -f "$track19_raw" && ! -f "$track19_iso" ]]; then
    printf 'SKIP: authentic Theron JP Track 19 is not staged\n'
    exit 77
fi
FIRESTAFF_THERON_MEDNAFEN_CACHE="$audio_cache" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --theron-native jp \
    --data-dir "$data_root" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,enter,action,tab,up,right,down,left,up,up' \
    --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 2,3,0 \
    --boot-probe-expect-champions 2 \
    --boot-probe-expect-asset-md5 "$expected_md5" \
    --boot-probe-expect-startup-active 0 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-runtime' "$output" ||
   ! grep -Fq 'levelLoaded=1' "$output" ||
   ! grep -Fq 'party=2,3,0 champions=2' "$output" ||
   grep -Fq 'theronSourceObjects=0' "$output" ||
   ! grep -Eq 'theronSourceObjects=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronDungeonLevelsLoaded=4' "$output" ||
   ! grep -Fq 'theronDungeonSourceHeaders=4' "$output" ||
   ! grep -Eq 'theronDungeonSourceNonzeroTiles=[1-9][0-9]*' "$output" ||
   ! grep -Fq 'theronActiveChampion=1' "$output" ||
   # A loose BIN has no inferred Track 01 companion. The archival CUE-split
   # Track 02 file may bind CDDA only through its exact authentic CUE pairing.
   ! grep -Fq "theronTrack01CddaReady=$expected_track01_cdda" "$output" ||
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

printf '%s\n' 'PASS: authentic Theron JP raw BIN reaches runtime and accepts six native movement inputs across its source map'
