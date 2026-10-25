#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}

dm1_archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
csb_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm2_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
if [[ ! -x "$app" || ! -f "$dm1_archive" || ! -f "$csb_archive" || ! -f "$dm2_archive" ]]; then
    echo 'SKIP: authentic DM1, CSB and DM2 FM Towns archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-main-game-picker.XXXXXX")
scan_cache_home="$scratch/shared-scan-cache-home"
mkdir -p "$scan_cache_home"
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

run_game_picker_case() {
    local game_id=$1
    local game_archive=$2
    local navigation=$3
    local edition=$4
    local input_method=${5:-keyboard}
    local platform=${6:-FM Towns}
    local menu_data_root=${7:-$data_root}
    local home="$scratch/$game_id-$input_method-home"
    local log="$scratch/$game_id-$input_method.log"
    local source_hash

    mkdir -p "$home"
    source_hash=$(firestaff_sha256_file "$game_archive")
    # Keep each config isolated while sharing the identity-bound asset cache
    # for the same original-media paths across the picker matrix.
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_DATA="$menu_data_root" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$scan_cache_home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 1920 --height 1080 \
        --data-dir "$menu_data_root" \
        --script "$navigation" --duration 90000 >"$log" 2>&1 || {
            cat "$log" >&2
            return 1
        }

    if ! grep -Fq 'startup game=none mode=menu' "$log"; then
        printf 'FAIL: %s session did not start at the no-game main menu\n' "$game_id" >&2
        cat "$log" >&2
        return 1
    fi
    if ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition source=$game_archive" "$log"; then
        printf 'FAIL: no-game main-menu selection did not launch authentic %s %s media from %s\n' "$game_id" "$platform" "$game_archive" >&2
        cat "$log" >&2
        return 1
    fi
    if [[ $(firestaff_sha256_file "$game_archive") != "$source_hash" ]]; then
        printf 'FAIL: original %s source changed during menu launch\n' "$game_id" >&2
        return 1
    fi
    printf 'PASS: no-game M12 main menu %s selected %s and handed off its authentic %s %s edition\n' \
        "$input_method" "$game_id" "$platform" "$edition"
}

run_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
run_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
run_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja

# Users may select one per-game folder as the collection root. The launcher
# must still discover sibling game folders instead of limiting the menu to
# the selected folder. Keep the source files in their documented layout and
# launch an authentic DM2 archive from the sibling directory.
run_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-ja keyboard-leaf 'FM Towns' "$data_root/dm1"
run_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-en keyboard-leaf 'FM Towns' "$data_root/csb"
run_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-en keyboard-leaf 'FM Towns' "$data_root/dm2"

# Exercise mouse hit testing on the first row as well as keyboard selection.
# These clicks target the centers of the DM1, CSB, and DM2 cards in the
# 1920x1080 main menu, then continue through the normal platform/presentation
# choices to an authenticated handoff.
run_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,click:701:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-en mouse
run_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,click:1173:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-en mouse
run_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,click:1645:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
    fmtowns-ja mouse

# The documented per-user collection must open the all-games launcher and
# launch each supported priority game when neither --game nor --data-dir is
# supplied. Each case gets an isolated HOME/config while reusing the authentic
# collection, so a saved selection or copied game data cannot mask discovery.
run_default_root_game_picker_case() {
    local game_id=$1
    local game_archive=$2
    local navigation=$3
    local edition=$4
    local home="$scratch/default-root-$game_id-home"
    local data_home="$scratch/default-data-home"
    local log="$scratch/default-root-$game_id.log"
    local graphics_member='DATA/GRAPHICS.DAT'
    local source_path
    local source_hash

    if [[ "$game_id" == csb ]]; then
        graphics_member='CDATA/GRAPHICS.DAT'
    fi
    source_path="$data_home/.firestaff/data/$game_id/$(basename "$game_archive")::$graphics_member"
    source_hash=$(firestaff_sha256_file "$game_archive")
    mkdir -p "$home" "$data_home/.firestaff"
    if [[ ! -e "$data_home/.firestaff/data" ]]; then
        ln -s "$data_root" "$data_home/.firestaff/data"
    fi
    (
        unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
        FIRESTAFF_CONFIG_PATH="$home/config.toml" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        HOME="$data_home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
            "$app" --debug --width 1920 --height 1080 \
            --script "$navigation" --duration 90000
    ) >"$log" 2>&1 || {
        cat "$log" >&2
        return 1
    }
    if ! grep -Fq 'startup game=none mode=menu' "$log" ||
       ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=FM Towns edition=$edition source=$source_path" "$log"; then
        printf 'FAIL: default .firestaff/data menu did not launch authentic %s FM Towns media\n' "$game_id" >&2
        cat "$log" >&2
        return 1
    fi
    if [[ $(firestaff_sha256_file "$game_archive") != "$source_hash" ]]; then
        printf 'FAIL: default .firestaff/data menu changed the original %s archive\n' "$game_id" >&2
        return 1
    fi
    printf 'PASS: default .firestaff/data menu launched authentic %s FM Towns\n' "$game_id"
}

run_default_root_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
run_default_root_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
run_default_root_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja

# Nexus and Theron use their original loose media in this data root. These
# cases exercise the same five-game launcher selector without requiring the
# user's separate archive tools or extracting/copying game data.
nexus_source="$data_root/nexus/DM.BIN"
theron_source="$data_root/theron/TQUS02.bin"
if [[ -f "$nexus_source" ]]; then
    run_game_picker_case nexus "$nexus_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        nexus-saturn-jp keyboard Saturn
else
    echo 'SKIP: authentic Nexus Saturn DM.BIN is unavailable'
fi
if [[ -f "$theron_source" ]]; then
    run_game_picker_case theron "$theron_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en keyboard 'PC Engine'
else
    echo 'SKIP: authentic Theron PC Engine TQUS02.bin is unavailable'
fi

# Exercise mouse hit testing for all five game cards. Nexus and Theron occupy
# the second row in the 3-by-2 game-card grid; keyboard-only coverage would
# miss a broken mouse rectangle or a mismatch between the drawn and clickable
# card positions.
if [[ -f "$nexus_source" ]]; then
    run_game_picker_case nexus "$nexus_source" \
        'wait-scan-complete,wait20,click:700:728,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        nexus-saturn-jp mouse Saturn
fi
if [[ -f "$theron_source" ]]; then
    run_game_picker_case theron "$theron_source" \
        'wait-scan-complete,wait20,click:1190:728,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en mouse 'PC Engine'
fi

run_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,click:700:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en mouse
run_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,click:1190:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en mouse
run_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,click:1680:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja mouse
