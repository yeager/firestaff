#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_csb_only_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_csb_only_real_media.sh <firestaff> <data-root>}
archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic CSB FM Towns archive is required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-csb-only.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

home="$scratch/home"
csb_root="$home/.firestaff/data/csb"
archive_name=$(basename "$archive")
mkdir -p "$csb_root"
ln -s "$archive" "$csb_root/$archive_name"
archive_hash=$(firestaff_sha256_file "$archive")
probe="$scratch/runtime.json"
log="$scratch/firestaff.log"

# Start with no CLI game/menu selection and only CSB in the documented user
# data tree. Moving down once must reach CSB even while DM1 is unavailable.
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' \
        --duration 30000
) >"$log" 2>&1 || {
    cat "$log" >&2
    exit 1
}

expected_source="$csb_root/$archive_name::CDATA/GRAPHICS.DAT"
for required in \
    'startup game=none mode=menu' \
    "CSB READY: gameId=csb dataDir=$csb_root/$archive_name variant=csb-fmtowns-en" \
    "launch phase=game-handoff mode=menu game=csb platform=FM Towns edition=fmtowns-en source=$expected_source"; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: CSB-only no-game menu is missing %s\n' "$required" >&2
        cat "$log" >&2
        exit 1
    fi
done

python3 - "$probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "inactive" or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1) or
        not probe.get("csbViewportHash")):
    raise SystemExit(f"FAIL: CSB-only no-game menu missed authentic runtime: {probe}")
print("PASS: CSB-only no-game M12 menu reached the original FM Towns first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB archive changed during the M12 menu launch' >&2
    exit 1
fi
echo 'PASS: default .firestaff/data/csb discovery launched only authentic CSB without CLI game selection'

# Escape from the platform picker must return to the game list. Reselect the
# still-highlighted CSB card and require exactly one authentic handoff.
back_home="$scratch/back-home"
back_csb_root="$back_home/.firestaff/data/csb"
mkdir -p "$back_csb_root"
ln -s "$archive" "$back_csb_root/$archive_name"
back_probe="$scratch/back-runtime.json"
back_log="$scratch/back-firestaff.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$back_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$back_probe" \
    HOME="$back_home" XDG_CONFIG_HOME="$back_home" APPDATA="$back_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:escape,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' \
        --duration 30000
) >"$back_log" 2>&1 || {
    cat "$back_log" >&2
    exit 1
}

if [[ $(grep -Fc 'launch phase=game-handoff' "$back_log") != 1 ]] ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=FM Towns edition=fmtowns-en source=$back_csb_root/$archive_name::CDATA/GRAPHICS.DAT" "$back_log"; then
    echo 'FAIL: backing out of CSB platform selection did not return to the game list and launch exactly once after reselection' >&2
    grep -E 'startup game=|game-handoff|error|failed' "$back_log" >&2 || true
    exit 1
fi

python3 - "$back_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "inactive" or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1) or
        not probe.get("csbViewportHash")):
    raise SystemExit(f"FAIL: CSB Back/reselect missed authentic runtime: {probe}")
print("PASS: backing out of CSB platform choice and reselecting reaches authentic FM Towns runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB archive changed during Back/reselect menu launch' >&2
    exit 1
fi
