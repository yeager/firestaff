#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

if [[ $# -lt 2 || $# -gt 3 ]]; then
    printf 'usage: %s <firestaff-binary> <original-CSB-CTRaw.7z> [en|multi]\n' "$0" >&2
    exit 2
fi
app=$1
source_archive=$2
mode=${3:-en}
if [[ "$mode" != en && "$mode" != multi ]]; then
    echo 'FAIL: expected mode en or multi' >&2
    exit 2
fi
if [[ ! -x "$app" || ! -f "$source_archive" ]] ||
    ! command -v python3 >/dev/null 2>&1; then
    echo 'SKIP: Firestaff, authentic CSB CTRaw archive, or Python is unavailable'
    exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-amiga35-ctraw-menu.XXXXXX")
log="$scratch/menu.log"
probe="$scratch/runtime.json"
cleanup() {
    rc=$?
    if [[ $rc != 0 && -f "$log" ]]; then cat "$log" >&2; fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
source_hash_before=$(firestaff_sha256_file "$source_archive")
selected_archive=$source_archive
member='ChaosStrikesBack_Disk1of2 GameDisk_English.raw'
expected_edition=amiga35-en
expected_graphics_md5=291e1bc6803e3dc4b974c60117ca5d68
menu_script='enter,enter,enter,wait:1200,enter,wait:100,up'
if [[ "$mode" = multi ]]; then
    if ! command -v 7zz >/dev/null 2>&1; then
        echo 'SKIP: development-only multilingual CTRaw staging requires 7zz'
        exit 77
    fi
    selected_archive="$scratch/CSB-Multilingual.zip"
    member='ChaosStrikesBack_Disk1of2_GameDisk_EnglishFrenchGerman.raw'
    expected_edition=amiga35-multi
    expected_graphics_md5=cefaddfdf5651df2c91f61b5611a8362
    # Match the original A35M language/entrance mouse route already exercised
    # by test_csb_v1_amiga35_original_adf_boot.sh, without a replacement ADF.
    menu_script='enter,enter,enter,wait:1000,click:138:70,click:250:50,wait:100,up'
    python3 - "$source_archive" "$selected_archive" "$member" <<'PYMEDIA'
import subprocess
import sys
import zipfile
import zlib

raw = subprocess.run(["7zz", "x", "-so", "-bd", sys.argv[1], sys.argv[3]],
                     stdout=subprocess.PIPE, check=True).stdout
if len(raw) != 3331587 or raw[:4] != b"CAPS" or zlib.crc32(raw) != 0x91568E76:
    raise SystemExit("FAIL: original multilingual CTRaw member identity changed")
with zipfile.ZipFile(sys.argv[2], "w", compression=zipfile.ZIP_STORED) as output:
    output.writestr(sys.argv[3], raw)
with zipfile.ZipFile(sys.argv[2]) as output:
    if output.namelist() != [sys.argv[3]] or output.read(sys.argv[3]) != raw:
        raise SystemExit("FAIL: staging changed original multilingual CTRaw bytes")
PYMEDIA
fi
selected_hash_before=$(firestaff_sha256_file "$selected_archive")
mkdir -p "$scratch/home"
# M12 AUTO must retain the selected native capture, consume fresh language
# and entrance commands, and hand movement to the loaded game.
if ! FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb --data-dir "$selected_archive" \
    --debug --verbose --script "$menu_script" \
    --duration 35000 >"$log" 2>&1; then
    echo 'FAIL: native CSB CTRaw M12 launch failed' >&2
    exit 1
fi
for required in \
    "mode=menu game=csb platform=Amiga edition=$expected_edition source=$selected_archive::$member::Graphics.DAT"; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: CSB CTRaw M12 launch lacks %s\n' "$required" >&2
        exit 1
    fi
done
if [[ "$mode" = en ]] && ! grep -Fq 'phase=csb-entrance-0' "$log"; then
    echo 'FAIL: English CTRaw M12 did not reach its native entrance' >&2
    exit 1
fi
python3 - "$probe" "$expected_graphics_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        probe["bootAssetMd5"] != sys.argv[2] or
        startup["receiptReady"] != 1 or startup["phase"] != "inactive" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["gameTick"] <= 0 or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: native CSB CTRaw M12 did not reach loaded runtime movement: {probe}")
print("PASS: native original CSB CTRaw M12 AUTO reaches loaded gameplay and movement")
PY
if [[ "$selected_hash_before" != "$(firestaff_sha256_file "$selected_archive")" ]]; then
    echo 'FAIL: selected original CTRaw archive changed during M12 launch' >&2
    exit 1
fi
if [[ "$source_hash_before" != "$(firestaff_sha256_file "$source_archive")" ]]; then
    echo 'FAIL: original CSB CTRaw archive changed during M12 launch' >&2
    exit 1
fi
echo "PASS: original CSB Amiga 3.5 CTRaw native M12 startup mode=$mode"
