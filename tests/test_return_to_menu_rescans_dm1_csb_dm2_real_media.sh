#!/usr/bin/env sh
set -eu

app=${1:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
dm1_archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
csb_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
dm2_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

if [ ! -x "$app" ] || [ ! -f "$dm1_archive" ] ||
   [ ! -f "$csb_archive" ] || [ ! -f "$dm2_archive" ]; then
    echo 'SKIP: authentic DM1, CSB, and DM2 archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/return-rescan.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM
run_return_to_menu_case() {
    game=$1
    platform=$2
    script=$3
    duration_ms=$4
    runtime_pattern=$5
    launch_pattern=$6
    case_root="$scratch/$game"
    runtime_probe="$case_root/runtime.json"
    run_log="$case_root/firestaff.log"
    mkdir -p "$case_root"

    # Keep a clean per-game config so a previous media selection cannot make
    # an explicit platform appear to work by reusing a cached source.
    set -- --width 320 --height 200 --menu --game "$game"
    if [ "$platform" != auto ]; then
        set -- "$@" --platform "$platform"
    fi
    set -- "$@" --data-dir "$data_root" --debug \
        --script "$script" --duration "$duration_ms"
    FIRESTAFF_CONFIG_PATH="$case_root/config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "$@" >"$run_log" 2>&1 || {
            cat "$run_log" >&2
            return 1
        }

    python3 - "$game" "$run_log" "$runtime_probe" "$runtime_pattern" \
        "$launch_pattern" <<'PY'
import json
import re
import sys

game_id, log_path, probe_path, runtime_pattern, launch_pattern = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

runtime = re.search(runtime_pattern, trace)
launch = re.search(launch_pattern, trace)
complete = re.search(r"return-menu rescan-complete data=", trace)
if (launch is None or runtime is None or complete is None or
        not launch.start() < runtime.start() < complete.start()):
    raise SystemExit(
        f"FAIL: ordinary {game_id} launch did not reach its source handoff and runtime before rescanning")

for scanned_game in ("dm1", "csb", "dm2"):
    match = re.search(
        rf"return-menu rescan game={scanned_game} available=(\d+)", trace)
    if match is None or match.group(1) != "1":
        raise SystemExit(
            f"FAIL: {game_id} return-to-menu did not rediscover authentic {scanned_game} media")

if (probe.get("launchedEver") != 1 or probe.get("active") != 0 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(
        f"FAIL: {game_id} did not finish back at the launcher: {probe}")

print(f"PASS: authentic {game_id} gameplay returned to the launcher and rescanned DM1, CSB, and DM2")
PY
}

# The key sequence is the same ordinary M12 game/platform/original choice
# used by the DM1 PC-34 and CSB FM Towns startup checks. Their source startup
# phases differ, so each case proves its own authentic runtime receipt before
# the shared ESC/Enter return interaction can pass.
run_return_to_menu_case dm1 pc \
    'key:enter,key:enter,key:enter,wait:12,key:escape,key:enter' \
    15000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
    'DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3'
run_return_to_menu_case csb fm-towns \
    'key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:240,back,wait:10,enter' \
    30000 'startup-frame game=csb .*phase=inactive .*level-loaded=1 map=4 party=22,18 dir=2 champions=1' \
    'CSB READY: gameId=csb dataDir=.*/data/csb/.*variant=csb-fmtowns-en .*handoff=f31-title-anm'
run_return_to_menu_case dm2 auto \
    'key:enter,key:enter,key:enter,wait:1800,click:115:65,click:100:60,wait:10,key:escape,key:enter' \
    60000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
    'launch phase=game-handoff mode=menu game=dm2 .*source=.*/data/dm2/'
