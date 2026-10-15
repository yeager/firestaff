#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"
if [[ $# -ne 3 ]]; then
    printf 'usage: %s <firestaff> <original-Atari-GRAPHICS.DAT> <original-Atari-DUNGEON.DAT>\n' "$0" >&2
    exit 2
fi
app=$1
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
original_graphics=$2
original_dungeon=$3
if [[ ! -x "$app" || ! -f "$original_graphics" || ! -f "$original_dungeon" ]] ||
    ! command -v python3 >/dev/null 2>&1; then
    echo 'SKIP: Firestaff or authentic Atari v1.2 loose media is unavailable'
    exit 77
fi
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-atari-loose-case.XXXXXX")
cleanup() {
    rc=$?
    if [[ -d "$scratch/inventory/dm2" ]]; then chmod 700 "$scratch/inventory/dm2"; fi
    if [[ $rc != 0 ]]; then
        for log in "$scratch"/*.log; do [[ ! -f "$log" ]] || cat "$log" >&2; done
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT
: >"$scratch/Case"
if [[ -e "$scratch/case" ]]; then
    rm "$scratch/Case"
    echo 'SKIP: original Atari filename regression requires case-sensitive scratch storage'
    exit 77
fi
rm "$scratch/Case"
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS FIRESTAFF_EXIT_AFTER_LAUNCH
original_graphics_sha=$(firestaff_sha256_file "$original_graphics")
original_dungeon_sha=$(firestaff_sha256_file "$original_dungeon")
mkdir -p "$scratch/media"
python3 - "$original_graphics" "$original_dungeon" "$scratch/media" <<'PY'
import hashlib
import pathlib
import sys

for source, name, size, expected in (
        (sys.argv[1], "graphics.dat", 271918, "9ce2eaf7a9e78620e3f17594437caffa"),
        (sys.argv[2], "DUNGEON.DAT", 33442, "050fb2cfded1b502ec2c53956b94c5bd")):
    data = pathlib.Path(source).read_bytes()
    if len(data) != size or hashlib.md5(data).hexdigest() != expected:
        raise SystemExit(f"FAIL: original Atari identity mismatch: {name}")
    target = pathlib.Path(sys.argv[3], name)
    target.write_bytes(data)
    if target.read_bytes() != data:
        raise SystemExit(f"FAIL: staging changed original Atari bytes: {name}")
PY
run_cli() {
    local route=$1
    local selected=${2:-"$scratch/media"}
    mkdir -p "$scratch/$route-home"
    HOME="$scratch/$route-home" XDG_CONFIG_HOME="$scratch/$route-home" \
    APPDATA="$scratch/$route-home" FIRESTAFF_CONFIG_PATH="$scratch/$route-home/config.toml" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --width 320 --height 200 --game dm1 --platform atari-st \
        --data-dir "$selected" --debug --boot-probe --boot-probe-frames 2 \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-asset-md5 9ce2eaf7a9e78620e3f17594437caffa \
        --duration 0 >"$scratch/$route.log" 2>&1
}
for route in uppercase lowercase; do
    if [[ "$route" = lowercase ]]; then
        mv "$scratch/media/DUNGEON.DAT" "$scratch/media/dungeon.dat"
    fi
    if ! run_cli "$route"; then
        echo "FAIL: unchanged original Atari $route dungeon did not boot" >&2
        exit 1
    fi
    grep -Fq "source=$scratch/media/graphics.dat" "$scratch/$route.log"
    grep -Fq 'phase=dm1-runtime' "$scratch/$route.log"
    grep -Fq 'levelLoaded=1' "$scratch/$route.log"
    grep -Fq 'handoff=atari-st-dmcsb1' "$scratch/$route.log"
done
mkdir -p "$scratch/duplicates/z-complete"
cp "$scratch/media/graphics.dat" "$scratch/duplicates/graphics.dat"
cp "$scratch/media/"* "$scratch/duplicates/z-complete/"
run_cli orphan-first "$scratch/duplicates"
grep -Fq "source=$scratch/duplicates/z-complete/graphics.dat" "$scratch/orphan-first.log"
if run_cli selected-orphan "$scratch/duplicates/graphics.dat"; then
    echo 'FAIL: explicit Atari orphan borrowed neighboring complete media' >&2; exit 1
fi
if grep -Fq 'levelLoaded=1' "$scratch/selected-orphan.log"; then exit 1; fi
echo 'PASS: Atari duplicate inventory selects complete same-parent original pair'
mkdir -p "$scratch/menu-home"
# Existing Atari M12 handoff smoke route: the three menu commands select the
# game/card/options, then the test exits after successful source admission.
if ! HOME="$scratch/menu-home" XDG_CONFIG_HOME="$scratch/menu-home" \
    APPDATA="$scratch/menu-home" FIRESTAFF_CONFIG_PATH="$scratch/menu-home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game dm1 --platform atari-st \
    --data-dir "$scratch/media" --debug --script enter,enter,enter --duration 1000 \
    >"$scratch/menu.log" 2>&1; then
    echo 'FAIL: M12 did not admit unchanged lowercase Atari dungeon' >&2
    exit 1
fi
grep -Fq 'DM1 READY: gameId=dm1' "$scratch/menu.log"
grep -Fq 'handoff=atari-st-dmcsb1' "$scratch/menu.log"
grep -Fq "graphicsPath=$scratch/media/graphics.dat" "$scratch/menu.log"
# A denied incidental DM2 directory must not clear authenticated DM1 data.
mkdir -p "$scratch/inventory/dm1" "$scratch/inventory/dm2" "$scratch/scan-home"
cp "$scratch/media/graphics.dat" "$scratch/inventory/dm1/graphics.dat"
cp "$scratch/media/dungeon.dat" "$scratch/inventory/dm1/dungeon.dat"
chmod 111 "$scratch/inventory/dm2"
if python3 - "$scratch/inventory/dm2" <<'PYDENIED'
import os
import sys
try:
    os.listdir(sys.argv[1])
except PermissionError:
    raise SystemExit(0)
raise SystemExit(1)
PYDENIED
then
    HOME="$scratch/scan-home" XDG_CONFIG_HOME="$scratch/scan-home" \
    APPDATA="$scratch/scan-home" FIRESTAFF_CONFIG_PATH="$scratch/scan-home/config.toml" \
    "$app" --scan-data --data-dir "$scratch/inventory" --debug \
        >"$scratch/denied-scan.log" 2>&1
    grep -Fq 'media-discovery-incomplete game=dm2' "$scratch/denied-scan.log"
    grep -Fq 'reason=directory-io' "$scratch/denied-scan.log"
    grep -Eq '^Dungeon Master[[:space:]]+READY$' "$scratch/denied-scan.log"
    echo 'PASS: denied DM2 directory preserves authenticated DM1 scan inventory'
else
    echo 'SKIP: current account can read permission-restricted directories'
fi
chmod 700 "$scratch/inventory/dm2"
# Continue beyond admission with no --game or --platform override. The normal
# Atari title and entrance consume their source inputs before the first Hall
# frame, so this must not use FIRESTAFF_EXIT_AFTER_LAUNCH.
mkdir -p "$scratch/unscoped-home"
unscoped_probe="$scratch/unscoped-runtime.json"
if ! HOME="$scratch/unscoped-home" XDG_CONFIG_HOME="$scratch/unscoped-home" \
    APPDATA="$scratch/unscoped-home" \
    FIRESTAFF_CONFIG_PATH="$scratch/unscoped-home/config.toml" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$unscoped_probe" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --data-dir "$scratch/inventory" \
    --debug --script enter,enter,enter,wait30,enter,wait60,enter --duration 15000 \
    >"$scratch/unscoped.log" 2>&1; then
    echo 'FAIL: unscoped M12 did not continue through authentic Atari startup' >&2
    exit 1
fi
grep -Fq "mode=menu game=dm1 platform=Atari ST edition=st12-en source=$scratch/inventory/dm1/graphics.dat" \
    "$scratch/unscoped.log"
python3 - "$unscoped_probe" <<'PYRUNTIME'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe["startup"]
party = probe["party"]
placement = startup["dm1StartupPartyPlacement"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or
        probe["bootAssetMd5"] != "9ce2eaf7a9e78620e3f17594437caffa" or
        startup["receiptReady"] != 1 or startup["phase"] != "dm1-runtime" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupTitleConsumed"] != 1 or
        startup["dm1StartupEntranceConsumed"] != 1 or
        startup["dm1StartupFullGraphicsConsumed"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        placement["executed"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0) or
        probe["gameTick"] <= 0 or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: unscoped original Atari startup did not reach continued runtime: {probe}")
print("PASS: unscoped M12 AUTO consumes original Atari title/entrance and continues in the first Hall")
PYRUNTIME
# Even identical original payloads cannot disambiguate two native filenames.
cp "$scratch/media/dungeon.dat" "$scratch/media/DUNGEON.DAT"
if run_cli ambiguous; then
    echo 'FAIL: ambiguous Atari dungeon filenames were admitted' >&2
    exit 1
fi
if grep -Eq 'FIRESTAFF BOOT PROBE READY|DM1 READY:|launch phase=runtime-entered' \
    "$scratch/ambiguous.log"; then
    echo 'FAIL: ambiguous Atari dungeon used a runtime fallback' >&2
    exit 1
fi
if [[ "$original_graphics_sha" != "$(firestaff_sha256_file "$original_graphics")" ||
      "$original_dungeon_sha" != "$(firestaff_sha256_file "$original_dungeon")" ]]; then
    echo 'FAIL: original Atari input bytes changed during filename tests' >&2
    exit 1
fi
echo 'PASS: authentic Atari loose dungeon boots with unique upper/lower case and rejects ambiguity'
