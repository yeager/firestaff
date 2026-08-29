#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_CSB_AMIGA_ADF_ARCHIVE:-"$HOME/.firestaff/data/csb/Chaos Strikes Back (FTL).zip"}
expected_md5=21197b1d4994fd835c403d5a33dcac2b

# The selected ZIP -> ADF chain is read directly in bounded memory.  Keep
# this real-media contract independent of diagnostic external extractors.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic CSB Amiga ADF archive is not staged'
    exit 77
fi

run_probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    printf '%s\n' "$output"
}

title_output=$(run_probe --game csb --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0)
if ! grep -Fq "assetMd5=$expected_md5" <<<"$title_output" ||
   ! grep -Fq 'sourceKind=5 sourceId=csb' <<<"$title_output" ||
   ! grep -Fq 'phase=csb-entrance-0' <<<"$title_output" ||
   ! grep -Fq 'startupActive=1' <<<"$title_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$title_output"; then
    printf '%s\n' "$title_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF archive did not retain its source entrance phase' >&2
    exit 1
fi

movement_output=$(run_probe --menu --game csb --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 800 --script enter,enter,enter,up --duration 0)
if ! grep -Fq 'phase=inactive' <<<"$movement_output" ||
   ! grep -Fq 'startupActive=0' <<<"$movement_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$movement_output" ||
   ! grep -Fq 'party=9,1,2' <<<"$movement_output" ||
   ! grep -Fq 'runtimeTick=' <<<"$movement_output"; then
    printf '%s\n' "$movement_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF archive start menu did not reach native movement' >&2
    exit 1
fi

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game csb --platform amiga --data-dir "$archive" \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'CSB READY: gameId=csb' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'route=startup' <<<"$menu_output" ||
   ! grep -Eq 'handoffHash=[0-9a-f]{8}' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF start-menu launch did not retain its archive source' >&2
    exit 1
fi
case "$menu_output" in
    *"variant=csb-amiga-a31m"*"handoff=a31m-titl-dat"*) ;;
    *"variant=csb-amiga-a31e"*"handoff=a31e-appb-bjeload-c03"*) ;;
    *)
        printf '%s\n' "$menu_output" >&2
        printf '%s\n' 'FAIL: authentic CSB Amiga ADF launch did not publish its A31 handoff' >&2
        exit 1
        ;;
esac

# Amiga is the second platform card.  This exercises the actual mouse-only
# CSB -> Amiga -> Original card route against the supplied ZIP -> ADF media.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game csb --platform amiga \
    --data-dir "$archive" \
    --script 'wait20,click:1173:262,wait20,click:934:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

printf '%s\n' 'PASS: authentic CSB Amiga ZIP -> ADF route reaches source entrance, menu launch, and native movement'
