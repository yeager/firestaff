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
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_DATA="$menu_data_root" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
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

run_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,click:700:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en mouse
run_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,click:1190:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en mouse
run_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,click:1680:262,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja mouse
