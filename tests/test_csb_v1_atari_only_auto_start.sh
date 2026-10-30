#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "${BASH_SOURCE[0]}")/lib/portable_sha256.sh"

app=${1:?usage: test_csb_v1_atari_only_auto_start.sh <firestaff>}
archive=${FIRESTAFF_CSB_ATARI_ST_ARCHIVE:-"$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic CSB Atari ST archive or Firestaff executable is unavailable'
    exit 77
fi
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-atari-only-auto.XXXXXX")
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
if target.parent == root and target.name.startswith("csb-atari-only-auto."):
    shutil.rmtree(target)
PY
    fi
}
trap cleanup EXIT HUP INT TERM

archive_hash=$(firestaff_sha256_file "$archive")
data_root="$scratch/data"
home="$scratch/home"
mkdir -p "$data_root/csb" "$home"
ln -s "$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)" "$data_root/csb/$(basename "$archive")"

# Keep the data root isolated: the Atari archive is the only CSB edition
# available to AUTO discovery in either route.
direct_log="$scratch/direct.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CSB_ATARI_ST_ARCHIVE="$archive" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --game csb --data-dir "$data_root" \
        --boot-probe --boot-probe-frames 1 --duration 0
) >"$direct_log" 2>&1 || {
    cat "$direct_log" >&2
    exit 1
}
for required in \
    'startup game=csb mode=direct platform=auto' \
    'selected game=csb platform=Atari ST edition=st20-21-en' \
    'CSB READY: gameId=csb' \
    'variant=csb-st20-21-en' \
    'handoff=atari-st-animate-ftlcode' \
    'phase=csb-atari-st-animation'; do
    if ! grep -Fq "$required" "$direct_log"; then
        printf 'FAIL: CSB bare CLI AUTO route is missing %s\n' "$required" >&2
        cat "$direct_log" >&2
        exit 1
    fi
done

menu_log="$scratch/menu.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CSB_ATARI_ST_ARCHIVE="$archive" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$data_root" \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' \
        --duration 60000
) >"$menu_log" 2>&1 || {
    cat "$menu_log" >&2
    exit 1
}
expected_source="$data_root/csb/$(basename "$archive")::Floppy Disks ST/Chaos Strikes Back for Atari ST v2.0 (English) Cracked (Automation Disk 192).st::GRAPHICS.DAT"
if [[ $(grep -Fc 'launch phase=game-handoff' "$menu_log") != 1 ]] ||
   ! grep -Fq 'startup game=none mode=menu' "$menu_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Atari ST edition=st20-21-en source=$expected_source" "$menu_log" ||
   ! grep -Fq 'handoff=atari-st-animate-ftlcode' "$menu_log"; then
    echo 'FAIL: isolated CSB-only M12 AUTO route did not launch authentic Atari ST media exactly once' >&2
    cat "$menu_log" >&2
    exit 1
fi

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original CSB Atari ST archive changed during AUTO startup checks' >&2
    exit 1
fi
echo 'PASS: CSB bare CLI AUTO and no-game M12 select authentic Atari ST media when it is the only installed edition'
