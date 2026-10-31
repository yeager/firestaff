#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_amiga_demo_archive_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE:-"$HOME/.firestaff/data/dm1/dm-amiga-demo-en.zip"}
expected_graphics_md5=491ca939f9abb33ceeb26619b841fe91
expected_dungeon_md5=78848e1a2d3d5a11e5954deb8c7b772b

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga demo archive is not staged'
    exit 77
fi

archive_hash_before=$(firestaff_sha256_file "$archive")
python3 - "$archive" "$expected_graphics_md5" "$expected_dungeon_md5" <<'PY'
import hashlib
import sys
import zipfile

archive, expected_graphics, expected_dungeon = sys.argv[1:]
with zipfile.ZipFile(archive) as package:
    graphics = hashlib.md5(package.read("graphics.dat")).hexdigest()
    dungeon = hashlib.md5(package.read("DemoDun.dat")).hexdigest()
if (graphics, dungeon) != (expected_graphics, expected_dungeon):
    raise SystemExit(
        f"FAIL: demo package has unexpected original media hashes: "
        f"graphics={graphics} dungeon={dungeon}")
PY

direct_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" --debug \
    --boot-probe --boot-probe-frames 120 --duration 0 2>&1) || {
    printf '%s\n' "$direct_output" >&2
    exit 1
}
grep -Fq "edition=amiga-demo-en matched source=$archive::graphics.dat" <<<"$direct_output"
grep -Fq "inventory-file=graphics matched source=$archive::graphics.dat" <<<"$direct_output"
grep -Fq "inventory-file=dungeon matched source=$archive::DemoDun.dat" <<<"$direct_output"
grep -Fq "FIRESTAFF BOOT PROBE READY: gameId=dm1 sourceKind=0 sourceId=dm1 assetMd5=$expected_graphics_md5" <<<"$direct_output"
grep -Fq 'phase=dm1-runtime' <<<"$direct_output"
grep -Fq 'levelLoaded=1' <<<"$direct_output"

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/amiga-demo-loose.XXXXXX")
trap 'rm -rf "$scratch"' EXIT

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" --debug --verbose \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
grep -Fq 'edition=amiga-demo-en' <<<"$menu_output"
grep -Fq "DM1 READY: gameId=dm1 dataDir=$archive" <<<"$menu_output"
grep -Fq "graphicsPath=$archive::graphics.dat" <<<"$menu_output"

mkdir -p "$scratch/home"
menu_runtime_log="$scratch/menu-runtime.log"
menu_runtime_probe="$scratch/menu-runtime.json"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
    FIRESTAFF_CONFIG_PATH="$scratch/menu-runtime.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --menu --game dm1 --platform amiga --data-dir "$archive" \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait-game-runtime' \
        --duration 10000
) >"$menu_runtime_log" 2>&1 || {
    cat "$menu_runtime_log" >&2
    exit 1
}
if ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga-demo-en source=$archive::graphics.dat" "$menu_runtime_log" ||
   ! grep -Fq 'DM1 READY: gameId=dm1' "$menu_runtime_log"; then
    echo 'FAIL: authentic DM1 Amiga demo M12 route did not retain its selected media owner' >&2
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
        startup.get("dm1StartupSwshConsumed") != 1 or
        startup.get("dm1StartupTitleConsumed") != 1 or
        startup.get("dm1StartupEntranceConsumed") != 1 or
        startup.get("dm1StartupFullGraphicsConsumed") != 1 or
        startup.get("dm1StartupHoCFirstFrameReady") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 7, 1, 2, 0)):
    raise SystemExit(f"FAIL: authentic DM1 Amiga demo M12 route missed its first runtime: {probe}")
print("PASS: authentic DM1 Amiga demo M12 reaches its first Hall runtime frame")
PY

# The scanner also admits these original files in an ordinary directory.
# Exercise the same bytes without archive transport: compressed Motorola
# dungeon intake and startup companion reads must not depend on "::".
python3 - "$archive" "$scratch" <<'PY'
import pathlib
import sys
import zipfile

with zipfile.ZipFile(sys.argv[1]) as package:
    for name in ("graphics.dat", "DemoDun.dat", "swoosh"):
        pathlib.Path(sys.argv[2], name).write_bytes(package.read(name))
PY
for route in direct menu; do
    args=(--game dm1 --platform amiga --data-dir "$scratch" --debug)
    if [[ "$route" = direct ]]; then
        args+=(--boot-probe --boot-probe-frames 2 --duration 0)
    else
        args+=(--menu --script enter,enter,enter --duration 10000)
    fi
    if ! FIRESTAFF_CONFIG_PATH="$scratch/$route.toml" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" "${args[@]}" >"$scratch/$route.log" 2>&1; then
        cat "$scratch/$route.log" >&2
        exit 1
    fi
    # APFS and other case-insensitive filesystems may report the original
    # asset path as GRAPHICS.DAT even though this ZIP member is lowercase.
    # The path identity is the assertion; filesystem spelling is not.
    grep -Fiq "graphicsPath=$scratch/graphics.dat" "$scratch/$route.log"
    if [[ "$route" = direct ]]; then
        grep -Fq 'phase=dm1-runtime' "$scratch/$route.log"
        grep -Fq 'levelLoaded=1' "$scratch/$route.log"
    else
        grep -Fq 'phase=amiga-swsh' "$scratch/$route.log"
        grep -Fq 'phase=title-f0437' "$scratch/$route.log"
        grep -Fq 'phase=entrance-f0441' "$scratch/$route.log"
        grep -Fq 'DM1 READY: gameId=dm1' "$scratch/$route.log"
    fi
done
python3 - "$archive" "$scratch" <<'PY'
import pathlib
import sys
import zipfile

with zipfile.ZipFile(sys.argv[1]) as package:
    for name in ("graphics.dat", "DemoDun.dat", "swoosh"):
        if pathlib.Path(sys.argv[2], name).read_bytes() != package.read(name):
            raise SystemExit(f"FAIL: original loose media changed: {name}")
PY

archive_hash_after=$(firestaff_sha256_file "$archive")
if [[ "$archive_hash_before" != "$archive_hash_after" ]]; then
    printf '%s\n' 'FAIL: Firestaff modified the original DM1 Amiga demo archive' >&2
    exit 1
fi

# Exercise the regular no-game picker using an isolated per-user data root.
# This must reach the same source-owned SWSH/title/Entrance handoff as the
# explicitly selected M12 route above, then present the first Hall frame.
picker_home="$scratch/picker-home"
picker_data="$picker_home/.firestaff/data/dm1"
mkdir -p "$picker_data"
ln -s "$archive" "$picker_data/$(basename "$archive")"
picker_log="$scratch/picker-runtime.log"
picker_probe="$scratch/picker-runtime.json"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
    FIRESTAFF_CONFIG_PATH="$picker_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$picker_probe" \
    HOME="$picker_home" XDG_CONFIG_HOME="$picker_home" APPDATA="$picker_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait-game-runtime' \
        --duration 20000
) >"$picker_log" 2>&1 || {
    cat "$picker_log" >&2
    exit 1
}
expected_picker_source="$picker_data/$(basename "$archive")::graphics.dat"
if ! grep -Fq 'startup game=none mode=menu' "$picker_log" ||
   ! grep -Fq 'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no original-candidate=yes' "$picker_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Amiga edition=amiga-demo-en source=$expected_picker_source" "$picker_log";
then
    echo 'FAIL: no-game M12 picker did not retain the authentic DM1 Amiga demo media owner' >&2
    grep -E 'startup game=|scan complete|startup-source|DM1 READY|game-handoff|error|failed' "$picker_log" >&2 || true
    exit 1
fi
python3 - "$picker_probe" "$expected_graphics_md5" <<'PY'
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
        startup.get("dm1StartupSwshConsumed") != 1 or
        startup.get("dm1StartupTitleConsumed") != 1 or
        startup.get("dm1StartupEntranceConsumed") != 1 or
        startup.get("dm1StartupFullGraphicsConsumed") != 1 or
        startup.get("dm1StartupHoCFirstFrameReady") != 1 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 7, 1, 2, 0)):
    raise SystemExit(f"FAIL: DM1 Amiga demo no-game picker missed its first Hall frame: {probe}")
print("PASS: DM1 Amiga demo no-game picker reached its first Hall frame")
PY

archive_hash_final=$(firestaff_sha256_file "$archive")
if [[ "$archive_hash_before" != "$archive_hash_final" ]]; then
    printf '%s\n' 'FAIL: Firestaff modified the original DM1 Amiga demo archive during no-game startup' >&2
    exit 1
fi

printf 'PASS: authentic DM1 Amiga demo starts from archive and loose files via CLI and M12; graphics=%s dungeon=%s originals=read-only\n' \
    "$expected_graphics_md5" "$expected_dungeon_md5"
