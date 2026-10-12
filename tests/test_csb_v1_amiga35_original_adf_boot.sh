#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <DMWeb-amiga-archive.7z>\n' "$0" >&2
    exit 2
fi

app=$1
source_archive=$2
member='Floppy Disks ADF/Chaos Strikes Back for Amiga Game Disk v3.5 (French, English, German) Original (Not working).adf'
expected_adf_md5=4d148dea3fdc6d7098b6a1bbfce7b5c9
expected_graphics_md5=cefaddfdf5651df2c91f61b5611a8362

seven_zip=$(command -v 7zz || command -v 7z || true)
if [[ ! -x "$app" || ! -f "$source_archive" || -z "$seven_zip" ]]; then
    echo 'SKIP: Firestaff, original DMWeb archive, or 7-Zip CLI is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga35-adf.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

source_sha_before=$(shasum -a 256 "$source_archive" | awk '{print $1}')
adf="$scratch/amiga35-original.adf"
zip="$scratch/amiga35-original.zip"
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
    raise SystemExit("FAIL: extracted A35 original ADF does not match its source identity")
with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_STORED) as archive:
    archive.writestr("Chaos Strikes Back Amiga 3.5 Original.adf", adf)
with zipfile.ZipFile(zip_path) as archive:
    packaged = archive.read("Chaos Strikes Back Amiga 3.5 Original.adf")
if packaged != adf:
    raise SystemExit("FAIL: ZIP transport changed the original A35 ADF bytes")
print("PASS: original A35 ADF retains its MD5 through ZIP transport")
PY

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game csb --platform amiga --data-dir "$zip" \
    --debug --boot-probe --boot-probe-frames 800 \
    --script 'click:138:70,click:250:50,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 9,1,2 --duration 0 2>&1) || {
        printf '%s\n' "$direct_output" >&2
        exit 1
    }
for required in \
    "platform=Amiga edition=amiga35-multi" \
    "source=$zip::Chaos Strikes Back Amiga 3.5 Original.adf::Graphics.DAT" \
    "assetMd5=$expected_graphics_md5" \
    'phase=inactive startupActive=0' \
    'levelLoaded=1 map=0 party=9,1,2 champions=0' \
    'csbViewportHash='; do
    if ! grep -Fq "$required" <<< "$direct_output"; then
        printf 'FAIL: A35 CLI probe is missing %s\n%s\n' "$required" "$direct_output" >&2
        exit 1
    fi
done
if ! grep -Eq 'csbViewportHash=[1-9][0-9]*' <<< "$direct_output"; then
    echo 'FAIL: A35 CLI probe has no nonzero viewport receipt' >&2
    printf '%s\n' "$direct_output" >&2
    exit 1
fi
echo 'PASS: original A35 CLI selects English and reaches the first native movement'

# With only the authenticated A35M archive installed, an unqualified
# --game csb CLI launch must resolve the original Amiga edition through AUTO.
auto_direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game csb --data-dir "$zip" \
    --debug --boot-probe --boot-probe-frames 800 \
    --script 'click:138:70,click:250:50,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-party 9,1,2 --duration 0 2>&1) || {
        printf '%s\n' "$auto_direct_output" >&2
        exit 1
    }
for required in \
    'platform=Amiga edition=amiga35-multi' \
    "source=$zip::Chaos Strikes Back Amiga 3.5 Original.adf::Graphics.DAT" \
    "assetMd5=$expected_graphics_md5" \
    'levelLoaded=1 map=0 party=9,1,2 champions=0' \
    'csbViewportHash='; do
    if ! grep -Fq "$required" <<< "$auto_direct_output"; then
        printf 'FAIL: A35 AUTO CLI probe is missing %s\n%s\n' \
            "$required" "$auto_direct_output" >&2
        exit 1
    fi
done
if ! grep -Eq 'csbViewportHash=[1-9][0-9]*' <<< "$auto_direct_output"; then
    printf '%s\n' 'FAIL: A35 AUTO CLI did not render a nonzero viewport hash' \
        "$auto_direct_output" >&2
    exit 1
fi
echo 'PASS: bare --game csb selects authentic A35M and reaches first movement'

probe="$scratch/menu-runtime.json"
home="$scratch/home"
mkdir -p "$home"
menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" HOME="$home" \
    XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb --platform amiga \
    --data-dir "$zip" --debug \
    --script 'enter,enter,enter,wait:1000,click:138:70,click:250:50,wait:100,up' \
    --duration 30000 2>&1) || {
        printf '%s\n' "$menu_output" >&2
        exit 1
    }
for required in \
    "platform=Amiga edition=amiga35-multi" \
    "source=$zip::Chaos Strikes Back Amiga 3.5 Original.adf::Graphics.DAT"; do
    if ! grep -Fq "$required" <<< "$menu_output"; then
        printf 'FAIL: A35 M12 menu output is missing %s\n%s\n' "$required" "$menu_output" >&2
        exit 1
    fi
done
python3 - "$probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        probe["bootAssetMd5"] != "cefaddfdf5651df2c91f61b5611a8362" or
        not probe["csbViewportHash"] or startup["receiptReady"] != 1 or
        startup["phase"] != "inactive" or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0} or
        probe["lastOutcome"] != "MOVE QUEUED"):
    raise SystemExit(f"FAIL: A35 M12 route did not reach authentic movement: {probe}")
print("PASS: original A35 M12 menu reaches runtime and first movement")
PY

# Repeat the production M12 route without an explicit platform override. The
# clean config and isolated A35M archive make AUTO select this authentic Amiga
# release, then exercise its language selector and first runtime movement.
auto_probe="$scratch/auto-menu-runtime.json"
auto_home="$scratch/auto-home"
mkdir -p "$auto_home"
auto_menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$auto_probe" \
    FIRESTAFF_CONFIG_PATH="$auto_home/config.toml" HOME="$auto_home" \
    XDG_CONFIG_HOME="$auto_home" APPDATA="$auto_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb --data-dir "$zip" --debug \
    --script 'enter,enter,enter,wait:1000,click:138:70,click:250:50,wait:100,up' \
    --duration 30000 2>&1) || {
        printf '%s\n' "$auto_menu_output" >&2
        exit 1
    }
if ! grep -Fq 'platform=Amiga edition=amiga35-multi' <<< "$auto_menu_output" ||
   ! grep -Fq \
       "source=$zip::Chaos Strikes Back Amiga 3.5 Original.adf::Graphics.DAT" \
       <<< "$auto_menu_output"; then
    printf '%s\n' 'FAIL: A35 AUTO M12 did not select the authenticated Amiga 3.5 package' \
        "$auto_menu_output" >&2
    exit 1
fi
python3 - "$auto_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        probe["bootAssetMd5"] != "cefaddfdf5651df2c91f61b5611a8362" or
        startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0} or
        probe["lastOutcome"] != "MOVE QUEUED"):
    raise SystemExit(f"FAIL: A35 AUTO M12 did not reach authentic movement: {probe}")
print("PASS: bare --game csb AUTO M12 selects A35M and reaches first movement")
PY

source_sha_after=$(shasum -a 256 "$source_archive" | awk '{print $1}')
if [[ "$source_sha_before" != "$source_sha_after" ]]; then
    echo 'FAIL: source DMWeb archive changed during A35 verification' >&2
    exit 1
fi
