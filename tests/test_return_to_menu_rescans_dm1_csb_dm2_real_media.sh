#!/usr/bin/env sh
set -eu
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh <firestaff> <data-root>}
test_mode=${3:-all}
dm1_archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
dm1_fmtowns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
dm1_atari_archive=${FIRESTAFF_DM1_ATARI_ARCHIVE:-"$data_root/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip"}
dm1_amiga36_archive=${FIRESTAFF_DM1_AMIGA36_ARCHIVE:-"$data_root/dm1/Dungeon-Master_Amiga_36.zip"}
csb_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_atari_archive=${FIRESTAFF_CSB_ATARI_ARCHIVE:-"$data_root/csb/chaos_strikes_back_ftl.zip"}
if [ -z "${FIRESTAFF_CSB_ATARI_ARCHIVE:-}" ] &&
   [ ! -f "$csb_atari_archive" ]; then
    # Some installed collections retain the authentic Atari archive under a
    # descriptive filename rather than the historical test-corpus alias.
    candidate="$data_root/csb/Chaos Strikes Back.stx"
    if [ -f "$candidate" ]; then
        csb_atari_archive=$candidate
    fi
fi
csb_amiga_archive=${FIRESTAFF_CSB_AMIGA_ARCHIVE:-"$data_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"}
dm2_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
dm2_dos_archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}
dm2_mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}
dm2_amiga_archive=${FIRESTAFF_DM2_AMIGA_ARCHIVE:-"$data_root/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"}
if [ -z "${FIRESTAFF_DM2_MAC_ARCHIVE:-}" ] && [ ! -f "$dm2_mac_archive" ]; then
    # Use the user's extracted authentic Macintosh retail files when the
    # optional ZIP alias is absent. The game scanner accepts DMFiles roots.
    candidate="$data_root/dm2/mac-en/Dungeon Master II/DMFiles"
    if [ -f "$candidate/Graphics.dat" ] &&
       [ -f "$candidate/Dungeon.dat" ] &&
       [ -f "$candidate/Title.MooV" ]; then
        dm2_mac_archive=$candidate
    fi
fi

if [ ! -x "$app" ] ||
   { [ "$test_mode" != dm2-amiga ] && [ "$test_mode" != dm1-sourcefile ] &&
     [ "$test_mode" != csb-sourcefile ] && [ "$test_mode" != dm2-sourcefile ] &&
     [ "$test_mode" != dm2-sourcefile-removed ] &&
     [ "$test_mode" != dm1-sourcefile-removed ] &&
     [ "$test_mode" != csb-atari-only ] &&
     [ "$test_mode" != dm1-atari ] && [ "$test_mode" != dm1-amiga36 ] &&
     [ "$test_mode" != direct-exit ] &&
     [ "$test_mode" != mouse-return ] &&
     [ "$test_mode" != return-window-close ] &&
     { [ ! -f "$dm1_archive" ] || [ ! -f "$dm1_fmtowns_archive" ] ||
       [ ! -f "$csb_archive" ] || [ ! -f "$dm2_archive" ]; }; } ||
   { [ "$test_mode" = direct-exit ] &&
     { [ ! -f "$dm1_fmtowns_archive" ] || [ ! -f "$csb_archive" ] ||
       [ ! -f "$dm2_archive" ]; }; } ||
   { [ "$test_mode" = mouse-return ] &&
     { [ ! -f "$dm1_archive" ] || [ ! -f "$csb_archive" ] ||
       [ ! -f "$dm2_archive" ]; }; } ||
   { [ "$test_mode" = return-window-close ] &&
     [ ! -f "$dm1_archive" ]; } ||
   { { [ "$test_mode" = dm1-sourcefile ] || [ "$test_mode" = csb-sourcefile ] ||
       [ "$test_mode" = dm2-sourcefile ] || [ "$test_mode" = dm2-sourcefile-removed ] ||
       [ "$test_mode" = dm1-sourcefile-removed ]; } &&
     { [ ! -f "$dm1_archive" ] || [ ! -f "$csb_archive" ] ||
       [ ! -f "$dm2_archive" ]; }; } ||
   { { [ "$test_mode" = dm2-sourcefile ] || [ "$test_mode" = dm2-sourcefile-removed ]; } &&
     [ ! -f "$dm2_dos_archive" ]; } ||
   { [ "$test_mode" = dm2-amiga ] && [ ! -f "$dm2_amiga_archive" ]; } ||
   { [ "$test_mode" = dm1-atari ] && [ ! -f "$dm1_atari_archive" ]; } ||
   { [ "$test_mode" = csb-atari-only ] && [ ! -f "$csb_atari_archive" ]; } ||
   { [ "$test_mode" = dm1-amiga36 ] && [ ! -f "$dm1_amiga36_archive" ]; }; then
    echo 'SKIP: authentic DM1, CSB, and DM2 archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/.codex-scratch"}}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/return-rescan.XXXXXX")
mutation_scratch=
addition_scratch=
removal_pid=
cleanup() {
    if [ -n "$removal_pid" ]; then
        kill "$removal_pid" 2>/dev/null || true
        wait "$removal_pid" 2>/dev/null || true
    fi
    if [ "${FIRESTAFF_KEEP_TEST_SCRATCH:-0}" = 1 ]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
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
    case_label=${10:-$game-$platform}
    case_root="$scratch/$case_label"
    runtime_probe="$case_root/runtime.json"
    run_log="$case_root/firestaff.log"
    entrance_input=
    mkdir -p "$case_root"
    case_data_root=${7:-$data_root}
    case_width=${8:-320}
    case_height=${9:-200}

    # Keep a clean per-game config so a previous media selection cannot make
    # an explicit platform appear to work by reusing a cached source.
    set -- --width "$case_width" --height "$case_height" --menu --game "$game"
    if [ "$platform" != auto ]; then
        set -- "$@" --platform "$platform"
    fi
    if [ "$game" = dm2 ] && [ "$platform" = amiga ]; then
        set -- "$@" --scale-mode 0
    fi
    if [ "$game" = dm1 ] && [ "$platform" = amiga ]; then
        entrance_input=key:return
    fi
    set -- "$@" --data-dir "$case_data_root" --debug \
        --script "$script" --duration "$duration_ms"
    removal_report="$case_root/source-removed.offset"
    if [ "$case_label" = dm2-sourcefile-removed ] ||
       [ "$case_label" = dm1-sourcefile-removed ]; then
        # Remove only the task-owned staged selection after real gameplay is
        # observed; the original media and its other staged link remain intact.
        python3 - "$run_log" "$case_data_root" "$scratch" "$removal_report" <<'PYWATCH' &
import pathlib
import re
import sys
import time

log, selected, scratch, report = map(pathlib.Path, sys.argv[1:])
selected.relative_to(scratch)
game_id = "dm1" if "dm1-sourcefile-removed" in str(selected) else "dm2"
if game_id == "dm2" and selected.name != "Selected-PC.ZIP":
    raise SystemExit("FAIL: DM2 removal target is not the staged selection")
if game_id == "dm1" and selected.name != "Dungeon-Master_DOS_EN_Version-34.zip":
    raise SystemExit("FAIL: DM1 removal target is not the staged selection")
deadline = time.monotonic() + 50
while time.monotonic() < deadline:
    trace = log.read_text(encoding="utf-8") if log.exists() else ""
    runtime_pattern = (r"startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1"
                       if game_id == "dm1" else
                       r"startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1")
    if re.search(runtime_pattern, trace):
        selected.unlink()
        report.write_text(str(len(trace)), encoding="ascii")
        break
    time.sleep(0.02)
else:
    raise SystemExit("FAIL: no original runtime before selected-file removal")
PYWATCH
        removal_pid=$!
    fi
    FIRESTAFF_CONFIG_PATH="$case_root/config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    HOME="$case_root" XDG_CONFIG_HOME="$case_root" APPDATA="$case_root" \
    FIRESTAFF_AUTOTEST_ENTRANCE_INPUT="$entrance_input" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "$@" >"$run_log" 2>&1 || {
            cat "$run_log" >&2
            return 1
        }

    if [ -n "$removal_pid" ]; then
        wait "$removal_pid" || return 1
        removal_pid=
    fi

    python3 - "$game" "$run_log" "$runtime_probe" "$runtime_pattern" \
        "$launch_pattern" "$case_label" "$case_data_root" "$removal_report" <<'PY'
import json
import os
import re
import sys

game_id, log_path, probe_path, runtime_pattern, launch_pattern, case_label, data_path, removal_report = sys.argv[1:]
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

scan_root = data_path
if not os.path.isdir(scan_root):
    scan_root = os.path.dirname(os.path.dirname(scan_root))

expected_available = {
    "dm1": 0 if case_label in ("csb-atari-only", "dm1-sourcefile-removed") else 1,
    "csb": 1,
    "dm2": 0 if case_label == "csb-atari-only" else 1,
    "nexus": int(
        os.path.isfile(os.path.join(scan_root, "nexus", "DM.BIN")) and
        os.path.isfile(os.path.join(scan_root, "nexus", "MENU.BPK"))),
    "theron": int(any(
        os.path.isfile(os.path.join(scan_root, "theron", name))
        for name in ("TQUS02.bin", "TQJP02.bin"))),
}

for scanned_game, expected in expected_available.items():
    match = re.search(
        rf"return-menu rescan game={scanned_game} available=(\d+)", trace)
    if match is None or int(match.group(1)) != expected:
        raise SystemExit(
            f"FAIL: {game_id} return-to-menu rescan reported "
            f"{scanned_game} available="
            f"{match.group(1) if match else 'missing'}, expected {expected} "
            f"from original media in {scan_root}")

if (probe.get("launchedEver") != 1 or probe.get("active") != 0 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(
        f"FAIL: {game_id} did not finish back at the launcher: {probe}")

if (case_label.endswith("-sourcefile") or case_label.startswith("dm1-sourcefile") or
        case_label.startswith("dm2-sourcefile")):
    expected_root = os.path.dirname(os.path.dirname(data_path))
    root = re.search(r"return-menu rescan-complete data=([^\n]+)", trace)
    if root is None or os.path.normpath(root.group(1)) != os.path.normpath(expected_root):
        raise SystemExit(f"FAIL: selected archive did not rescan its lexical collection: {root}")

if case_label in ("dm1-sourcefile-removed", "dm2-sourcefile-removed"):
    with open(removal_report, encoding="ascii") as stream:
        removal_offset = int(stream.read())
    if os.path.lexists(data_path) or removal_offset >= complete.start():
        raise SystemExit("FAIL: selected archive was not removed before the return scan")

inactive_rates = re.findall(
    r"perf game=\S+ active=0[^\n]* game-ticks-per-sec=(\S+)", trace)
if any(float(rate) != 0.0 for rate in inactive_rates):
    raise SystemExit(f"FAIL: inactive launcher reports gameplay ticks: {inactive_rates}")
if (case_label.endswith("-sourcefile") or case_label.startswith("dm1-sourcefile") or
        case_label.startswith("dm2-sourcefile")) and not inactive_rates:
    raise SystemExit("FAIL: selected-archive return omitted inactive performance windows")

print(f"PASS: authentic {game_id} gameplay returned to the launcher and "
      "reported availability for all five games")
PY
}

run_direct_game_exit_case() {
    game=$1
    case_root="$scratch/direct-game-exit-$game"
    runtime_probe="$case_root/runtime.json"
    run_log="$case_root/firestaff.log"
    case "$game" in
        dm1)
            archive=$dm1_fmtowns_archive
            script='wait-game-runtime,key:escape,key:enter'
            ;;
        csb)
            archive=$csb_archive
            script='wait:700,click:52:110,wait:10,click:250:50,wait:240,wait-game-runtime,back,enter'
            ;;
        dm2)
            archive=$dm2_archive
            script='wait:1800,click:115:65,click:100:60,wait-game-runtime,key:escape,key:enter'
            ;;
        nexus)
            archive=
            for candidate in "$data_root"/nexus/*.cue "$data_root"/nexus/*.iso "$data_root"/nexus/DM.BIN; do
                if [ -f "$candidate" ]; then
                    archive=$candidate
                    break
                fi
            done
            script='wait:100,key:escape'
            ;;
        theron)
            archive=
            for candidate in "$data_root"/theron/TQUS02.bin "$data_root"/theron/TQJP02.bin; do
                if [ -f "$candidate" ]; then
                    archive=$candidate
                    break
                fi
            done
            script='wait:100,key:escape'
            ;;
        *) echo "FAIL: unsupported direct-exit game: $game" >&2; return 1 ;;
    esac
    if [ -z "$archive" ] || [ ! -f "$archive" ]; then
        echo "SKIP: authentic $game original media unavailable for direct-exit test"
        return 0
    fi
    mkdir -p "$case_root"
    FIRESTAFF_CONFIG_PATH="$case_root/config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    HOME="$case_root" XDG_CONFIG_HOME="$case_root" APPDATA="$case_root" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --game "$game" \
            --data-dir "$data_root" --debug \
            --script "$script" \
            --duration 90000 >"$run_log" 2>&1 || {
                cat "$run_log" >&2
                return 1
            }

python3 - "$game" "$run_log" "$runtime_probe" <<'PY'
import json
import sys

game_id, log_path, probe_path = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

exit_marker = (
    "direct game ended; exiting instead of opening the start menu")
if (f"launch phase=game-handoff mode=direct game={game_id} " not in trace or
        f"startup-frame game={game_id} " not in trace or
        exit_marker not in trace or
        "return-menu rescan-complete data=" in trace or
        probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(
        f"FAIL: direct --game {game_id} did not exit after Return to Menu: "
        f"probe={probe}")

print(f"PASS: direct --game {game_id} exits after Return to Menu "
      "without reopening the launcher")
PY
}

run_direct_game_exit_matrix() {
    run_direct_game_exit_case dm1
    run_direct_game_exit_case csb
    run_direct_game_exit_case dm2
    run_direct_game_exit_case nexus
    run_direct_game_exit_case theron
}

run_return_menu_window_close_case() {
    return_method=$1
    case_root="$scratch/return-menu-window-close-$return_method"
    runtime_probe="$case_root/runtime.json"
    run_log="$case_root/firestaff.log"
    runtime_pattern='startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2'
    case "$return_method" in
        keyboard)
            return_script='key:enter,key:enter,key:enter,wait:12,key:escape,key:enter'
            ;;
        mouse)
            return_script='key:enter,key:enter,key:enter,wait-game-runtime,key:escape,click:110:109'
            ;;
        *) echo "FAIL: unsupported return method: $return_method" >&2; return 1 ;;
    esac
    mkdir -p "$case_root"
    FIRESTAFF_CONFIG_PATH="$case_root/config.toml" \
    FIRESTAFF_AUTOTEST=1 \
    FIRESTAFF_AUTOTEST_QUIT_DURING_RESCAN=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    HOME="$case_root" XDG_CONFIG_HOME="$case_root" APPDATA="$case_root" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --menu --game dm1 --platform pc \
            --data-dir "$data_root" --debug \
            --script "$return_script" \
            --duration 60000 >"$run_log" 2>&1 || {
                cat "$run_log" >&2
                return 1
            }
    python3 - "$return_method" "$run_log" "$runtime_probe" \
        "$runtime_pattern" <<'PY'
import json
import re
import sys

return_method, log_path, probe_path, runtime_pattern = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

launch = re.search(
    r"DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3", trace)
runtime = re.search(runtime_pattern, trace)
injected = "autotest injected window-close during return-menu rescan" in trace
cancelled = "return-menu rescan-cancelled by window close" in trace
if (launch is None or runtime is None or not launch.start() < runtime.start() or
        not injected or not cancelled or
        "return-menu rescan-complete data=" in trace or
        probe.get("launchedEver") != 1 or probe.get("active") != 0):
    raise SystemExit(
        f"FAIL: authentic DM1 {return_method} return-menu scan did not stop "
        f"on window close: "
        f"probe={probe}, injected={injected}, cancelled={cancelled}")
print(f"PASS: authentic DM1 {return_method} return closes during the "
      "originals rescan")
PY
}

run_return_menu_window_close_matrix() {
    run_return_menu_window_close_case keyboard
    run_return_menu_window_close_case mouse
}

stage_csb_return_data_root() {
    stage_root=$1
    mkdir -p "$stage_root/dm1" "$stage_root/csb" "$stage_root/dm2"
    ln -s "$dm1_archive" "$stage_root/dm1/$(basename "$dm1_archive")"
    ln -s "$csb_archive" "$stage_root/csb/$(basename "$csb_archive")"
    ln -s "$csb_atari_archive" "$stage_root/csb/$(basename "$csb_atari_archive")"
    ln -s "$csb_amiga_archive" "$stage_root/csb/$(basename "$csb_amiga_archive")"
    ln -s "$dm2_archive" "$stage_root/dm2/$(basename "$dm2_archive")"
}

stage_csb_atari_only_data_root() {
    stage_root=$1
    mkdir -p "$stage_root/csb"
    ln -s "$csb_atari_archive" \
        "$stage_root/csb/$(basename "$csb_atari_archive")"
}

stage_dm2_amiga_return_data_root() {
    stage_root=$1
    mkdir -p "$stage_root/dm1" "$stage_root/csb" "$stage_root/dm2"
    ln -s "$dm1_archive" "$stage_root/dm1/$(basename "$dm1_archive")"
    ln -s "$dm1_fmtowns_archive" \
        "$stage_root/dm1/$(basename "$dm1_fmtowns_archive")"
    ln -s "$csb_archive" "$stage_root/csb/$(basename "$csb_archive")"
    ln -s "$dm2_amiga_archive" \
        "$stage_root/dm2/$(basename "$dm2_amiga_archive")"
}

stage_dm1_return_data_root() {
    stage_root=$1
    dm1_source_archive=$2
    mkdir -p "$stage_root/dm1" "$stage_root/csb" "$stage_root/dm2"
    ln -s "$dm1_source_archive" \
        "$stage_root/dm1/$(basename "$dm1_source_archive")"
    ln -s "$csb_archive" "$stage_root/csb/$(basename "$csb_archive")"
    ln -s "$dm2_archive" "$stage_root/dm2/$(basename "$dm2_archive")"
}

# File selection remains exact at launch; returning to the launcher must
# inventory the surrounding data/<game> collection, not only that file.
for sourcefile_case in dm1-sourcefile csb-sourcefile dm2-sourcefile dm2-sourcefile-removed dm1-sourcefile-removed; do
if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
     [ "$test_mode" = "$sourcefile_case" ]; }; then
    sourcefile_root="$scratch/$sourcefile_case/data"
    mkdir -p "$sourcefile_root/dm1" "$sourcefile_root/csb" "$sourcefile_root/dm2"
    case "$sourcefile_case" in
        dm1-sourcefile|dm1-sourcefile-removed)
            sourcefile_game=dm1
            sourcefile_platform=pc
            sourcefile_archive=$dm1_archive
            sourcefile_path="$sourcefile_root/dm1/$(basename "$dm1_archive")"
            sourcefile_script='key:enter,key:enter,key:enter,wait-game-runtime'
            sourcefile_duration=60000
            sourcefile_runtime='startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2'
            sourcefile_launch='launch phase=game-handoff mode=menu game=dm1 platform=PC edition=pc34-en source=.*/Dungeon-Master_DOS_EN_Version-34[.]zip::DATA/GRAPHICS[.]DAT'
            ;;
        csb-sourcefile)
            sourcefile_game=csb
            sourcefile_platform=fm-towns
            sourcefile_archive=$csb_archive
            sourcefile_path="$sourcefile_root/csb/$(basename "$csb_archive")"
            sourcefile_script='key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:240,wait-game-runtime,back,wait:10'
            sourcefile_duration=90000
            sourcefile_runtime='startup-frame game=csb .*phase=inactive .*level-loaded=1 map=4 party=22,18 dir=2 champions=1'
            sourcefile_launch='launch phase=game-handoff mode=menu game=csb platform=FM Towns edition=fmtowns-en source=.*/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN[.]zip::CDATA/GRAPHICS[.]DAT'
            ;;
        *)
            sourcefile_game=dm2
            sourcefile_platform=pc
            sourcefile_archive=$dm2_dos_archive
            sourcefile_path="$sourcefile_root/dm2/Selected-PC.ZIP"
            sourcefile_script='key:enter,key:enter,key:enter,wait:1800,key:enter,key:enter,wait-game-runtime'
            sourcefile_duration=55000
            sourcefile_runtime='startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1'
            sourcefile_launch='launch phase=game-handoff mode=menu game=dm2 platform=PC edition=pc-en source=.*Selected-PC[.]ZIP::data/graphics[.]dat'
            ;;
    esac

    if [ "$sourcefile_game" != dm1 ]; then
        ln -s "$dm1_archive" "$sourcefile_root/dm1/$(basename "$dm1_archive")"
    fi
    if [ "$sourcefile_game" != csb ]; then
        ln -s "$csb_archive" "$sourcefile_root/csb/$(basename "$csb_archive")"
    fi
    if [ "$sourcefile_game" != dm2 ]; then
        ln -s "$dm2_archive" "$sourcefile_root/dm2/$(basename "$dm2_archive")"
    fi
    ln -s "$sourcefile_archive" "$sourcefile_path"

    if [ "$sourcefile_case" = dm2-sourcefile-removed ]; then
        ln -s "$dm2_dos_archive" "$sourcefile_root/dm2/$(basename "$dm2_dos_archive")"
        sourcefile_script="$sourcefile_script,wait:90"
    fi
    if [ "$sourcefile_case" = dm1-sourcefile-removed ]; then
        sourcefile_script="$sourcefile_script,wait:90"
    fi
    if [ "$sourcefile_game" = csb ]; then
        sourcefile_script="$sourcefile_script,enter"
    else
        sourcefile_script="$sourcefile_script,key:escape,key:enter"
    fi
    run_return_to_menu_case "$sourcefile_game" "$sourcefile_platform" \
        "$sourcefile_script" "$sourcefile_duration" "$sourcefile_runtime" \
        "$sourcefile_launch" "$sourcefile_path" 320 200 "$sourcefile_case"
fi
done

# The key sequence is the same ordinary M12 game/platform/original choice
# used by the DM1 PC-34 and CSB FM Towns startup checks. Their source startup
# phases differ, so each case proves its own authentic runtime receipt before
# the return-confirmation interaction can pass. Each game also covers the
# shared mouse-driven YES branch through an authentic game session.
if [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
   [ "$test_mode" = direct-exit ] || [ "$test_mode" = mouse-return ] ||
   [ "$test_mode" = return-window-close ] ||
   [ "$test_mode" = csb-atari ] || [ "$test_mode" = csb-amiga ] ||
   [ "$test_mode" = dm2-dos ] || [ "$test_mode" = dm2-mac ] ||
   [ "$test_mode" = dm2-mouse-return ] ||
   [ "$test_mode" = dm1-atari ] || [ "$test_mode" = dm1-amiga36 ] ||
   [ "$test_mode" = dm2-amiga ]; then
    if [ "$test_mode" = return-window-close ]; then
        run_return_menu_window_close_matrix
    fi
    if [ "$test_mode" = all ] || [ "$test_mode" = returns ]; then
        run_return_menu_window_close_matrix
    fi
    if [ "$test_mode" = direct-exit ]; then
        run_direct_game_exit_matrix
    fi
    if [ "$test_mode" = all ] || [ "$test_mode" = returns ]; then
        run_direct_game_exit_matrix
    run_return_to_menu_case dm1 pc \
        'key:enter,key:enter,key:enter,wait:12,key:escape,key:enter' \
        15000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
        'DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3'
    run_return_to_menu_case dm1 fmtowns \
        'key:enter,key:enter,key:enter,wait:5,key:escape,wait:10,key:enter' \
        30000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
        'launch phase=game-handoff mode=menu game=dm1 platform=FM Towns edition=fmtowns-en source=.*/data/dm1/.*GRAPHICS.DAT'
    # Scanning the complete real-media tree before the CSB FM Towns title
    # handoff can exceed the original 30s bound on software-rendered hosts.
    run_return_to_menu_case csb fm-towns \
        'key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:240,back,wait:10,enter' \
        60000 'startup-frame game=csb .*phase=inactive .*level-loaded=1 map=4 party=22,18 dir=2 champions=1' \
        'CSB READY: gameId=csb dataDir=.*/data/csb/.*variant=csb-fmtowns-en .*handoff=f31-title-anm'
    fi
    if [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
       [ "$test_mode" = mouse-return ]; then
        run_return_to_menu_case dm1 pc \
            'key:enter,key:enter,key:enter,wait-game-runtime,key:escape,click:110:109' \
            30000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1 map=0 party=1,3 dir=2' \
            'DM1 READY: gameId=dm1 dataDir=.*/data/dm1/.*handoff=pc-img3' \
            "$data_root" 320 200 dm1-mouse-return
        run_return_to_menu_case csb fm-towns \
            'key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:240,wait-game-runtime,back,wait:10,click:110:109' \
            60000 'startup-frame game=csb .*phase=inactive .*level-loaded=1 map=4 party=22,18 dir=2 champions=1' \
            'CSB READY: gameId=csb dataDir=.*/data/csb/.*variant=csb-fmtowns-en .*handoff=f31-title-anm' \
            "$data_root" 320 200 csb-mouse-return
    fi
    if { [ "$test_mode" = dm1-atari ] ||
         [ "$test_mode" = all ] || [ "$test_mode" = returns ]; } &&
       [ -f "$dm1_atari_archive" ]; then
        dm1_atari_data_root="$scratch/dm1-atari-data"
        stage_dm1_return_data_root "$dm1_atari_data_root" "$dm1_atari_archive"
        dm1_atari_route='enter,enter,enter,wait30,enter,wait60,enter'
        for token in up up up up turn-left up up up turn-left \
            up up up up up turn-right up up turn-right up turn-left \
            up up turn-right up turn-left up up turn-left; do
            dm1_atari_route="$dm1_atari_route,wait30,$token"
        done
        dm1_atari_route="$dm1_atari_route,wait30,click:384:236,wait10,click:420:300,wait30,key:kp6,wait60,wait-game-runtime,key:escape,enter"
        run_return_to_menu_case dm1 atari-st "$dm1_atari_route" \
            120000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1' \
            'DM1 READY: gameId=dm1 dataDir=.*/dm1/.*handoff=atari-st-dmcsb1' \
            "$dm1_atari_data_root" 960 540 dm1-atari-st
    elif [ "$test_mode" = dm1-atari ]; then
        echo "SKIP: DM1 Atari ST archive unavailable: $dm1_atari_archive"
    fi
    if { [ "$test_mode" = dm1-amiga36 ] ||
         [ "$test_mode" = all ] || [ "$test_mode" = returns ]; } &&
       [ -f "$dm1_amiga36_archive" ]; then
        dm1_amiga36_data_root="$scratch/dm1-amiga36-data"
        stage_dm1_return_data_root "$dm1_amiga36_data_root" "$dm1_amiga36_archive"
        run_return_to_menu_case dm1 amiga \
            'enter,enter,enter,wait-game-runtime,key:escape,enter' \
            120000 'startup-frame game=dm1 .*phase=dm1-runtime .*level-loaded=1' \
            'startup-f0441-authenticated-handoff game=dm1 platform=amiga-v36 graphics=7f9458e4a3972d06e649a6fa85a7f34b' \
            "$dm1_amiga36_data_root" 320 200 dm1-amiga36
    elif [ "$test_mode" = dm1-amiga36 ]; then
        echo "SKIP: DM1 Amiga 3.6 archive unavailable: $dm1_amiga36_archive"
    fi
    if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = csb-atari ]; } &&
       [ -f "$csb_atari_archive" ] && [ -f "$csb_amiga_archive" ]; then
        atari_data_root="$scratch/csb-atari-data"
        stage_csb_return_data_root "$atari_data_root"
        run_return_to_menu_case csb auto \
            'wait20,click:586:131,wait20,click:729:202,wait20,click:225:202,wait:3600,click:813:156,wait-game-runtime,back,enter' \
            150000 \
            'startup-frame game=csb .*phase=(csb-entrance-4|inactive) .*level-loaded=1 map=0 party=9,0 dir=2 champions=0' \
            'CSB READY: gameId=csb dataDir=.*variant=csb-st20-21-en .*handoff=atari-st-animate-ftlcode' \
            "$atari_data_root" 960 600 csb-atari
    elif [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = csb-atari ]; then
        echo "SKIP: authentic CSB Atari ST and Amiga archives are required for the mixed-media return case"
    fi
    if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = csb-amiga ]; } &&
       [ -f "$csb_amiga_archive" ] && [ -f "$csb_atari_archive" ]; then
        amiga_data_root="$scratch/csb-amiga-data"
        stage_csb_return_data_root "$amiga_data_root"
        run_return_to_menu_case csb auto \
            'enter,right,enter,enter,wait:1000,click:100:100,key:enter,wait:300,back,wait:10,enter' \
            45000 \
            'startup-frame game=csb .*phase=inactive .*level-loaded=1 map=0 party=9,0 dir=2 champions=0' \
            'CSB READY: gameId=csb dataDir=.*variant=(?:csb-amiga-a31m.*handoff=a31m-titl-dat|csb-amiga-a31e.*handoff=a31e-appb-bjeload-c03)' \
            "$amiga_data_root" 320 200 csb-amiga
    else
        if [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
           [ "$test_mode" = csb-amiga ]; then
            echo "SKIP: authentic CSB Amiga archive unavailable: $csb_amiga_archive"
        fi
    fi
    if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-dos ]; } && [ -f "$dm2_dos_archive" ]; then
    run_return_to_menu_case dm2 pc \
        'key:enter,key:enter,key:enter,wait:1800,key:enter,key:enter,wait-game-runtime,key:escape,key:enter' \
        120000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
        'launch phase=game-handoff mode=menu game=dm2 platform=PC edition=pc-en source=.*/data/dm2/' \
        "$data_root" 320 200 dm2-dos
    elif [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-dos ]; then
        echo "SKIP: authentic DM2 DOS archive unavailable: $dm2_dos_archive"
    fi
    if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-mac ]; } &&
       { [ -f "$dm2_mac_archive" ] || [ -d "$dm2_mac_archive" ]; }; then
    run_return_to_menu_case dm2 mac \
        'key:enter,key:enter,key:enter,wait:1200,key:enter,wait:30,click:115:65,wait:30,click:112:130,wait:30,key:right,wait:30,key:s,wait:30,key:up,wait:20,key:command+q,key:enter' \
        45000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
        'launch phase=game-handoff mode=menu game=dm2 platform=Macintosh edition=mac-en-retail source=.*/data/dm2/' \
        "$data_root" 320 200 dm2-mac
    elif [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-mac ]; then
        echo "SKIP: authentic DM2 Macintosh archive unavailable: $dm2_mac_archive"
    fi
    if { [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-amiga ]; } && [ -f "$dm2_amiga_archive" ]; then
    dm2_amiga_data_root="$scratch/dm2-amiga-data"
    stage_dm2_amiga_return_data_root "$dm2_amiga_data_root"
    run_return_to_menu_case dm2 amiga \
        'key:enter,key:enter,key:enter,wait:2000,click:435:265,wait-game-runtime,back,enter' \
        150000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
        'launch phase=game-handoff mode=menu game=dm2 platform=Amiga edition=amiga-en source=.*/dm2/' \
        "$dm2_amiga_data_root" 960 600 dm2-amiga
    elif [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
         [ "$test_mode" = dm2-amiga ]; then
        echo "SKIP: authentic DM2 Amiga archive unavailable: $dm2_amiga_archive"
    fi
    if [ "$test_mode" = all ] || [ "$test_mode" = returns ] ||
       [ "$test_mode" = dm2-mouse-return ] || [ "$test_mode" = mouse-return ]; then
    run_return_to_menu_case dm2 auto \
        'key:enter,key:enter,key:enter,wait:1800,click:115:65,click:100:60,wait:10,key:escape,click:110:109' \
        60000 'startup-frame game=dm2 .*phase=dm2-runtime .*level-loaded=1' \
        'launch phase=game-handoff mode=menu game=dm2 platform=FM Towns edition=fmtowns-ja source=.*/data/dm2/' \
        "$data_root" 320 200 dm2-mouse-return
    fi
fi

if [ "$test_mode" = csb-atari-only ]; then
    atari_only_root="$scratch/csb-atari-only-data"
    stage_csb_atari_only_data_root "$atari_only_root"
    archive_hash_before=$(firestaff_sha256_file "$csb_atari_archive")
    run_return_to_menu_case csb auto \
        'wait20,click:586:131,wait20,click:729:202,wait20,click:225:202,wait:3600,click:813:156,wait-game-runtime,back,enter' \
        150000 \
        'startup-frame game=csb .*phase=(csb-entrance-4|inactive) .*level-loaded=1 map=0 party=9,0 dir=2 champions=0' \
        'CSB READY: gameId=csb dataDir=.*variant=csb-st20-21-en .*handoff=atari-st-animate-ftlcode' \
        "$atari_only_root" 960 600 csb-atari-only
    archive_hash_after=$(firestaff_sha256_file "$csb_atari_archive")
    if [ "$archive_hash_before" != "$archive_hash_after" ]; then
        echo 'FAIL: original CSB Atari archive changed during return-to-menu rescan' >&2
        exit 1
    fi
    echo 'PASS: authentic CSB Atari-only launch returned to the menu, rescanned availability and preserved the source archive'
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
