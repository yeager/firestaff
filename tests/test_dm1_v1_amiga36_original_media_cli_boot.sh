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
    grep -Fq 'phase=dm1-runtime-direct' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0

menu_runtime_probe=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}/dm1-amiga36-menu-runtime-$$.json
mkdir -p "$(dirname "$menu_runtime_probe")"
cleanup() { rm -f "$menu_runtime_probe"; }
trap cleanup EXIT HUP INT TERM
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" \
    --script enter,enter,enter --duration 10000 >/dev/null 2>&1
python3 - "$menu_runtime_probe" "$expected_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != sys.argv[2] or
        startup["receiptReady"] != 1 or startup["phase"] != "dm1-runtime-direct" or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1):
    raise SystemExit(f"FAIL: authentic DM1 Amiga 3.6 M12 launch failed: {probe}")
print("PASS: authentic DM1 Amiga 3.6 M12 reaches its direct runtime frame")
PY

printf '%s\n' \
    'PASS: authentic DM1 Amiga 3.6 CLI and start-menu runtime routes' \
    'NOTE: the source-owned Amiga F0441 entrance is not covered by this direct-runtime route'
