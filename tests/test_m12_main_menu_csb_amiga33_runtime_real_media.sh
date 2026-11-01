#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_csb_amiga33_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_csb_amiga33_runtime_real_media.sh <firestaff> <data-root>}
archive="$data_root/csb/Chaos Strikes Back for Amiga version 3.3 English, French, German.zip"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: Firestaff or authentic CSB Amiga 3.3 archive is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-csb-amiga33.XXXXXX")
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

# The only installed game media is the original A33 Amiga package. Let the
# production no-game picker discover it, identify the shared 3.1/3.3
# multilingual family, and complete the native language/startup handoff.
navigation='wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240,click:100:100,wait10,key:enter,up'
env -u FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS \
    -u FIRESTAFF_ORIGINALS_DIR -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_DATA="$menu_data" \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --width 320 --height 200 --data-dir "$menu_data" \
        --script "$navigation" --duration 45000 >"$log" 2>&1

expected_prefix="$menu_data/csb/$(basename "$archive")::ChaosStrikesBackAmiga33EnglishFrenchGerman.adf::"
if ! grep -Fq 'startup game=none mode=menu' "$log" ||
   ! grep -Fq 'scan complete' "$log" ||
   ! grep -Fq 'games=dm1:no,csb:yes,dm2:no,nexus:no,theron:no' "$log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Amiga edition=amiga31-multi source=$expected_prefix" "$log" ||
   ! grep -Fq 'CSB READY: gameId=csb' "$log" ||
   ! grep -Eq 'startup-source-complete game=csb phase=amiga-swsh-ftl profile=ff3872baaed8ee4e83ee3c0684b2eeec frames=[1-9][0-9]* source-vblanks=30 audio=' "$log" ||
   ! grep -Eq 'phase=csb-amiga-a31-titl.*title-frame=[1-9][0-9]*/' "$log"; then
    echo 'FAIL: CSB Amiga 3.3 no-game picker missed its authentic animated intro or runtime' >&2
    grep -E 'startup game=|scan complete|CSB READY|game-handoff|amiga-swsh-ftl|swsh-intro|csb-amiga-a31-titl|failed|error' "$log" | tail -n 100 >&2 || true
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
        probe.get("bootAssetMd5") != "61fbfd56887c94adc26888a9491c6611" or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0) or
        not probe.get("csbViewportHash") or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: CSB Amiga 3.3 picker missed its authentic first runtime: {probe}")
print("PASS: CSB Amiga 3.3 no-game picker reached the authentic first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB Amiga 3.3 archive changed during no-game startup' >&2
    exit 1
fi
echo 'PASS: CSB Amiga 3.3 no-game M12 route used and preserved the original archive'
