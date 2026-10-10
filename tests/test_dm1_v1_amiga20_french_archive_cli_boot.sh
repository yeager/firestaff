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
    "phase=title-f0437 frames=18 source=$expected_swsh graphics=$expected_graphics" \
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
    "phase=title-f0437 frames=18 source=$expected_swsh graphics=$expected_graphics" \
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
