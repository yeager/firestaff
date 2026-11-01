#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_csb_amiga35_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_csb_amiga35_runtime_real_media.sh <firestaff> <data-root>}
archive="$data_root/csb/Chaos Strikes Back for Amiga 3.5 CTRaw.7z"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: Firestaff or authentic CSB Amiga 3.5 English CTRaw archive is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-csb-amiga35.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 && -f "$scratch/menu.log" ]]; then
        grep -E 'startup game=|scan complete|CSB READY|game-handoff|amiga-swsh-ftl|swsh-intro|FAIL:|error=' "$scratch/menu.log" | tail -n 80 >&2 || true
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

# Expose only the authentic English 3.5 Amiga package. Start in the no-game
# picker, choose CSB -> Amiga -> Custom, use the edition tile and keyboard to
# select the authenticated English version, then launch through production M12.
env -u FIRESTAFF_DATA -u FIRESTAFF_ORIGINALS_DIR -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --verbose --width 1920 --height 1080 --data-dir "$menu_data" \
        --script 'wait-scan-complete,wait20,click:1190:262,wait20,click:934:405,wait20,click:1458:405,wait20,click:520:442,wait20,key:left,wait20,click:960:892,wait1200,enter' \
        --duration 45000 >"$log" 2>&1

expected_source="$menu_data/csb/$(basename "$archive")::ChaosStrikesBack_Disk1of2 GameDisk_English.raw::Graphics.DAT"
if ! grep -Fq 'startup game=none mode=menu' "$log" ||
   ! grep -Fq 'scan complete' "$log" ||
   ! grep -Fq 'games=dm1:no,csb:yes,dm2:no,nexus:no,theron:no' "$log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Amiga edition=amiga35-en source=$expected_source" "$log" ||
   ! grep -Eq 'startup-source-complete game=csb phase=amiga-swsh-ftl profile=ff3872baaed8ee4e83ee3c0684b2eeec frames=[1-9][0-9]* source-vblanks=30 audio=' "$log"; then
    echo 'FAIL: CSB Amiga 3.5 custom-edition picker missed its authenticated SWSH intro or runtime' >&2
    grep -E 'startup game=|scan complete|CSB READY|game-handoff|amiga-swsh-ftl|swsh-intro|failed|error' "$log" >&2 || true
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
        probe.get("bootAssetMd5") != "291e1bc6803e3dc4b974c60117ca5d68" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "inactive" or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0) or
        not probe.get("csbViewportHash") or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: CSB Amiga 3.5 menu did not reach its authentic first runtime: {probe}")
print("PASS: CSB Amiga 3.5 custom-edition menu reached its authentic first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB Amiga 3.5 archive changed during menu startup' >&2
    exit 1
fi
echo 'PASS: CSB Amiga 3.5 custom-edition startup used and preserved the original archive'
