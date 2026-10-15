#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <original-CSB-CTRaw.7z>\n' "$0" >&2
    exit 2
fi
app=$1
source_archive=$2
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
mkdir -p "$scratch/home"
# M12 AUTO must launch the native English disk from the untouched .7z,
# consume a fresh entrance command, and hand movement to the loaded game.
if ! FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game csb --data-dir "$source_archive" \
    --debug --verbose --script 'enter,enter,enter,wait:1200,enter,wait:100,up' \
    --duration 35000 >"$log" 2>&1; then
    echo 'FAIL: native CSB CTRaw M12 launch failed' >&2
    exit 1
fi
for required in \
    "mode=menu game=csb platform=Amiga edition=amiga35-en source=$source_archive::ChaosStrikesBack_Disk1of2 GameDisk_English.raw::Graphics.DAT" \
    'phase=csb-entrance-0'; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: CSB CTRaw M12 launch lacks %s\n' "$required" >&2
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
        probe["bootAssetMd5"] != "291e1bc6803e3dc4b974c60117ca5d68" or
        startup["receiptReady"] != 1 or startup["phase"] != "inactive" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 1, 2, 0) or
        probe["gameTick"] <= 0 or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: native CSB CTRaw M12 did not reach loaded runtime movement: {probe}")
print("PASS: native original CSB CTRaw M12 AUTO reaches loaded gameplay and movement")
PY
if [[ "$source_hash_before" != "$(firestaff_sha256_file "$source_archive")" ]]; then
    echo 'FAIL: original CSB CTRaw archive changed during M12 launch' >&2
    exit 1
fi
echo 'PASS: original CSB Amiga 3.5 CTRaw native M12 startup'
