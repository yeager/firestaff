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
archive=${FIRESTAFF_DM1_ATARI_NESTED_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip"}
# Both authenticated English Atari ST v1.2 media revisions are supported.
# The nested image distributed as "Dungeon Master V1.2 (1987)(FTL)(en)[!]"
# has the latter GRAPHICS.DAT identity; do not reject it merely because a
# different preservation dump was used when this probe was first written.
expected_md5s=(
    b3cfd84e44cdf07ce2eeba47e87f772b
    9ce2eaf7a9e78620e3f17594437caffa
)

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic nested DM1 Atari ST archive is not staged'
    exit 77
fi

probe() {
    local output
    local matched=0
    local expected_md5
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    for expected_md5 in "${expected_md5s[@]}"; do
        if grep -Fq "assetMd5=$expected_md5" <<<"$output"; then
            matched=1
            break
        fi
    done
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       [[ $matched -ne 1 ]] ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
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
    printf '%s\n' "$menu_output" >&2; exit 1;
}
grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" &&
grep -Fq "dataDir=$archive" <<<"$menu_output" &&
grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"

# Unlike --boot-probe, this follows the ordinary M12 → M11 path with the
# authenticated nested archive. The first three Enter inputs select the game,
# Atari card and Original options; subsequent source inputs advance the Atari
# title/entrance owner. Keep the receipt tied to the real menu handoff so a
# direct-launch probe cannot satisfy this first-runtime assertion.
test_scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$test_scratch"
menu_probe_json="$test_scratch/dm1-atari-menu-runtime-$$.json"
menu_home="$test_scratch/dm1-atari-menu-home-$$"
mkdir -p "$menu_home"
trap 'rm -f "$menu_probe_json"; rm -rf "$menu_home"' EXIT
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
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" \
    --script "$m12_hoc_route" --duration 45000 >/dev/null 2>&1
python3 - "$menu_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm1-runtime" or
        (probe["party"]["mapIndex"], probe["party"]["mapX"],
         probe["party"]["mapY"], probe["party"]["direction"],
         probe["party"]["championCount"]) != (0, 10, 4, 1, 1) or
        probe["dm1HoC"] != {"candidatePanel": 0, "candidateOrdinal": -1,
                           "candidatePartyIndex": -1}):
    raise SystemExit(f"FAIL: authentic DM1 Atari start menu did not confirm C127 ordinal 14 and route input: {probe}")
print("PASS: authentic DM1 Atari ST start menu confirms C127 ordinal 14 and routes live input")
PY

# The third platform card is Atari ST.  This verifies pointer-only card
# selection against the supplied nested preservation archive.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform atari-st \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

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
    printf '%s\n' 'FAIL: authentic DM1 Atari ST up input did not reach native movement' >&2
    exit 1
fi

# Run each initial action from a fresh original ZIP → ZIP → STX session. This
# retains only source-owned title, dungeon and party state; no generated save,
# replacement STX, or synthetic map is admitted into the input matrix.
probe_runtime_input() {
    local input=$1
    local expected_party=$2
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform atari-st --data-dir "$archive" \
        --boot-probe --boot-probe-frames 500 --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output" &&
    grep -Fq "map=0 party=$expected_party" <<<"$output"
}

probe_runtime_input down 1,3,2
probe_runtime_input left 1,3,1
probe_runtime_input right 1,3,3
probe_runtime_input strafe-left 1,3,2
probe_runtime_input strafe-right 1,3,2
probe_runtime_input action 1,3,2

printf '%s\n' 'PASS: authentic DM1 nested Atari ZIP -> ZIP -> STX reaches CLI, confirms M12 Hall choice, and passes native input matrix'
