#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_dm1_amiga20_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_dm1_amiga20_runtime_real_media.sh <firestaff> <data-root>}
archive="$data_root/dm1/Dungeon-Master_Amiga_EN_Version-20.zip"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: Firestaff or authentic DM1 Amiga 2.0 preservation archive is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-dm1-amiga20.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 && -f "$scratch/menu.log" ]]; then
        grep -E 'startup game=|scan complete|DM1 READY|game-handoff|startup-source|FAIL:|error=' \
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
mkdir -p "$menu_data/dm1"
ln -s "$archive" "$menu_data/dm1/$(basename "$archive")"
archive_hash=$(firestaff_sha256_file "$archive")
home="$scratch/home"
probe="$scratch/runtime.json"
log="$scratch/menu.log"
mkdir -p "$home"

# Select DM1 and Amiga through the no-game picker. The isolated data root has
# only the original nested ZIP -> ZIP -> ADF preservation package, so the
# picker must authenticate and launch Amiga 2.0 rather than a synthetic asset
# or another installed Amiga edition.
env -u FIRESTAFF_DATA -u FIRESTAFF_ORIGINALS_DIR -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --width 1920 --height 1080 --data-dir "$menu_data" \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait20' \
        --duration 15000 >"$log" 2>&1

expected_media="$menu_data/dm1/$(basename "$archive")::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf"
if ! grep -Fq 'startup game=none mode=menu' "$log" ||
   ! grep -Fq 'scan complete' "$log" ||
   ! grep -Fq 'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no' "$log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga20-en source=$expected_media" "$log" ||
   ! grep -Fq 'platform=amiga-v20 phase=title-f0437' "$log" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' "$log" ||
   ! grep -Fq 'phase=dm1-runtime' "$log"; then
    echo 'FAIL: DM1 Amiga 2.0 no-game picker did not complete authenticated title and Entrance startup' >&2
    grep -E 'startup game=|scan complete|DM1 READY|game-handoff|startup-source|phase=title-f0437|phase=entrance-f0441|phase=dm1-runtime|failed|error' \
        "$log" | tail -n 100 >&2 || true
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
        probe.get("sourceId") != "dm1" or
        probe.get("bootAssetMd5") != "6a2f135b53c2220f0251fa103e2a6e7e" or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "dm1-runtime" or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("dm1StartupHandoffExecuted") != 1 or
        startup.get("dm1StartupHoCFirstFrameReady") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 1, 3, 2, 0) or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: DM1 Amiga 2.0 no-game menu missed its authentic first runtime: {probe}")
print("PASS: DM1 Amiga 2.0 no-game menu reached its authenticated first runtime frame")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original DM1 Amiga 2.0 archive changed during menu startup' >&2
    exit 1
fi
echo 'PASS: DM1 Amiga 2.0 no-game startup used and preserved the original archive'
