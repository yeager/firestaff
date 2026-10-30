#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_csb_amiga_multi_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_csb_amiga_multi_runtime_real_media.sh <firestaff> <data-root>}
archive="$data_root/csb/Game,Chaos_Strikes_Back,Amiga,Software.7z"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: Firestaff or authentic CSB Amiga multilingual preservation archive is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-csb-amiga-multi.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 && -f "$scratch/menu.log" ]]; then
        grep -E 'startup game=|scan complete|CSB READY|game-handoff|FAIL:|error=' \
            "$scratch/menu.log" | tail -n 100 >&2 || true
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

menu_data="$scratch/data"
mkdir -p "$menu_data/csb"
ln -s "$archive" "$menu_data/csb/$(basename "$archive")"
archive_hash=$(firestaff_sha256_file "$archive")
home="$scratch/home"
probe="$scratch/runtime.json"
log="$scratch/menu.log"
mkdir -p "$home"

# Select CSB -> Amiga -> Custom, switch from the matched English A31 edition
# to its authenticated multilingual edition, then choose English in the
# original APPB language screen. The isolated root exposes only the original
# Amiga software archive.
navigation='wait-scan-complete,wait20,click:1190:262,wait20,click:934:405,wait20,click:1458:405,wait20,click:520:442,wait20,click:960:892,wait:1000,click:100:100,key:enter,up'
env -u FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS \
    -u FIRESTAFF_ORIGINALS_DIR -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_DATA="$menu_data" \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --width 1920 --height 1080 --data-dir "$menu_data" \
        --script "$navigation" --duration 45000 >"$log" 2>&1

expected_source="$menu_data/csb/$(basename "$archive")::Floppy Disks ADF/Chaos Strikes Back for Amiga Game Disk v3.1 (French, English, German) Cracked.adf::graphics.dat"
if ! grep -Fq 'startup game=none mode=menu' "$log" ||
   ! grep -Fq 'scan complete' "$log" ||
   ! grep -Fq 'games=dm1:no,csb:yes,dm2:no,nexus:no,theron:no' "$log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Amiga edition=amiga31-multi source=$expected_source" "$log" ||
   ! grep -Fq 'CSB READY: gameId=csb' "$log"; then
    echo 'FAIL: CSB Amiga multilingual no-game picker did not select its authentic edition' >&2
    grep -E 'startup game=|scan complete|CSB READY|game-handoff|failed|error' "$log" | tail -n 100 >&2 || true
    exit 1
fi

python3 - "$probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        probe.get("bootAssetMd5") != "21197b1d4994fd835c403d5a33dcac2b" or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0) or
        not probe.get("csbViewportHash") or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: CSB Amiga multilingual menu missed its authentic runtime: {probe}")
print("PASS: CSB Amiga multilingual no-game menu reached its English runtime frame")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB Amiga multilingual archive changed during menu startup' >&2
    exit 1
fi
echo 'PASS: CSB Amiga multilingual no-game startup used and preserved the original archive'
