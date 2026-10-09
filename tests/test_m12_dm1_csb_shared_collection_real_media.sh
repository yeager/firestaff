#!/usr/bin/env sh
set -eu

app=${1:?usage: test_m12_dm1_csb_shared_collection_real_media.sh <firestaff> <data-root>}
source_root=${2:?usage: test_m12_dm1_csb_shared_collection_real_media.sh <firestaff> <data-root>}

dm1_towns=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$source_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
dm1_pc=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$source_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
csb_towns=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$source_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_amiga=${FIRESTAFF_CSB_AMIGA_ARCHIVE:-"$source_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"}
csb_atari=${FIRESTAFF_CSB_ATARI_ARCHIVE:-"$source_root/csb/chaos_strikes_back_ftl.zip"}

if [ ! -x "$app" ] || [ ! -f "$dm1_towns" ] || [ ! -f "$dm1_pc" ] ||
   [ ! -f "$csb_towns" ] || [ ! -f "$csb_amiga" ] ||
   [ ! -f "$csb_atari" ]; then
    echo 'SKIP: authentic DM1 and CSB multi-platform archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-shared-dm1-csb.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM
data_root="$scratch/data"
mkdir -p "$data_root/dm1" "$data_root/csb"
ln -s "$dm1_towns" "$data_root/dm1/$(basename "$dm1_towns")"
ln -s "$dm1_pc" "$data_root/dm1/$(basename "$dm1_pc")"
ln -s "$csb_towns" "$data_root/csb/$(basename "$csb_towns")"
ln -s "$csb_amiga" "$data_root/csb/$(basename "$csb_amiga")"
ln -s "$csb_atari" "$data_root/csb/$(basename "$csb_atari")"

run_game() {
    game=$1
    script=$2
    duration=$3
    run_dir="$scratch/$game"
    mkdir -p "$run_dir"
    FIRESTAFF_CONFIG_PATH="$run_dir/config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$run_dir/runtime.json" \
    HOME="$run_dir" XDG_CONFIG_HOME="$run_dir" APPDATA="$run_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --width 320 --height 200 --menu --game "$game" \
        --data-dir "$data_root" --debug --script "$script" \
        --duration "$duration" >"$run_dir/firestaff.log" 2>&1 || {
            cat "$run_dir/firestaff.log" >&2
            return 1
        }
}

run_game dm1 'enter,enter,enter' 15000
python3 - "$scratch/dm1/firestaff.log" "$scratch/dm1/runtime.json" \
    "$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip::DATA/GRAPHICS.DAT" <<'PY'
import json
import sys

log_path, probe_path, towns_source = sys.argv[1:]
trace = open(log_path, encoding="utf-8").read()
probe = json.load(open(probe_path, encoding="utf-8"))
if "dm1 platform=FM Towns edition=fmtowns-en matched" not in trace or \
        "dm1 platform=PC edition=pc34-en matched" not in trace or \
        f"launch phase=game-handoff mode=menu game=dm1 platform=FM Towns edition=fmtowns-en source={towns_source}" not in trace:
    raise SystemExit("FAIL: DM1 did not resolve its own FM Towns AUTO launch from the shared collection")
startup, party, towns = probe["startup"], probe["party"], probe["dm1FmtownsStartup"]
if (probe["sourceId"] != "dm1" or startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1 or startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        towns["program"] != "EDM.EXP" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: shared-collection DM1 did not reach authentic runtime: {probe}")
print("PASS: shared collection -> M12 DM1 AUTO -> authentic FM Towns runtime")
PY

run_game csb 'enter,enter,enter,wait700,click:52:110,wait10,click:250:50,wait240' 30000
python3 - "$scratch/csb/firestaff.log" "$scratch/csb/runtime.json" \
    "$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip" <<'PY'
import json
import re
import sys

log_path, probe_path, towns_archive = sys.argv[1:]
trace = open(log_path, encoding="utf-8").read()
probe = json.load(open(probe_path, encoding="utf-8"))
for platform, edition in (("FM Towns", "fmtowns-en"),
                          ("Amiga", "amiga31-multi"),
                          ("Atari ST", "st20-21-en")):
    if re.search(rf"csb platform={re.escape(platform)} edition={edition} matched source=", trace) is None:
        raise SystemExit(f"FAIL: shared collection did not authenticate CSB {platform} {edition}")
if re.search(r"CSB READY: gameId=csb dataDir=" + re.escape(towns_archive) +
             r" variant=csb-fmtowns-en route=startup handoff=f31-title-anm ", trace) is None:
    raise SystemExit("FAIL: CSB M12 AUTO did not select its FM Towns startup route")
startup, party = probe["startup"], probe["party"]
if (probe.get("sourceId") != "csb" or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1)):
    raise SystemExit(f"FAIL: shared-collection CSB did not reach authentic MINI.DAT runtime: {probe}")
print("PASS: shared collection -> M12 CSB AUTO -> authentic FM Towns MINI.DAT runtime")
PY
