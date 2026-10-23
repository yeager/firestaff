#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_dm1_only_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_dm1_only_real_media.sh <firestaff> <data-root>}
archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic DM1 FM Towns archive is required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-dm1-only.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

home="$scratch/home"
dm1_root="$home/.firestaff/data/dm1"
mkdir -p "$dm1_root"
ln -s "$archive" "$dm1_root/$(basename "$archive")"
archive_hash=$(firestaff_sha256_file "$archive")
log="$scratch/firestaff.log"

# No --game or --data-dir: discover the sole installed original from the
# documented per-user data tree, then select it from the no-game menu.
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 \
        --script 'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        --duration 60000
) >"$log" 2>&1 || {
    grep -E 'startup game=|scan complete|game-handoff|error|failed' "$log" >&2 || true
    exit 1
}

expected_source="$dm1_root/$(basename "$archive")::DATA/GRAPHICS.DAT"
for required in \
    'startup game=none mode=menu' \
    'games=dm1:yes,csb:no,dm2:no,nexus:no,theron:no original-candidate=yes' \
    "launch phase=game-handoff mode=menu game=dm1 platform=FM Towns edition=fmtowns-en source=$expected_source"; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: DM1-only no-game menu is missing %s\n' "$required" >&2
        grep -E 'startup game=|scan complete|game-handoff|error|failed' "$log" >&2 || true
        exit 1
    fi
done

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original DM1 archive changed during the M12 menu launch' >&2
    exit 1
fi
echo 'PASS: default .firestaff/data/dm1 discovery launched authentic DM1 FM Towns from the no-game menu'
