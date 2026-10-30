#!/usr/bin/env sh
set -eu
. "$(dirname "$0")/lib/portable_sha256.sh"

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
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm2-dos-fr-picker.XXXXXX")
runtime_probe="$scratch/runtime.json"
runtime_log="$scratch/firestaff.log"
picker_home="$scratch/home"
picker_data_root="$scratch/data"
archive=$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)
picker_archive="$picker_data_root/dm2/$(basename "$archive")"
mkdir -p "$picker_home" "$(dirname "$picker_archive")"
ln -s "$archive" "$picker_archive"
archive_hash=$(firestaff_sha256_file "$archive")
cleanup() {
    if [ "${FIRESTAFF_KEEP_TEST_SCRATCH:-0}" = 1 ]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM
FIRESTAFF_CONFIG_PATH="$picker_home/config.toml" \
FIRESTAFF_DATA="$picker_data_root" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
HOME="$picker_home" XDG_CONFIG_HOME="$picker_home" APPDATA="$picker_home" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --debug --verbose --width 1920 --height 1080 --data-dir "$picker_data_root" \
    --script 'wait-scan-complete,wait20,click:1680:262,wait20,click:934:405,wait20,click:450:405,wait:1000,key:enter,key:enter' \
    --duration 30000 >"$runtime_log" 2>&1
if ! grep -Fq 'startup game=none mode=menu' "$runtime_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm2 platform=PC edition=pc-fr source=$picker_archive" "$runtime_log"; then
    echo 'FAIL: no-game M12 picker did not select the isolated authentic French DOS edition' >&2
    cat "$runtime_log" >&2
    exit 1
fi
python3 - "$runtime_probe" "$runtime_log" "$picker_archive" "$archive" <<'PY'
import json
import os
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
with open(sys.argv[2], encoding="utf-8") as log_file:
    log = log_file.read()
picker_archive = os.path.realpath(sys.argv[3])
archive = os.path.realpath(sys.argv[4])
handoffs = [line for line in log.splitlines()
            if "launch phase=game-handoff mode=menu game=dm2 "
            "platform=PC edition=pc-fr source=" in line]
if "startup game=none mode=menu" not in log or not handoffs:
    raise SystemExit("FAIL: DM2 French DOS did not start from no-game M12 or select pc-fr")
selected_source = handoffs[-1].split(" source=", 1)[1].split("::", 1)[0]
if os.path.realpath(selected_source) != archive or picker_archive != archive:
    raise SystemExit(f"FAIL: no-game M12 selected a different DM2 source: {handoffs[-1]}")
startup = probe["startup"]
party = probe["party"]
frame_ownership = probe["dm2FrameOwnership"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm2-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1)):
    raise SystemExit(f"FAIL: authentic no-game DM2 French DOS picker did not reach runtime: {probe}")
if (probe["dm2RuntimeFrame"] != {"accepted": 1, "realAssets": 1,
                                  "noCoreFallbacks": 1, "fallbackDraws": 0} or
        frame_ownership["hudBlits"] < 8 or
        frame_ownership["blockedMaterials"] != 0):
    raise SystemExit(
        "FAIL: authentic no-game DM2 French DOS frame did not consume the complete source HUD: "
        f"{probe}")
print("PASS: no-game M12 picker selected authentic DM2 French DOS and reached its runtime boundary")
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
after_hash=$(firestaff_sha256_file "$archive")
if [ "$after_hash" != "$archive_hash" ]; then
    echo 'FAIL: original DM2 French DOS archive changed during its startup tests' >&2
    exit 1
fi
echo 'PASS: native DM2 French DOS ZIP start menu reaches runtime and moves in memory'
