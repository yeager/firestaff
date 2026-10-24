#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    printf 'usage: %s <firestaff-binary> <m12-m11-boundary-test> <DMWeb-amiga-archive.7z>\n' "$0" >&2
    exit 2
fi

app=$1
boundary_test=$2
source_archive=$3
member='Floppy Disks ADF/Chaos Strikes Back for Amiga Game Disk v3.5 (English) Original (Not working).adf'
expected_adf_md5=d1808d54faf36c6ab7f054908ce7ebd5
expected_graphics_md5=291e1bc6803e3dc4b974c60117ca5d68

seven_zip=$(command -v 7zz || command -v 7z || true)
if [[ ! -x "$app" || ! -x "$boundary_test" || ! -f "$source_archive" || -z "$seven_zip" ]]; then
    echo 'SKIP: Firestaff, CSB boundary test, original DMWeb archive, or 7-Zip CLI is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga35-english-adf.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        python3 - "$scratch" <<'PY'
import pathlib
import shutil
import sys
shutil.rmtree(pathlib.Path(sys.argv[1]))
PY
    fi
}
trap cleanup EXIT HUP INT TERM

source_sha_before=$(shasum -a 256 "$source_archive" | awk '{print $1}')
adf="$scratch/amiga35-english-original.adf"
zip="$scratch/amiga35-english-original.zip"
"$seven_zip" x -so "$source_archive" "$member" > "$adf"
python3 - "$adf" "$zip" "$expected_adf_md5" <<'PY'
import hashlib
import pathlib
import sys
import zipfile

adf_path = pathlib.Path(sys.argv[1])
zip_path = pathlib.Path(sys.argv[2])
expected_md5 = sys.argv[3]
adf = adf_path.read_bytes()
if hashlib.md5(adf).hexdigest() != expected_md5:
    raise SystemExit("FAIL: extracted A35E ADF does not match the authenticated original identity")
with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_STORED) as archive:
    archive.writestr("Chaos Strikes Back Amiga 3.5 English Original.adf", adf)
with zipfile.ZipFile(zip_path) as archive:
    packaged = archive.read("Chaos Strikes Back Amiga 3.5 English Original.adf")
if packaged != adf:
    raise SystemExit("FAIL: ZIP transport changed the original A35E ADF bytes")
print("PASS: authentic A35E ADF retains its identity through ZIP transport")
PY

title_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game csb --platform amiga --data-dir "$zip" \
    --debug --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
        printf '%s\n' "$title_output" >&2
        exit 1
    }
for required in \
    'edition=amiga35-en' \
    "assetMd5=$expected_graphics_md5" \
    'phase=csb-entrance-0' \
    'startupActive=1' \
    'levelLoaded=1' \
    'party=9,0,2'; do
    if ! grep -Fq "$required" <<< "$title_output"; then
        printf 'FAIL: A35E CLI startup is missing %s\n%s\n' "$required" "$title_output" >&2
        exit 1
    fi
done
echo 'PASS: original A35E CLI selects its authenticated package and opens the native entrance'

runtime_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game csb --platform amiga --data-dir "$zip" \
    --debug --boot-probe --boot-probe-frames 800 --script 'enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-runtime-tick-min 1 --duration 0 2>&1) || {
        printf '%s\n' "$runtime_output" >&2
        exit 1
    }
for required in \
    'edition=amiga35-en' \
    "assetMd5=$expected_graphics_md5" \
    'phase=inactive' \
    'startupActive=0' \
    'levelLoaded=1' \
    'party=9,1,2'; do
    if ! grep -Fq "$required" <<< "$runtime_output"; then
        printf 'FAIL: A35E CLI runtime is missing %s\n%s\n' "$required" "$runtime_output" >&2
        exit 1
    fi
done
if ! grep -Eq 'csbViewportHash=[1-9][0-9]*' <<< "$runtime_output"; then
    printf 'FAIL: A35E runtime did not publish a nonzero viewport receipt\n%s\n' "$runtime_output" >&2
    exit 1
fi
if ! grep -Fq 'game-action game=csb tick=native input=forward' <<< "$runtime_output" ||
   ! grep -Fq 'game-debug-capabilities game=csb source=csb-runtime' <<< "$runtime_output" ||
   ! grep -Fq 'player-attack=source-melee-receipt monster-health=source-melee-hp-receipt' <<< "$runtime_output" ||
   ! grep -Fq 'champion-health=source-party-state' <<< "$runtime_output" ||
   ! grep -Fq 'game-state game=csb source=csb-runtime tick=' <<< "$runtime_output" ||
   ! grep -Fq 'party=9,1,0 dir=2' <<< "$runtime_output"; then
    printf 'FAIL: CSB debug action did not report the source runtime party state\n%s\n' \
        "$runtime_output" >&2
    exit 1
fi
echo 'PASS: original A35E CLI consumes its first movement and renders the native viewport'

# The no-platform route must resolve this original Amiga edition from the
# authenticated media identity rather than requiring an explicit platform.
auto_runtime_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game csb --data-dir "$zip" \
    --debug --boot-probe --boot-probe-frames 800 --script 'enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-runtime-tick-min 1 --duration 0 2>&1) || {
        printf '%s\n' "$auto_runtime_output" >&2
        exit 1
    }
for required in \
    'platform=Amiga edition=amiga35-en' \
    "assetMd5=$expected_graphics_md5" \
    'phase=inactive' \
    'startupActive=0' \
    'levelLoaded=1' \
    'party=9,1,2'; do
    if ! grep -Fq "$required" <<< "$auto_runtime_output"; then
        printf 'FAIL: A35E bare CLI route is missing %s\n%s\n' "$required" "$auto_runtime_output" >&2
        exit 1
    fi
done
if ! grep -Eq 'csbViewportHash=[1-9][0-9]*' <<< "$auto_runtime_output"; then
    printf 'FAIL: A35E bare CLI route did not publish a nonzero viewport receipt\n%s\n' "$auto_runtime_output" >&2
    exit 1
fi
echo 'PASS: bare --game csb selects authentic A35E and reaches first movement'

menu_probe="$scratch/menu-runtime.json"
menu_home="$scratch/menu-home"
mkdir -p "$menu_home"
menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
    FIRESTAFF_CONFIG_PATH="$scratch/menu-config.toml" \
    HOME="$menu_home" XDG_CONFIG_HOME="$menu_home" \
    APPDATA="$menu_home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb --platform amiga \
    --data-dir "$zip" --debug --verbose \
    --script 'enter,enter,enter,wait:1200,enter,wait:100,up' \
    --duration 35000 2>&1) || {
        printf '%s\n' "$menu_output" >&2
        exit 1
    }
for required in \
    "mode=menu game=csb platform=Amiga edition=amiga35-en source=$zip::Chaos Strikes Back Amiga 3.5 English Original.adf::Graphics.DAT" \
    'phase=csb-entrance-0' \
    'party=9,0 dir=2'; do
    if ! grep -Fq "$required" <<< "$menu_output"; then
        printf 'FAIL: A35E visible M12 menu route is missing %s\n%s\n' "$required" "$menu_output" >&2
        exit 1
    fi
done
python3 - "$menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        probe["bootAssetMd5"] != "291e1bc6803e3dc4b974c60117ca5d68" or
        not probe["csbViewportHash"] or startup["receiptReady"] != 1 or
        startup["phase"] != "inactive" or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0} or
        probe["lastOutcome"] != "MOVE QUEUED"):
    raise SystemExit(f"FAIL: authentic A35E M12 route did not reach runtime movement: {probe}")
print("PASS: authentic A35E M12 menu reaches the first runtime movement")
PY

# Exercise production M12 AUTO with no --platform pin, using an isolated
# config root so a previously selected edition cannot affect the result.
auto_menu_probe="$scratch/auto-menu-runtime.json"
auto_menu_home="$scratch/auto-menu-home"
mkdir -p "$auto_menu_home"
auto_menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$auto_menu_probe" \
    FIRESTAFF_CONFIG_PATH="$scratch/auto-menu-config.toml" \
    HOME="$auto_menu_home" XDG_CONFIG_HOME="$auto_menu_home" \
    APPDATA="$auto_menu_home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb \
    --data-dir "$zip" --debug --verbose \
    --script 'enter,enter,enter,wait:1200,enter,wait:100,up' \
    --duration 35000 2>&1) || {
        printf '%s\n' "$auto_menu_output" >&2
        exit 1
    }
for required in \
    "mode=menu game=csb platform=Amiga edition=amiga35-en source=$zip::Chaos Strikes Back Amiga 3.5 English Original.adf::Graphics.DAT" \
    'phase=csb-entrance-0' \
    'party=9,0 dir=2'; do
    if ! grep -Fq "$required" <<< "$auto_menu_output"; then
        printf 'FAIL: A35E M12 AUTO route is missing %s\n%s\n' "$required" "$auto_menu_output" >&2
        exit 1
    fi
done
python3 - "$auto_menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        probe["bootAssetMd5"] != "291e1bc6803e3dc4b974c60117ca5d68" or
        not probe["csbViewportHash"] or startup["receiptReady"] != 1 or
        startup["phase"] != "inactive" or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0} or
        probe["lastOutcome"] != "MOVE QUEUED"):
    raise SystemExit(f"FAIL: A35E M12 AUTO route did not reach runtime movement: {probe}")
print("PASS: authentic A35E M12 AUTO reaches the first runtime movement")
PY

handoff_output=$(FIRESTAFF_CSB_AMIGA35_EN_DATA_DIR="$zip" \
    FIRESTAFF_CSB_AMIGA35_EN_ONLY=1 "$boundary_test" 2>&1) || {
        printf '%s\n' "$handoff_output" >&2
        exit 1
    }
if ! grep -Fq '0 failed, 0 skipped' <<< "$handoff_output" ||
   ! grep -Fq 'M12/M11 launcher handoff boundary is wired' <<< "$handoff_output"; then
    printf 'FAIL: original A35E M12-to-M11 handoff was not fully verified\n%s\n' "$handoff_output" >&2
    exit 1
fi
printf '%s\n' "$handoff_output" | tail -4

source_sha_after=$(shasum -a 256 "$source_archive" | awk '{print $1}')
if [[ "$source_sha_before" != "$source_sha_after" ]]; then
    echo 'FAIL: source DMWeb archive changed during A35E verification' >&2
    exit 1
fi
