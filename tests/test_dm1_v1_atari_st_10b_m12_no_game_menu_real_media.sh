#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm1_v1_atari_st_10b_m12_no_game_menu_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_dm1_v1_atari_st_10b_m12_no_game_menu_real_media.sh <firestaff> <data-root>}
archive="$data_root/dm1/Game,Dungeon_Master,Atari_ST,Software.7z"
extractor=${FIRESTAFF_7ZZ:-7zz}
member='Floppy Disks STX/Dungeon Master for Atati ST v1.0 (English) 1987-12-11.stx'
expected_graphics_md5=7eee396993745e8af212f44d75ff6c1a

if [[ ! -x "$app" || ! -f "$archive" ]] || ! command -v "$extractor" >/dev/null 2>&1 || ! command -v zip >/dev/null 2>&1; then
    echo 'SKIP: authentic DM1 Atari ST 1.0b archive, 7zz, or zip is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/dm1-st10b-no-game-menu.XXXXXX")
cleanup() {
    rc=$?
    if [[ $rc != 0 && -f "$scratch/menu.log" ]]; then
        grep -E 'startup game=|scan complete|game-handoff|DM1 READY|FAIL:|error=' \
            "$scratch/menu.log" | tail -n 80 >&2 || true
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

stage="$scratch/stage"
menu_data="$scratch/data"
home="$scratch/home"
mkdir -p "$stage" "$menu_data/dm1" "$home"
archive_hash=$(firestaff_sha256_file "$archive")
stx="$stage/Dungeon Master Atari ST 1.0b.stx"
zip_path="$menu_data/dm1/Dungeon-Master_Atari-ST_EN.zip"
"$extractor" e -so "$archive" "$member" > "$stx" 2>/dev/null
if [[ ! -s "$stx" ]]; then
    echo 'FAIL: original DMWeb Atari ST 1.0b STX could not be extracted' >&2
    exit 1
fi
(
    cd "$stage"
    zip -q "$zip_path" "$(basename "$stx")"
)
repacked_hash=$(firestaff_sha256_file "$zip_path")
probe="$scratch/runtime.json"
log="$scratch/menu.log"

# The isolated data root contains only the authentic original 1.0b STX,
# repacked into ZIP for Firestaff's supported archive path. Exercise the
# production no-game picker through DM1 -> Atari ST -> first runtime.
env -u FIRESTAFF_ORIGINALS_DIR \
    FIRESTAFF_DATA="$menu_data" \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --data-dir "$menu_data" \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        --duration 8000 > "$log" 2>&1

expected_source="$menu_data/dm1/$(basename "$zip_path")::$(basename "$stx")::GRAPHICS.DAT"
if ! grep -Fq 'startup game=none mode=menu' "$log" ||
   ! grep -Fq 'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no' "$log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm1 platform=Atari ST edition=st10b-en source=$expected_source" "$log" ||
   ! grep -Fq 'DM1 READY: gameId=dm1' "$log" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' "$log"; then
    echo 'FAIL: authentic DM1 Atari ST 1.0b did not reach its no-game M12 runtime' >&2
    exit 1
fi

python3 - "$probe" "$expected_graphics_md5" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm1" or
        probe.get("bootAssetMd5") != sys.argv[2] or
        startup.get("receiptReady") != 1 or
        startup.get("phase") != "dm1-runtime" or
        startup.get("levelLoaded") != 1 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0} or
        party.get("mapIndex") != 0):
    raise SystemExit(f"FAIL: DM1 Atari ST 1.0b no-game route missed its authentic runtime: {probe}")
print("PASS: DM1 Atari ST 1.0b no-game M12 route reached its authentic first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ||
      $(firestaff_sha256_file "$zip_path") != "$repacked_hash" ]]; then
    echo 'FAIL: original or repacked DM1 Atari ST 1.0b data changed during startup' >&2
    exit 1
fi
echo 'PASS: authentic DM1 Atari ST 1.0b startup menu used the original DMWeb STX and preserved its input data'
