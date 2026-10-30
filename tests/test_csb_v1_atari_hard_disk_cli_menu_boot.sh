#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/portable_sha256.sh"

app=${1:?usage: test_csb_v1_atari_hard_disk_cli_menu_boot.sh <firestaff>}
archive=${FIRESTAFF_CSB_ATARI_ST_ARCHIVE:-"$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic CSB Atari ST archive or Firestaff executable is unavailable'
    exit 77
fi
if ! command -v 7z >/dev/null 2>&1; then
    echo 'SKIP: 7z is required to extract the authentic CSB Atari hard-disk profile'
    exit 77
fi
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-atari-hard-disk.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        python3 - "$scratch_root" "$scratch" <<'PY'
import pathlib
import shutil
import sys

root = pathlib.Path(sys.argv[1]).resolve()
target = pathlib.Path(sys.argv[2]).resolve()
if target.parent == root and target.name.startswith("csb-atari-hard-disk."):
        shutil.rmtree(target, ignore_errors=True)
PY
    fi
}
trap cleanup EXIT HUP INT TERM

archive_hash=$(firestaff_sha256_file "$archive")
extraction_root="$scratch/original-media"
media_dir="$extraction_root/HardDisk/2009-02-22 PP"
data_root="$scratch/data"
mkdir -p "$extraction_root" "$data_root/csb"
7z x -y -o"$extraction_root" "$archive" 'HardDisk/2009-02-22 PP/*' >/dev/null
for required in GRAPHICS.DAT DUNGEON.DAT FTLCODE ANIMATE.DAT ANIMATE.SCR; do
    if [[ ! -f "$media_dir/$required" ]]; then
        echo "FAIL: authentic Atari hard-disk source is missing $required" >&2
        exit 1
    fi
done
python3 - "$media_dir/GRAPHICS.DAT" "$media_dir/DUNGEON.DAT" <<'PY'
import hashlib
import pathlib
import sys

expected = {
    "GRAPHICS.DAT": "e0ce7ac5160ca5540e90cf09ab9fad49",
    "DUNGEON.DAT": "6695d2acebce49f95db1d8f3a5c733de",
}
for filename in sys.argv[1:]:
    path = pathlib.Path(filename)
    digest = hashlib.md5(path.read_bytes()).hexdigest()
    if digest != expected[path.name]:
        raise SystemExit(
            f"FAIL: extracted original {path.name} has unexpected MD5 {digest}"
        )
PY
# Link the extracted original files into an isolated install tree so the test
# cannot see the floppy edition or any other game in the user's collection.
python3 - "$media_dir" "$data_root/csb" <<'PY'
import pathlib
import sys

source = pathlib.Path(sys.argv[1])
target = pathlib.Path(sys.argv[2])
for child in source.iterdir():
    (target / child.name).symlink_to(child, target_is_directory=child.is_dir())
PY

mkdir -p "$scratch/home"

direct_log="$scratch/direct.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_CSB_ATARI_ST_ARCHIVE
    FIRESTAFF_CONFIG_PATH="$scratch/direct.toml" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --game csb --platform atari-st --data-dir "$data_root" \
        --boot-probe --boot-probe-frames 2 --duration 0
) >"$direct_log" 2>&1 || {
    cat "$direct_log" >&2
    exit 1
}
for required in \
    'startup game=csb mode=direct platform=Atari ST' \
    'selected game=csb platform=Atari ST edition=st20-21-hd-en' \
    'sourceId=csb assetMd5=e0ce7ac5160ca5540e90cf09ab9fad49' \
    'variant=csb-st20-21-en' \
    'phase=csb-atari-st-animation' \
    'startupAnimation=animate-scr' \
    'titleReady=1 levelLoaded=0'; do
    if ! grep -Fq "$required" "$direct_log"; then
        printf 'FAIL: authentic CSB Atari hard-disk CLI route is missing %s\n' \
            "$required" >&2
        cat "$direct_log" >&2
        exit 1
    fi
done

runtime_log="$scratch/runtime.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_CSB_ATARI_ST_ARCHIVE
    FIRESTAFF_CONFIG_PATH="$scratch/runtime.toml" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --game csb --platform atari-st --data-dir "$data_root" \
        --boot-probe --boot-probe-frames 180 --script enter \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0
) >"$runtime_log" 2>&1 || {
    cat "$runtime_log" >&2
    exit 1
}
case "$(cat "$runtime_log")" in
    *sourceId=csb*assetMd5=e0ce7ac5160ca5540e90cf09ab9fad49*phase=inactive*startupActive=0*levelLoaded=1*map=0*party=9,0,2*runtimeTick=*csbViewportHash=*) ;;
    *)
        echo 'FAIL: authentic Atari hard-disk title Enter did not reach CSB map 0' >&2
        cat "$runtime_log" >&2
        exit 1
        ;;
esac
if ! grep -Eq 'csbViewportHash=[1-9][0-9]*' "$runtime_log"; then
    echo 'FAIL: authentic Atari hard-disk runtime did not render a nonempty CSB viewport' >&2
    cat "$runtime_log" >&2
    exit 1
fi

menu_log="$scratch/menu.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_CSB_ATARI_ST_ARCHIVE
    FIRESTAFF_CONFIG_PATH="$scratch/menu.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$data_root" \
        --script \
        'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' \
        --duration 60000
) >"$menu_log" 2>&1 || {
    cat "$menu_log" >&2
    exit 1
}
expected_source="$data_root/csb/GRAPHICS.DAT"
if [[ $(grep -Fc 'launch phase=game-handoff' "$menu_log") != 1 ]] ||
   ! grep -Fq 'startup game=none mode=menu' "$menu_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Atari ST edition=st20-21-hd-en source=$expected_source" "$menu_log" ||
   ! grep -Fq 'variant=csb-st20-21-en' "$menu_log"; then
    echo 'FAIL: no-game M12 did not select the authentic CSB Atari hard-disk profile exactly once' >&2
    cat "$menu_log" >&2
    exit 1
fi

# A launcher handoff alone is not proof that the selected title completed its
# native startup. Repeat the no-game M12 route without EXIT_AFTER_LAUNCH and
# require the Atari HD source to load its first playable map and render a real
# viewport, as the direct CLI route above already does.
menu_runtime_log="$scratch/menu-runtime.log"
menu_runtime_probe="$scratch/menu-runtime.json"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_CSB_ATARI_ST_ARCHIVE
    FIRESTAFF_CONFIG_PATH="$scratch/menu-runtime.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe" \
    HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$data_root" \
        --script \
        'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240,wait-game-runtime' \
        --duration 60000
) >"$menu_runtime_log" 2>&1 || {
    cat "$menu_runtime_log" >&2
    exit 1
}
if ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Atari ST edition=st20-21-hd-en source=$expected_source" "$menu_runtime_log" ||
   ! grep -Fq 'CSB READY: gameId=csb' "$menu_runtime_log" ||
   ! grep -Fq 'md5=e0ce7ac5160ca5540e90cf09ab9fad49' "$menu_runtime_log" ||
   ! grep -Fq 'level-loaded=1 map=0 party=9,0 dir=2 champions=0' "$menu_runtime_log"; then
    echo 'FAIL: authentic CSB Atari hard-disk M12 runtime route lost its selected media owner' >&2
    cat "$menu_runtime_log" >&2
    exit 1
fi
python3 - "$menu_runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        probe.get("bootAssetMd5") != "e0ce7ac5160ca5540e90cf09ab9fad49" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "inactive" or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0) or
        not probe.get("csbViewportHash")):
    raise SystemExit(
        f"FAIL: CSB Atari hard-disk M12 menu missed its original runtime: {probe}")
print("PASS: CSB Atari hard-disk no-game M12 menu reached its authenticated first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB Atari ST archive changed during hard-disk startup checks' >&2
    exit 1
fi
echo 'PASS: authentic CSB Atari hard-disk media reaches runtime through direct CLI and the no-game M12 menu'
