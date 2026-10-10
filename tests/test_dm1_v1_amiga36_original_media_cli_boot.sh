#!/usr/bin/env bash
set -euo pipefail

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

app=${1:?usage: test_dm1_v1_amiga36_original_media_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_AMIGA36_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Amiga_36.zip"}
expected_md5=7f9458e4a3972d06e649a6fa85a7f34b

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga 3.6 original-media ZIP is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime ' <<<"$output" &&
    grep -Fq 'map=0 party=1,3,2 champions=0' <<<"$output" &&
    grep -Fq 'dm1StartupHandoffExecuted=1' <<<"$output" &&
    grep -Fq "startup-f0441-authenticated-handoff game=dm1 platform=amiga-v36 graphics=$expected_md5 input=keyboard+mouse palette=rgb4 visual-parity=unverified" <<<"$output"
}

probe --debug --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0

menu_runtime_probe=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}/dm1-amiga36-menu-runtime-$$.json
mkdir -p "$(dirname "$menu_runtime_probe")"
cleanup() { rm -f "$menu_runtime_probe" "${menu_runtime_probe%.json}.log"; }
trap cleanup EXIT HUP INT TERM
menu_output=${menu_runtime_probe%.json}.log
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_ENTRANCE_INPUT=key:return \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --debug \
    --menu --game dm1 --platform amiga --data-dir "$archive" \
    --script enter,enter,enter --duration 10000 >"$menu_output" 2>&1
grep -Fq "startup-f0441-authenticated-handoff game=dm1 platform=amiga-v36 graphics=$expected_md5 input=keyboard+mouse palette=rgb4 visual-parity=unverified" "$menu_output"
grep -Fq 'startup-input-source keyboard command=' "$menu_output"
python3 - "$menu_runtime_probe" "$expected_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != sys.argv[2] or
        startup["receiptReady"] != 1 or startup["phase"] != "dm1-runtime" or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        (probe["party"]["mapIndex"], probe["party"]["mapX"],
         probe["party"]["mapY"], probe["party"]["direction"],
         probe["party"]["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic DM1 Amiga 3.6 M12 launch failed: {probe}")
print("PASS: authentic DM1 Amiga 3.6 M12 reaches its direct runtime frame")
PY

printf '%s\n' \
    'PASS: authentic DM1 Amiga 3.6 CLI and start-menu authenticated entrance handoff routes' \
    'PASS: M12 consumed a fresh Return after launch-input drain and reached the expected first A36 runtime pose' \
    'NOTE: A36 visual entrance parity is unverified; C430/C431/C432/C427/C435 zones and C011 button placement are not yet rendered from original layout records'
