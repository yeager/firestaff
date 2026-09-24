#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQUS02.bin"
expected_md5=f23601102138f87c33025877767ebf76

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron USA raw Track 02 BIN is not staged\n'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/build/test-scratch"}
mkdir -p "$scratch_root"
output=$(mktemp "$scratch_root/firestaff-theron-raw-bin.XXXXXX")
us_only_root=$(mktemp -d "$scratch_root/firestaff-theron-us-only.XXXXXX")
trap 'rm -f "$output"; rm -rf "$us_only_root"' EXIT
ln -s "$track02" "$us_only_root/TQUS02.bin"

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --theron-native us \
    --data-dir "$data_root" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,down,down,down,down,down,down,enter,down,enter,tab' \
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
   ! grep -Fq 'theronTrack01CddaReady=0' "$output" ||
   ! grep -Fq 'theronSpawnSourceAuthenticated=1' "$output" ||
   ! grep -Fq 'theronSpawnSourceVariant=2' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameBanks=7' "$output" ||
   ! grep -Fq 'theronTrack02ItemNameVariant=2' "$output" ||
   ! grep -Fq 'theronTrack19NameBankReady=1' "$output" ||
   ! grep -Fq 'theronTrack19NameVariant=2' "$output" ||
   ! grep -Fq 'theronTrack19ItemMappingProven=1' "$output" ||
   grep -Fq 'deterministic fallback assets' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron USA raw BIN did not reach the source-backed runtime route' >&2
    exit 1
fi

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game theron --platform pce \
    --data-dir "$track02" \
    --script 'wait2,click:1173:728,wait2,click:410:405,wait2,click:450:405,wait2' \
    --duration 5000 2>&1) || {
        printf '%s\n' "$menu_output" >&2
        printf '%s\n' 'FAIL: authentic raw Track 02 did not launch through mouse-selected M12 cards' >&2
        exit 1
    }
if ! grep -Fq '[TQR] Verified Track 02 accepted:' <<<"$menu_output" ||
   grep -Fq 'deterministic fallback assets' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: mouse-selected M12 cards lost the authentic Track 02 route' >&2
    exit 1
fi

set +e
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron --theron-native jp --data-dir "$us_only_root" \
    --boot-probe --duration 0 >"$output" 2>&1
wrong_region_rc=$?
set -e
if [[ $wrong_region_rc -ne 2 ]] ||
   ! grep -Fq 'that region' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: JP native selection borrowed the available USA Track 02' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron USA raw BIN reaches runtime with source map, party and object records'
