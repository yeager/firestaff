#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_ATARI_ST_FR_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_FR_Version-13.zip"}
expected_md5=0d7af44dd14f383464288abdcec76afc

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic French DM1 Atari ST 1.3 archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq "assetMd5=$expected_md5" <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output" ||
       ! grep -Fq 'map=0 party=1,3,2' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic French DM1 Atari ST start menu did not bind DMCSB1 source media' >&2
    exit 1
fi

# Follow the same normal M12 -> M11 title/entrance handoff as the English
# Atari ST v1.2 route, confirm the authentic Hall choice, then exercise input.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_probe="$app_dir/dm1-atari-st-fr-runtime-$$.json"
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
menu_home="$scratch_root/dm1-atari-st-fr-menu-home-$$"
mkdir -p "$menu_home"
trap 'rm -f "$runtime_probe"; rm -rf "$menu_home"' EXIT
# The default 960x540 host view presents a centered 640x400 game image. This
# point maps to the source C127 portrait hit point (112,83).
m12_hoc_route='enter,enter,enter,wait30,enter,wait60,enter'
for token in up up up up turn-left up up up turn-left \
    up up up up up turn-right up up turn-right up turn-left \
    up up turn-right up turn-left up up turn-left; do
    m12_hoc_route+=",wait30,$token"
done
m12_hoc_route+=',wait30,click:384:236,wait10,click:420:300,wait30,key:kp6,wait60'
HOME="$menu_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" \
    --script "$m12_hoc_route" --duration 45000 >/dev/null 2>&1
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm1-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 10, 4, 1, 1) or
        probe["dm1HoC"] != {"candidatePanel": 0, "candidateOrdinal": -1,
                           "candidatePartyIndex": -1}):
    raise SystemExit(f"FAIL: authentic French DM1 Atari start menu did not confirm C127 ordinal 14 and route live input: {probe}")
print("PASS: authentic French DM1 Atari M12 confirms C127 ordinal 14 and routes live input")
PY

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic French DM1 Atari ST start menu did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic French DM1 Atari ST 1.3 nested ZIP -> ST reaches CLI, menu, and native movement runtime'
