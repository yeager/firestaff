#!/usr/bin/env bash
set -euo pipefail

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
app=${1:?usage: test_dm1_v1_amiga20_french_archive_cli_boot.sh <firestaff-binary>}
repack_helper=${2:?usage: test_dm1_v1_amiga20_french_archive_cli_boot.sh <firestaff-binary> <real-media-helper>}
archive=${FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z"}
expected_graphics=dd373954b3fb127db7387946131ea322
expected_swsh=1038138978975415571a878bb08f54be

if [[ ! -x "$app" || ! -x "$repack_helper" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic Amiga 2.0 French ADF archive is not staged'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-amiga20-fr.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM
repacked="$scratch/Dungeon-Master_Amiga_EN_Version-20.zip"
"$repack_helper" --emit-french-adf "$archive" | python3 -c '
import sys
import zipfile
payload = sys.stdin.buffer.read()
if len(payload) != 901120 or payload[:4] != bytes((68, 79, 83, 0)):
    raise SystemExit("FAIL: A20F ADF stream is not the original OFS image")
import io
inner = io.BytesIO()
with zipfile.ZipFile(inner, "w", compression=zipfile.ZIP_STORED) as out:
    out.writestr("Dungeon Master v2.0 (1988)(FTL).adf", payload)
with zipfile.ZipFile(sys.argv[1], "w", compression=zipfile.ZIP_STORED) as out:
    out.writestr("Dungeon Master v2.0 (1988)(FTL).zip", inner.getvalue())
' "$repacked"
selected_media="$repacked::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf"

"$repack_helper" --check-virtual "$repacked"

# The private ZIP holds only the exact French ADF bytes selected from the
# original 7z. English and German siblings cannot win the architecture scan.

cli_log="$scratch/cli.log"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --debug --game dm1 --platform amiga --data-dir "$repacked" \
    --boot-probe --boot-probe-frames 2 --duration 0 >"$cli_log" 2>&1 || {
        cat "$cli_log" >&2
        exit 1
    }
for expected in \
    "assetMd5=$expected_graphics" \
    "phase=amiga-swsh profile=$expected_swsh" \
    "phase=title-f0437 frames=18 palette-fades=5 source=$expected_swsh graphics=$expected_graphics" \
    "phase=entrance-f0441 steps=31 graphics=$expected_graphics input=mouse-only palette=rgb4" \
    'phase=dm1-runtime' 'levelLoaded=1'; do
    if ! grep -Fq "$expected" "$cli_log"; then
        cat "$cli_log" >&2
        printf 'FAIL: French CLI startup lacks %s\n' "$expected" >&2
        exit 1
    fi
done

menu_log="$scratch/menu.log"
menu_probe="$scratch/menu.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --debug --menu --game dm1 --platform amiga --data-dir "$repacked" \
    --script enter,enter,enter --duration 10000 >"$menu_log" 2>&1 || {
        cat "$menu_log" >&2
        exit 1
    }
for expected in \
    "DM1 READY: gameId=dm1 dataDir=$selected_media handoff=amiga-img2" \
    "phase=amiga-swsh profile=$expected_swsh" \
    "phase=title-f0437 frames=18 palette-fades=5 source=$expected_swsh graphics=$expected_graphics" \
    "phase=entrance-f0441 steps=31 graphics=$expected_graphics input=mouse-only palette=rgb4"; do
    if ! grep -Fq "$expected" "$menu_log"; then
        cat "$menu_log" >&2
        printf 'FAIL: French M12 startup lacks %s\n' "$expected" >&2
        exit 1
    fi
done
python3 - "$menu_probe" "$expected_graphics" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or probe["bootAssetMd5"] != sys.argv[2] or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic A20F M12 handoff: {probe}")
print("PASS: authentic A20F CLI and M12 startup use the paired SWSH, F0437 title and F0441 entrance")
PY

# Exercise the same authentic edition through the no-game picker. This catches
# regressions where CLI startup recognizes French Amiga media but the launcher
# fails to classify or hand it off from its scanned data root.
no_game_data="$scratch/no-game-data"
mkdir -p "$no_game_data/dm1"
ln -s "$repacked" "$no_game_data/dm1/Dungeon-Master_Amiga_FR_Version-20.zip"
no_game_log="$scratch/no-game-menu.log"
no_game_probe="$scratch/no-game-runtime.json"
no_game_home="$scratch/no-game-home"
mkdir -p "$no_game_home"
env -u FIRESTAFF_DATA -u FIRESTAFF_ORIGINALS_DIR -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_CONFIG_PATH="$no_game_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$no_game_probe" \
    HOME="$no_game_home" XDG_CONFIG_HOME="$no_game_home" APPDATA="$no_game_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --width 1920 --height 1080 --data-dir "$no_game_data" \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait20' \
        --duration 15000 >"$no_game_log" 2>&1 || {
            cat "$no_game_log" >&2
            exit 1
        }
expected_menu_media="$no_game_data/dm1/Dungeon-Master_Amiga_FR_Version-20.zip::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf"
for expected in \
    'startup game=none mode=menu' \
    'scan complete' \
    'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no' \
    "launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga20-fr source=$expected_menu_media" \
    "phase=amiga-swsh profile=$expected_swsh" \
    'platform=amiga-v20 phase=title-f0437' \
    'platform=amiga-v20 phase=entrance-f0441' \
    'phase=dm1-runtime'; do
    if ! grep -Fq "$expected" "$no_game_log"; then
        grep -E 'startup game=|scan complete|games=|DM1 READY|game-handoff|startup-source|phase=title-f0437|phase=entrance-f0441|phase=dm1-runtime|failed|error' \
            "$no_game_log" | tail -n 100 >&2 || cat "$no_game_log" >&2
        printf 'FAIL: French no-game M12 startup lacks %s\n' "$expected" >&2
        exit 1
    fi
done
python3 - "$no_game_probe" "$expected_graphics" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm1" or probe.get("bootAssetMd5") != sys.argv[2] or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "dm1-runtime" or startup.get("levelLoaded") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("dm1StartupHandoffExecuted") != 1 or
        startup.get("dm1StartupHoCFirstFrameReady") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 1, 3, 2, 0) or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: French no-game Amiga runtime handoff: {probe}")
print("PASS: French Amiga 2.0 no-game picker reached its authentic first DM1 runtime frame")
PY
