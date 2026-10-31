#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_amiga20_german_archive_cli_boot.sh <firestaff-binary> <media-probe>}
media_probe=${2:?usage: test_dm1_v1_amiga20_german_archive_cli_boot.sh <firestaff-binary> <media-probe>}
source_archive=${FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Amiga,Software.7z"}
member='Floppy Disks ADF/Dungeon Master for Amiga v2.0 (German) Cracked.adf'
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
expected_adf_md5=e1a0cb7ef2b8262d3f048929511052e4
expected_graphics_md5=0679e39da9dcc2e855cb33c6c64ddcb5
expected_dungeon_md5=59e332cf47852d5c7c88a9538f0adcea

if [[ ! -x "$app" || ! -x "$media_probe" || ! -f "$source_archive" ]]; then
    echo 'SKIP: authentic DM1 Amiga software archive is not staged'
    exit 77
fi
source_archive_hash=$(firestaff_sha256_file "$source_archive")
for tool in 7zz zip python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "SKIP: $tool is required to repack the original German ADF bytes into ZIP"
        exit 77
    fi
done

media_output=$(FIRESTAFF_DM1_AMIGA_SOFTWARE_ARCHIVE="$source_archive" \
    "$media_probe" --check-german-adf "$source_archive" 2>&1) || {
    printf '%s\n' "$media_output" >&2
    exit 1
}
if ! grep -Fq "graphics_md5=$expected_graphics_md5 graphics_lang=3 swsh_md5=916cb3cd462e7582d2a39c73e40b3339 dungeon_md5=$expected_dungeon_md5 world_trim_0=1 world_trim_2=0 f0437=1 f0441=1" <<<"$media_output"; then
    printf '%s\n' "$media_output" >&2
    echo 'FAIL: authentic German A20 dungeon length does not match its source-owned world layout' >&2
    exit 1
fi
echo 'PASS: authentic German A20 SWSH, title, entrance and dungeon receipts match original media'

mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-amiga20-de.XXXXXX")
trap 'rm -rf "$scratch"' EXIT HUP INT TERM
adf="$scratch/Dungeon-Master-Amiga-2.0-DE.adf"
archive="$scratch/Dungeon-Master-Amiga-2.0-DE.zip"

# Repack the exact original ADF member. The ZIP exists only to exercise the
# product's bounded-memory ZIP/ADF readers without requiring 7z at runtime.
7zz x -so "$source_archive" "$member" >"$adf"
python3 - "$adf" "$expected_adf_md5" <<'PY'
import hashlib
import pathlib
import sys

data = pathlib.Path(sys.argv[1]).read_bytes()
actual = hashlib.md5(data).hexdigest()
if len(data) != 901120 or actual != sys.argv[2]:
    raise SystemExit(
        f"FAIL: original German ADF changed: bytes={len(data)} md5={actual}")
PY
zip -0 -j -q "$archive" "$adf"
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --verbose \
    --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-asset-md5 "$expected_graphics_md5" --duration 0 2>&1) || {
    printf '%s\n' "$direct_output" >&2
    exit 1
}
if ! grep -Fq "selected game=dm1 platform=Amiga edition=amiga20-de source=$archive::Dungeon-Master-Amiga-2.0-DE.adf::graphics.dat" <<<"$direct_output" ||
   ! grep -Fq "inventory-file=dungeon matched source=$archive::Dungeon-Master-Amiga-2.0-DE.adf::DUNGEON.DAT" <<<"$direct_output" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$direct_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$direct_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$direct_output"; then
    printf '%s\n' "$direct_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 CLI launch did not load its paired dungeon' >&2
    exit 1
fi
echo 'PASS: authentic German Amiga 2.0 CLI launch loads both original files'

trace_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 2 --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --duration 0 2>&1) || {
    printf '%s\n' "$trace_output" >&2
    exit 1
}
if ! grep -Fq 'platform=amiga-v20 phase=title-f0437' <<<"$trace_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' <<<"$trace_output" ||
   ! grep -Fq 'startup-media-check-skipped game=dm1 edition=amiga20-de check=F0906-F0907 reason=not-compiled-for-A20G-A22G' <<<"$trace_output" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$trace_output"; then
    printf '%s\n' "$trace_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 CLI skipped its authenticated source title or entrance' >&2
    exit 1
fi
echo 'PASS: direct German Amiga 2.0 launch completes F0437 and F0441'

# The same authentic package must be discoverable through M12's normal game
# menu and launch the German edition selected by its media, without synthetic
# fixtures or external archive tools.
menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" --debug \
    --script 'enter,enter,enter' --duration 5000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq 'handoff=amiga-img2' <<<"$menu_output" ||
   ! grep -Fq 'edition=amiga20-de' <<<"$menu_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=title-f0437' <<<"$menu_output" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    echo 'FAIL: authentic German Amiga 2.0 start-menu launch did not select its edition' >&2
    exit 1
fi
echo 'PASS: authentic German Amiga 2.0 start-menu launch reaches runtime'

# Keep M12 alive through the selected German edition's first loaded Hall.
# The earlier handoff-only run proves card/version selection; this second run
# proves that the exact original ADF owner survives the native startup path.
menu_runtime_log="$scratch/menu-runtime.log"
menu_runtime_probe="$scratch/menu-runtime.json"
mkdir -p "$scratch/menu-runtime-home"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
    FIRESTAFF_CONFIG_PATH="$scratch/menu-runtime.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe" \
    HOME="$scratch/menu-runtime-home" \
    XDG_CONFIG_HOME="$scratch/menu-runtime-home" \
    APPDATA="$scratch/menu-runtime-home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --menu --game dm1 --platform amiga --data-dir "$archive" \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait20' \
        --duration 15000
) >"$menu_runtime_log" 2>&1 || {
    cat "$menu_runtime_log" >&2
    exit 1
}
if ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga20-de source=$archive::Dungeon-Master-Amiga-2.0-DE.adf::graphics.dat" "$menu_runtime_log" ||
   ! grep -Fq 'platform=amiga-v20 phase=title-f0437' "$menu_runtime_log" ||
   ! grep -Fq 'platform=amiga-v20 phase=entrance-f0441' "$menu_runtime_log"; then
    echo 'FAIL: German A20 M12 runtime did not retain the authenticated title and Entrance route' >&2
    cat "$menu_runtime_log" >&2
    exit 1
fi
python3 - "$menu_runtime_probe" "$expected_graphics_md5" <<'PY'
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
        startup.get("active") != 1 or startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("dm1StartupHandoffExecuted") != 1 or
        startup.get("dm1StartupHoCFirstFrameReady") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: German A20 M12 missed its authenticated first Hall: {probe}")
print("PASS: authentic German Amiga 2.0 M12 reaches its first Hall runtime frame")
PY

if [[ $(firestaff_sha256_file "$source_archive") != "$source_archive_hash" ]]; then
    echo 'FAIL: original German Amiga software archive changed during startup checks' >&2
    exit 1
fi
