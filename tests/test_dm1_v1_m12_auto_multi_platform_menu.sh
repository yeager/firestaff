#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_m12_auto_multi_platform_menu.sh <firestaff>}
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
towns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
pc_archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
dm1_data_dir=$(dirname "$towns_archive")
atari_archive=${FIRESTAFF_DM1_ATARI_ARCHIVE:-"$dm1_data_dir/Dungeon-Master_Atari-ST_EN.zip"}
amiga_archive=${FIRESTAFF_DM1_AMIGA_ARCHIVE:-"$dm1_data_dir/Dungeon-Master_Amiga_EN_Version-20.zip"}

if [[ ! -x "$app" || ! -f "$towns_archive" || ! -f "$pc_archive" ]]; then
    echo 'SKIP: authentic DM1 FM Towns and PC-3.4 archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-auto-menu.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM

# Stage authentic archives in the documented per-user install layout.  No
# --data-dir or FIRESTAFF_DATA override should mask AUTO's platform choice.
data_root="$scratch/home/.firestaff/data"
dm1_root="$data_root/dm1"
mkdir -p "$dm1_root" "$scratch/home"
# Keep the original archive names so catalogue identity and package checks
# see the same media users install; the test never extracts or modifies it.
ln -s "$towns_archive" "$dm1_root/$(basename "$towns_archive")"
ln -s "$pc_archive" "$dm1_root/$(basename "$pc_archive")"
extra_atari_media=""
extra_amiga_media=""
if [[ -f "$atari_archive" ]]; then
    ln -s "$atari_archive" "$dm1_root/$(basename "$atari_archive")"
    extra_atari_media=1
fi
if [[ -f "$amiga_archive" ]]; then
    ln -s "$amiga_archive" "$dm1_root/$(basename "$amiga_archive")"
    extra_amiga_media="dm1 platform=Amiga edition=amiga20-en matched"
fi

probe="$scratch/runtime.json"
log="$scratch/firestaff.log"
HOME="$scratch/home" \
XDG_CONFIG_HOME="$scratch/home" \
APPDATA="$scratch/home" \
FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 1920 --height 1080 --menu --game dm1 \
    --debug --verbose --script 'enter,enter,enter' \
    --duration 10000 >"$log" 2>&1 || {
        cat "$log" >&2
        exit 1
    }

python3 - "$log" "$probe" "$dm1_root" "$extra_atari_media" "$extra_amiga_media" <<'PY'
import json
import re
import sys

log_path, probe_path, dm1_root, *extra_media = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

towns_source = f"{dm1_root}/Dungeon-Master_FM-Towns_JA-EN.zip::DATA/GRAPHICS.DAT"
checks = (
    ("dm1 platform=FM Towns edition=fmtowns-en matched", "FM Towns media discovery"),
    ("dm1 platform=PC edition=pc34-en matched", "PC media discovery"),
    (f"launch phase=game-handoff mode=menu game=dm1 platform=FM Towns edition=fmtowns-en source={towns_source}",
     "FM Towns AUTO handoff"),
)
for needle, label in checks:
    if needle not in trace:
        raise SystemExit(f"FAIL: {label} missing from M12 trace\n{trace}")
if extra_media[0] and re.search(
        r"dm1 platform=Atari ST edition=(?:st10a-en|st10b-en|st11-en|st12-en) matched source=",
        trace) is None:
    raise SystemExit(f"FAIL: authentic Atari ST media was not discovered\n{trace}")
if extra_media[1] and "dm1 platform=Amiga edition=amiga20-en matched source=" not in trace:
    raise SystemExit(f"FAIL: authentic Amiga 2.0 media was not discovered\n{trace}")

startup = probe["startup"]
party = probe["party"]
fmtowns = probe["dm1FmtownsStartup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "c10c512f63461ebe79b5ac365115b61b" or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        fmtowns["program"] != "EDM.EXP" or
        fmtowns["programMd5"] != "c27e7b984df9753912c3375dc121919f" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: DM1 AUTO M12 did not reach authentic FM Towns runtime: {probe}")

print("PASS: DM1 M12 AUTO discovered the installed authentic platform media, selected FM Towns, and reached its first runtime frame")
PY

# Direct --game startup uses the same mixed library without the launcher
# choosing an edition first. Lock the requested FM Towns default to its own
# authenticated program so a platform-order regression cannot hide in CLI.
cli_probe="$scratch/cli-runtime.json"
cli_log="$scratch/cli-firestaff.log"
HOME="$scratch/home" \
XDG_CONFIG_HOME="$scratch/home" \
APPDATA="$scratch/home" \
FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$cli_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --game dm1 --debug --verbose \
    --boot-probe --boot-probe-frames 2 --duration 0 >"$cli_log" 2>&1 || {
        cat "$cli_log" >&2
        exit 1
    }

python3 - "$cli_log" "$cli_probe" "$extra_atari_media" "$extra_amiga_media" <<'PY'
import json
import re
import sys

log_path, probe_path, *extra_media = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

startup = probe["startup"]
if ("platform=FM Towns" not in trace or
        "edition=fmtowns-en" not in trace or
        probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "c10c512f63461ebe79b5ac365115b61b" or
        probe["dm1FmtownsStartup"]["program"] != "EDM.EXP" or
        probe["dm1FmtownsStartup"]["programMd5"] != "c27e7b984df9753912c3375dc121919f" or
        startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1):
    raise SystemExit(f"FAIL: bare DM1 CLI did not select FM Towns from the mixed library: {probe}\n{trace}")
if extra_media[0] and re.search(
        r"dm1 platform=Atari ST edition=(?:st10a-en|st10b-en|st11-en|st12-en) matched source=",
        trace) is None:
    raise SystemExit(f"FAIL: bare CLI did not discover authentic Atari ST media\n{trace}")
if extra_media[1] and "dm1 platform=Amiga edition=amiga20-en matched source=" not in trace:
    raise SystemExit(f"FAIL: bare CLI did not discover authentic Amiga 2.0 media\n{trace}")
print("PASS: bare DM1 CLI discovered the installed authentic platform media, selected FM Towns and reached runtime")
PY

# A mixed library must also honor an explicit platform selected in M12 and
# carry that exact original edition into the first runtime frame. The shared
# data root intentionally contains several editions; using --data-dir on an
# individual archive here would miss cross-edition source-selection errors.
run_mixed_menu_platform() {
    local platform=$1
    local card_click=$2
    local startup_inputs=$3
    local expected_handoff=$4
    local expected_asset_md5=$5
    local runtime_probe="$scratch/${platform}-runtime.json"
    local runtime_log="$scratch/${platform}-menu.log"
    local route="wait20,click:700:262,wait20,click:${card_click},wait20,click:450:405,wait20${startup_inputs}"

    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" \
        APPDATA="$scratch/home" FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
        "$app" --width 1920 --height 1080 --menu --game dm1 \
        --platform "$platform" --debug --verbose --script "$route" \
        --duration 15000 >"$runtime_log" 2>&1 || {
            cat "$runtime_log" >&2
            return 1
        }

    python3 - "$runtime_log" "$runtime_probe" "$dm1_root" \
        "$expected_handoff" "$expected_asset_md5" "$platform" <<'PY'
import json
import sys

log_path, probe_path, data_root, handoff, asset_md5s, platform = sys.argv[1:]
accepted_asset_md5s = set(asset_md5s.split(","))
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
expected_ready = (
    f"DM1 READY: gameId=dm1 dataDir={data_root}/"
)
if (expected_ready not in trace or f"handoff={handoff}" not in trace or
        probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] not in accepted_asset_md5s or
        startup["receiptReady"] != 1 or startup["active"] != 1 or
        startup["startupActive"] != 0 or startup["phase"] != "dm1-runtime" or
        startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(
        f"FAIL: mixed-library DM1 M12 {platform} selection did not reach its "
        f"source-owned first runtime frame: {probe}\n{trace}")
print(f"PASS: mixed-library DM1 M12 {platform} selected {handoff} and reached its authentic first runtime frame")
PY
}

if [[ -n "$extra_atari_media" ]]; then
    run_mixed_menu_platform atari-st 1458:405 ',wait30,enter,wait60,enter' \
        atari-st-dmcsb1 \
        b3cfd84e44cdf07ce2eeba47e87f772b,9ce2eaf7a9e78620e3f17594437caffa
fi
if [[ -n "$extra_amiga_media" ]]; then
    run_mixed_menu_platform amiga 410:679 '' \
        amiga-img2 6a2f135b53c2220f0251fa103e2a6e7e
fi
run_mixed_menu_platform pc 934:405 '' pc-img3 fa6b1aa29e191418713bf2cda93d962e
