#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

app=${1:?usage: test_dm1_v1_amiga_hd_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_AMIGA_HD_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Amiga_EN.zip"}
expected_md5=6a2f135b53c2220f0251fa103e2a6e7e

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga HD archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2; return 1;
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform amiga --data-dir "$archive" --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform amiga --data-dir "$archive" --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform amiga --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2; exit 1;
}
grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" &&
grep -Fq "dataDir=$archive" <<<"$menu_output" &&
grep -Fq 'handoff=amiga-img2' <<<"$menu_output"

# Verify the actual M12 -> Amiga HD route through a first runtime frame.
# Unlike Amiga v2.0 floppy media, the installed HD image has no source-owned
# title/Entrance transaction; require its authenticated image and native
# dungeon pose instead of borrowing the v2.0 handoff receipt.
test_scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$test_scratch"
menu_runtime_probe_json="$test_scratch/dm1-amiga-hd-menu-runtime-$$.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" \
    --script enter,enter,enter --duration 10000 >/dev/null 2>&1
python3 - "$menu_runtime_probe_json" "$expected_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != sys.argv[2] or
        startup["receiptReady"] != 1 or startup["phase"] != "dm1-runtime" or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(
        f"FAIL: authentic DM1 Amiga HD M12 route did not reach the first "
        f"runtime frame: {probe}")
print("PASS: authentic DM1 Amiga HD start menu reached its native runtime frame")
PY
rm -f "$menu_runtime_probe_json"
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2; exit 1;
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 Amiga up input did not reach native movement' >&2
    exit 1
fi

# Exercise the complete initial input matrix from a fresh native launch for
# every row.  The original ZIP → ZIP → ADF package remains the sole source of
# title, dungeon and party state; no generated save or replacement map can
# contribute to these results.
probe_runtime_input() {
    local input=$1
    local expected_party=$2
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform amiga --data-dir "$archive" \
        --boot-probe --boot-probe-frames 500 --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output" &&
    grep -Fq "map=0 party=$expected_party" <<<"$output"
}

# A fresh south input is blocked at the authentic initial position.  The
# north-then-south sequence verifies backing up from the open tile.
probe_runtime_input up 1,4,2
probe_runtime_input down 1,3,2
probe_runtime_input up,down 1,3,2
probe_runtime_input left 1,3,1
probe_runtime_input right 1,3,3
probe_runtime_input strafe-left 1,3,2
probe_runtime_input strafe-right 1,3,2
probe_runtime_input action 1,3,2

for mode in v1 v20 v21; do
    case "$mode" in v1) mode_index=0;; v20) mode_index=1;; v21) mode_index=2;; esac
    mode_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform amiga --data-dir "$archive" \
        --presentation-mode "$mode" \
        --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
        printf '%s\n' "$mode_output" >&2; exit 1;
    }
    if ! grep -Fq "presentationMode=$mode_index " <<<"$mode_output" ||
       ! grep -Fq "assetMd5=$expected_md5" <<<"$mode_output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$mode_output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$mode_output"; then
        printf '%s\n' "$mode_output" >&2
        printf 'FAIL: Amiga CLI did not retain requested %s presentation\n' "$mode" >&2
        exit 1
    fi
done
printf '%s\n' 'PASS: authentic DM1 Amiga ZIP reaches CLI/menu, retains three CLI presentation modes, and accepts the native input matrix in memory'
