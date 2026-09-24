#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_fr_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_FR_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_FR.zip"}

# The native ZIP reader owns this separate French retail identity as well.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 French DOS archive is not staged'
    exit 77
fi

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# A launch exit only proves M12 handed the French archive to M11. Drive the
# ordinary menu, wait until the retail MVE owner has finished, and then send
# the two source New-Game inputs. The boot-probe below remains a separate
# direct-launch check and cannot stand in for this menu route.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_probe="$app_dir/dm2-dos-fr-menu-runtime-$$.json"
trap 'rm -f "$runtime_probe"' EXIT HUP INT TERM
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game dm2 --platform pc \
    --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter,wait:1000,key:enter,key:enter' \
    --duration 30000 >/dev/null 2>&1
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm2-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1)):
    raise SystemExit(f"FAIL: authentic DM2 French DOS start menu did not reach runtime: {probe}")
print("PASS: authentic DM2 French DOS start menu reached its source runtime boundary")
PY

# The French retail GDAT has its own authenticated identity.  It shares the
# DOS dungeon bytes with the English release but must retain the PC French
# GAME_LOAD owner all the way through the first input, rather than borrowing
# an English startup path.
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform pc --data-dir "$archive" --boot-probe \
    --boot-probe-frames 5000 --script 'key:enter,key:enter,key:enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

case "$output" in
    *'assetMd5=b4d733576ea60c41737f79f212faf528'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$output" >&2; exit 1 ;;
esac
echo 'PASS: native DM2 French DOS ZIP start menu reaches runtime and moves in memory'
