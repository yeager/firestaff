#!/usr/bin/env sh
set -eu

app=${1:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
test_mode=${3:-all}
dm1_archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
dm1_fmtowns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
csb_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
dm2_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

if [ ! -x "$app" ] || [ ! -f "$dm1_archive" ] ||
   [ ! -f "$dm1_fmtowns_archive" ] ||
   [ ! -f "$csb_archive" ] || [ ! -f "$dm2_archive" ]; then
    echo 'SKIP: authentic DM1, CSB, and DM2 archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/.codex-scratch"}}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/return-rescan.XXXXXX")
mutation_scratch=
addition_scratch=
cleanup() {
    rm -rf "$scratch"
    if [ -n "$mutation_scratch" ]; then
        rm -rf "$mutation_scratch"
    fi
    if [ -n "$addition_scratch" ]; then
        rm -rf "$addition_scratch"
    fi
}
trap cleanup EXIT HUP INT TERM
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
    HOME="$case_root" XDG_CONFIG_HOME="$case_root" APPDATA="$case_root" \
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
if [ "$test_mode" = all ] || [ "$test_mode" = returns ]; then
    run_return_to_menu_case dm1 pc \
        'key:enter,key:enter,key:enter,wait:12,key:escape,key:enter' \
        15000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
        'DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3'
    run_return_to_menu_case dm1 fmtowns \
        'key:enter,key:enter,key:enter,wait:5,key:escape,wait:10,key:enter' \
        30000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
        'launch phase=game-handoff mode=menu game=dm1 platform=FM Towns edition=fmtowns-en source=.*/data/dm1/.*GRAPHICS.DAT'
    run_return_to_menu_case csb fm-towns \
        'key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:240,back,wait:10,enter' \
        30000 'startup-frame game=csb .*phase=inactive .*level-loaded=1 map=4 party=22,18 dir=2 champions=1' \
        'CSB READY: gameId=csb dataDir=.*/data/csb/.*variant=csb-fmtowns-en .*handoff=f31-title-anm'
    run_return_to_menu_case dm2 auto \
        'key:enter,key:enter,key:enter,wait:1800,click:115:65,click:100:60,wait:10,key:escape,key:enter' \
        60000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
        'launch phase=game-handoff mode=menu game=dm2 platform=FM Towns edition=fmtowns-ja source=.*/data/dm2/'
fi

# Prove the return scan refreshes availability after the player changes the
# installed media while gameplay is active. Stage only genuine archives using
# hard links where possible (symlinks on other volumes); removing a staged
# link never alters the original archive.
if [ "$test_mode" = all ] || [ "$test_mode" = mutations ]; then
mutation_scratch=$(mktemp -d "$scratch_root/return-rescan-mutation.XXXXXX")
mutation_data_root="$mutation_scratch/data"
mutation_home="$mutation_scratch/home"
mutation_log="$mutation_scratch/firestaff.log"
mutation_probe="$mutation_scratch/runtime.json"
mkdir -p "$mutation_data_root/dm1" "$mutation_data_root/csb" \
    "$mutation_data_root/dm2" "$mutation_home"
for media_pair in \
    "$dm1_archive|$mutation_data_root/dm1/$(basename "$dm1_archive")" \
    "$csb_archive|$mutation_data_root/csb/$(basename "$csb_archive")" \
    "$dm2_archive|$mutation_data_root/dm2/$(basename "$dm2_archive")"; do
    source_archive=${media_pair%%|*}
    staged_archive=${media_pair#*|}
    if ! ln "$source_archive" "$staged_archive" 2>/dev/null; then
        ln -s "$source_archive" "$staged_archive"
    fi
done

mutation_script='key:enter,key:enter,key:enter,wait:12,key:escape,key:enter'
mutation_runtime_pattern='startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2'
FIRESTAFF_CONFIG_PATH="$mutation_home/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$mutation_probe" \
HOME="$mutation_home" XDG_CONFIG_HOME="$mutation_home" \
APPDATA="$mutation_home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game dm1 --platform pc \
    --data-dir "$mutation_data_root" --debug --script "$mutation_script" \
    --duration 30000 >"$mutation_log" 2>&1 &
mutation_pid=$!
poll=0
while ! grep -Eq "$mutation_runtime_pattern" "$mutation_log" 2>/dev/null; do
    if ! kill -0 "$mutation_pid" 2>/dev/null; then
        wait "$mutation_pid" || true
        echo 'FAIL: authentic DM1 did not reach gameplay before the media mutation' >&2
        cat "$mutation_log" >&2
        exit 1
    fi
    poll=$((poll + 1))
    if [ "$poll" -ge 600 ]; then
        kill "$mutation_pid" 2>/dev/null || true
        wait "$mutation_pid" || true
        echo 'FAIL: timed out waiting for authentic DM1 gameplay before the media mutation' >&2
        cat "$mutation_log" >&2
        exit 1
    fi
    sleep 0.05
done
rm "$mutation_data_root/csb/$(basename "$csb_archive")"
if ! wait "$mutation_pid"; then
    cat "$mutation_log" >&2
    exit 1
fi
python3 - "$mutation_log" "$mutation_probe" "$mutation_runtime_pattern" <<'PY'
import json
import re
import sys

log_path, probe_path, runtime_pattern = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

launch = re.search(
    r"DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3", trace)
runtime = re.search(runtime_pattern, trace)
complete = re.search(r"return-menu rescan-complete data=", trace)
if launch is None or runtime is None or complete is None or not (
        launch.start() < runtime.start() < complete.start()):
    raise SystemExit("FAIL: authentic DM1 did not return to the launcher after gameplay")

for game, expected in (("dm1", "1"), ("csb", "0"), ("dm2", "1")):
    match = re.search(rf"return-menu rescan game={game} available=(\d+)", trace)
    if match is None or match.group(1) != expected:
        raise SystemExit(
            f"FAIL: return scan did not refresh {game} availability to {expected}")

if probe.get("launchedEver") != 1 or probe.get("active") != 0:
    raise SystemExit(f"FAIL: DM1 did not finish back at the launcher: {probe}")
print("PASS: authentic DM1 return scan reflects removed CSB media and retains DM1/DM2")
PY
rm -rf "$mutation_scratch"
mutation_scratch=

# Verify the inverse transition with original media: a game archive added
# while DM1 is running must become available after returning to M12. This
# models the user granting folder access or installing another title without
# restarting Firestaff.
addition_scratch=$(mktemp -d "$scratch_root/return-rescan-addition.XXXXXX")
addition_data_root="$addition_scratch/data"
addition_home="$addition_scratch/home"
addition_log="$addition_scratch/firestaff.log"
addition_probe="$addition_scratch/runtime.json"
mkdir -p "$addition_data_root/dm1" "$addition_data_root/dm2" "$addition_home"
for media_pair in \
    "$dm1_archive|$addition_data_root/dm1/$(basename "$dm1_archive")" \
    "$dm2_archive|$addition_data_root/dm2/$(basename "$dm2_archive")"; do
    source_archive=${media_pair%%|*}
    staged_archive=${media_pair#*|}
    if ! ln "$source_archive" "$staged_archive" 2>/dev/null; then
        ln -s "$source_archive" "$staged_archive"
    fi
done
addition_runtime_pattern='startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2'
FIRESTAFF_CONFIG_PATH="$addition_home/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$addition_probe" \
HOME="$addition_home" XDG_CONFIG_HOME="$addition_home" \
APPDATA="$addition_home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game dm1 --platform pc \
    --data-dir "$addition_data_root" --debug \
    --script 'key:enter,key:enter,key:enter,wait:12,key:escape,key:enter' \
    --duration 60000 >"$addition_log" 2>&1 &
addition_pid=$!
poll=0
while ! grep -Eq "$addition_runtime_pattern" "$addition_log" 2>/dev/null; do
    if ! kill -0 "$addition_pid" 2>/dev/null; then
        wait "$addition_pid" || true
        echo 'FAIL: authentic DM1 did not reach gameplay before adding CSB media' >&2
        cat "$addition_log" >&2
        exit 1
    fi
    poll=$((poll + 1))
    if [ "$poll" -ge 600 ]; then
        kill "$addition_pid" 2>/dev/null || true
        wait "$addition_pid" || true
        echo 'FAIL: timed out waiting for authentic DM1 gameplay before adding CSB media' >&2
        cat "$addition_log" >&2
        exit 1
    fi
    sleep 0.05
done
mkdir -p "$addition_data_root/csb"
if ! ln "$csb_archive" \
    "$addition_data_root/csb/$(basename "$csb_archive")" 2>/dev/null; then
    ln -s "$csb_archive" \
        "$addition_data_root/csb/$(basename "$csb_archive")"
fi
if ! wait "$addition_pid"; then
    cat "$addition_log" >&2
    exit 1
fi
python3 - "$addition_log" "$addition_probe" "$addition_runtime_pattern" <<'PY'
import json
import re
import sys

log_path, probe_path, runtime_pattern = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

launch = re.search(
    r"DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3", trace)
runtime = re.search(runtime_pattern, trace)
complete = re.search(r"return-menu rescan-complete data=", trace)
if launch is None or runtime is None or complete is None or not (
        launch.start() < runtime.start() < complete.start()):
    raise SystemExit(
        "FAIL: authentic DM1 did not return to the launcher after the media addition")

for game in ("dm1", "csb", "dm2"):
    match = re.search(rf"return-menu rescan game={game} available=(\d+)", trace)
    if match is None or match.group(1) != "1":
        raise SystemExit(
            f"FAIL: return scan did not discover newly added original media for {game}")

if probe.get("launchedEver") != 1 or probe.get("active") != 0:
    raise SystemExit(f"FAIL: DM1 did not finish back at the launcher: {probe}")
print("PASS: returning from authentic DM1 discovers newly added authentic CSB media")
PY
rm -rf "$addition_scratch"
addition_scratch=
fi
