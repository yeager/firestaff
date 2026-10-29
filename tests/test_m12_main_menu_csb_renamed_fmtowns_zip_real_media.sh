#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_csb_renamed_fmtowns_zip_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_csb_renamed_fmtowns_zip_real_media.sh <firestaff> <data-root>}
source_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}

if [[ ! -x "$app" ]]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi
if [[ ! -f "$source_archive" ]]; then
    echo "SKIP: authentic CSB FM Towns ZIP is unavailable: $source_archive"
    exit 77
fi

mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-renamed-fmtowns.XXXXXX")
cleanup() {
    local status=$?
    trap - EXIT HUP INT TERM
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
    exit "$status"
}
trap cleanup EXIT HUP INT TERM

source_hash=$(firestaff_sha256_file "$source_archive")
unset FIRESTAFF_ORIGINALS_DIR FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
menu_data_root="$scratch/menu-data"
menu_csb_dir="$menu_data_root/csb"
renamed_archive="$menu_csb_dir/owned-retail-disc.zip"
home="$scratch/home"
mkdir -p "$menu_csb_dir" "$home"
if ! ln -s "$source_archive" "$renamed_archive" 2>/dev/null; then
    ln "$source_archive" "$renamed_archive"
fi

log="$scratch/menu.log"
FIRESTAFF_CONFIG_PATH="$home/config.toml" \
FIRESTAFF_DATA="$menu_data_root" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --width 1920 --height 1080 \
    --data-dir "$menu_data_root" \
    --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    --duration 90000 >"$log" 2>&1 || {
        cat "$log" >&2
        exit 1
    }

if ! grep -Fq 'startup game=none mode=menu' "$log"; then
    echo 'FAIL: renamed CSB ZIP route did not start in the no-game M12 menu' >&2
    cat "$log" >&2
    exit 1
fi
if ! grep -Fq \
    "launch phase=game-handoff mode=menu game=csb platform=FM Towns edition=fmtowns-en source=$renamed_archive::" "$log"; then
    echo 'FAIL: no-game M12 menu did not select and hand off the renamed authentic CSB ZIP' >&2
    cat "$log" >&2
    exit 1
fi
if [[ $(firestaff_sha256_file "$source_archive") != "$source_hash" ]]; then
    echo 'FAIL: CSB original source archive changed during renamed-ZIP discovery' >&2
    exit 1
fi

echo 'PASS: no-game M12 discovers renamed authentic CSB FM Towns ZIP and hands off the selected archive'
