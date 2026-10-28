#!/usr/bin/env sh
set -eu
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm2_v1_mac_extracted_native_cli_boot.sh <firestaff>}
data_dir=${FIRESTAFF_DM2_MAC_EXTRACTED_DIR:-"$HOME/.firestaff/data/dm2/mac-en/Dungeon Master II/DMFiles"}
data_root=${FIRESTAFF_DATA:-"$HOME/.firestaff/data"}
scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}

if [ ! -x "$app" ]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi
for name in Graphics.dat Dungeon.dat Title.MooV; do
    if [ ! -f "$data_dir/$name" ]; then
        echo "SKIP: authentic extracted DM2 Macintosh media is unavailable ($name)"
        exit 77
    fi
done

mkdir -p "$scratch"
probe="$scratch/dm2-mac-extracted-$$.json"
output="$scratch/dm2-mac-extracted-$$.log"
menu_probe="$scratch/dm2-mac-extracted-menu-$$.json"
menu_output="$scratch/dm2-mac-extracted-menu-$$.log"
cleanup() { rm -f "$probe" "$output" "$menu_probe" "$menu_output"; }
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

graphics_before=$(firestaff_sha256_file "$data_dir/Graphics.dat")
dungeon_before=$(firestaff_sha256_file "$data_dir/Dungeon.dat")
movie_before=$(firestaff_sha256_file "$data_dir/Title.MooV")

if ! FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$data_dir" \
    --width 320 --height 200 \
    --script 'wait:1200,key:enter,wait:30,click:115:65,wait:30,click:112:130,wait:30,key:right,wait:30,key:s,wait:30,key:up' \
    --duration 45000 >"$output" 2>&1; then
    cat "$output" >&2
    exit 1
fi

python3 - "$probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

startup = probe.get("startup", {})
movie = probe.get("dm2Startup", {})
party = probe.get("party", {})
frame = probe.get("dm2RuntimeFrame", {})
output = probe.get("outputSize", {})
script = probe.get("script", {})
if (
    probe.get("sourceId") != "dm2"
    or probe.get("bootAssetMd5") != "5cab25f6b975957eae4a203174e7f2a6"
    or movie.get("platform") != 4
    or movie.get("movieActive") != 0
    or movie.get("movieComplete") != 1
    or movie.get("movieRejected") != 0
    or startup.get("phase") != "dm2-runtime"
    or startup.get("startupActive") != 0
    or startup.get("levelLoaded") != 1
    or (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
        party.get("direction"), party.get("championCount")) != (0, 3, 8, 1, 1)
    or frame != {"accepted": 1, "realAssets": 1,
                 "noCoreFallbacks": 1, "fallbackDraws": 0}
    or script.get("waitFramesRemaining") != 0
    or script.get("pending") != 0
    or output.get("valid") != 1
    or (output.get("windowWidth"), output.get("windowHeight"),
        output.get("drawableWidth"), output.get("drawableHeight")) !=
       (320, 200, 320, 200)
):
    raise SystemExit(f"FAIL: extracted original DM2 Macintosh route: {probe}")
PY

if ! FIRESTAFF_DATA="$data_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --scale-mode 0 --width 1920 --height 1080 --menu --game dm2 --platform mac \
    --script 'wait20,click:1645:262,wait20,click:410:679,wait20,click:450:405,wait20,wait:1200,key:enter,click:900:500,wait:20,click:1074:580,wait:30,key:up,wait:30,key:right,wait:30,key:up,wait:30,key:up' \
    --duration 44000 >"$menu_output" 2>&1; then
    cat "$menu_output" >&2
    exit 1
fi

python3 - "$menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

startup = probe.get("startup", {})
movie = probe.get("dm2Startup", {})
party = probe.get("party", {})
frame = probe.get("dm2RuntimeFrame", {})
output = probe.get("outputSize", {})
script = probe.get("script", {})
if (
    probe.get("sourceId") != "dm2"
    or probe.get("bootAssetMd5") != "5cab25f6b975957eae4a203174e7f2a6"
    or probe.get("launchedEver") != 1
    or probe.get("active") != 1
    or movie.get("platform") != 4
    or movie.get("movieActive") != 0
    or movie.get("movieComplete") != 1
    or movie.get("movieRejected") != 0
    or startup.get("phase") != "dm2-runtime"
    or startup.get("startupActive") != 0
    or startup.get("levelLoaded") != 1
    or (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
        party.get("direction"), party.get("championCount")) != (0, 3, 6, 1, 1)
    or frame != {"accepted": 1, "realAssets": 1,
                 "noCoreFallbacks": 1, "fallbackDraws": 0}
    or script.get("waitFramesRemaining") != 0
    or script.get("pending") != 0
    or output.get("valid") != 1
    or (output.get("windowWidth"), output.get("windowHeight"),
        output.get("drawableWidth"), output.get("drawableHeight")) !=
       (1920, 1080, 1920, 1080)
):
    raise SystemExit(f"FAIL: extracted original DM2 Macintosh menu route: {probe}")
PY

[ "$graphics_before" = "$(firestaff_sha256_file "$data_dir/Graphics.dat")" ]
[ "$dungeon_before" = "$(firestaff_sha256_file "$data_dir/Dungeon.dat")" ]
[ "$movie_before" = "$(firestaff_sha256_file "$data_dir/Title.MooV")" ]
echo 'PASS: extracted original DM2 Macintosh CLI and start menu complete title, New Game, mirror selection and movement'
