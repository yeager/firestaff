#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_amiga_hd_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_AMIGA_HD_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Amiga_EN.zip"}
expected_md5=6a2f135b53c2220f0251fa103e2a6e7e

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga HD archive is not staged'
    exit 77
fi
archive_hash_before=$(firestaff_sha256_file "$archive")

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
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --debug --menu --game dm1 \
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
no_game_scratch=$(mktemp -d "$test_scratch/dm1-amiga-hd-no-game.XXXXXX")
cleanup() {
    rm -f "$menu_runtime_probe_json" "$no_game_scratch/runtime.json"
    rm -rf "$no_game_scratch"
}
trap cleanup EXIT HUP INT TERM
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

# First-run selection must also discover the HD archive from a clean data root
# without --game. Keep other installed editions and saved user settings out of
# this route so the test proves the authentic Amiga HD handoff itself.
mkdir -p "$no_game_scratch/data/dm1" "$no_game_scratch/home"
ln -s "$(cd "$(dirname "$archive")" && pwd -P)/$(basename "$archive")" \
    "$no_game_scratch/data/dm1/$(basename "$archive")"
no_game_archive_hash_before=$(firestaff_sha256_file "$archive")
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    HOME="$no_game_scratch/home" \
    XDG_CONFIG_HOME="$no_game_scratch/home" \
    APPDATA="$no_game_scratch/home" \
    FIRESTAFF_CONFIG_PATH="$no_game_scratch/home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$no_game_scratch/runtime.json" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --data-dir "$no_game_scratch/data" --debug \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        --duration 15000 >"$no_game_scratch/no-game.log" 2>&1
) || {
    cat "$no_game_scratch/no-game.log" >&2
    exit 1
}
grep -Fq 'startup game=none mode=menu' "$no_game_scratch/no-game.log" &&
grep -Fq 'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no' "$no_game_scratch/no-game.log" &&
grep -Fq 'launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga20-en' \
    "$no_game_scratch/no-game.log" &&
grep -Fq 'Dungeon-Master_Amiga_EN.zip::Dungeon Master (1988)(FTL)[HD].zip::Dungeon Master (1988)(FTL)[HD].adf::GRAPHICS.DAT' \
    "$no_game_scratch/no-game.log" || {
    cat "$no_game_scratch/no-game.log" >&2
    printf '%s\n' 'FAIL: no-game menu did not select authentic DM1 Amiga HD media' >&2
    exit 1
}
python3 - "$no_game_scratch/runtime.json" "$expected_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm1" or
        probe.get("bootAssetMd5") != sys.argv[2] or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "dm1-runtime" or
        startup.get("active") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) !=
        (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: DM1 Amiga HD no-game M12 route missed runtime: {probe}")
print("PASS: no-game M12 selected authentic DM1 Amiga HD and reached runtime")
PY
if [[ $(firestaff_sha256_file "$archive") != "$no_game_archive_hash_before" ||
      $(firestaff_sha256_file "$archive") != "$archive_hash_before" ]]; then
    printf '%s\n' 'FAIL: original DM1 Amiga HD archive changed during startup tests' >&2
    exit 1
fi
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
