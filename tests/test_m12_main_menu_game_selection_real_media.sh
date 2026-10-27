#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}

dm1_archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
csb_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm2_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
if [[ ! -x "$app" ]]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 && ( ! -f "$dm1_archive" || ! -f "$csb_archive" || ! -f "$dm2_archive" ) ]]; then
    echo 'SKIP: authentic DM1, CSB and DM2 FM Towns archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-main-game-picker.XXXXXX")
archive_data_root="$scratch/archive-only-data"
declare -a original_archives=()
declare -a original_archive_hashes=()

# The normal data root may contain extracted editions beside original
# archives. Build a symlink-only view of every top-level ZIP so picker
# ordering and edition availability stay intact without exposing extracted
# trees. Snapshot each original archive so the test verifies it stays intact.
for game_id in dm1 csb dm2; do
    source_game_dir="$data_root/$game_id"
    overlay_game_dir="$archive_data_root/$game_id"
    mkdir -p "$overlay_game_dir"
    if [[ ! -d "$source_game_dir" ]]; then
        continue
    fi
    while IFS= read -r -d '' archive; do
        ln -s "$archive" "$overlay_game_dir/$(basename "$archive")"
        original_archives+=("$archive")
        original_archive_hashes+=("$(firestaff_sha256_file "$archive")")
    done < <(find "$source_game_dir" -maxdepth 1 -type f -iname '*.zip' -print0)
done

verify_original_archives_unchanged() {
    local archive
    local index=0
    for archive in "${original_archives[@]}"; do
        if [[ $(firestaff_sha256_file "$archive") != "${original_archive_hashes[$index]}" ]]; then
            printf 'FAIL: original game archive changed during picker test: %s\n' "$archive" >&2
            return 1
        fi
        index=$((index + 1))
    done
}

cleanup() {
    local test_status=$?
    trap - EXIT HUP INT TERM
    if ! verify_original_archives_unchanged; then
        test_status=1
    fi
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
    exit "$test_status"
}
trap cleanup EXIT HUP INT TERM

if [[ ${#original_archives[@]} == 0 ]]; then
    echo 'SKIP: no authentic top-level ZIP archives are available for DM1, CSB or DM2'
    exit 77
fi

run_game_picker_case() {
    local game_id=$1
    local game_archive=$2
    local navigation=$3
    local edition=$4
    local input_method=${5:-keyboard}
    local platform=${6:-FM Towns}
    local menu_data_root=${7:-$data_root}
    local case_id=${8:-$game_id-$input_method}
    local source_mode=${9:-exact}
    local edition_pattern=$edition
    local edition_label=$edition
    local home="$scratch/$case_id-home"
    local log="$scratch/$case_id.log"
    local source_hash

    mkdir -p "$home"
    source_hash=$(firestaff_sha256_file "$game_archive")
    # Keep both configuration and HOME isolated so previous cases cannot
    # supply stale scanner or extracted-asset state.
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
    if [[ "$source_mode" == exact ]]; then
        if ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition source=$game_archive" "$log"; then
            printf 'FAIL: no-game main-menu selection did not launch authentic %s %s media from %s\n' "$game_id" "$platform" "$game_archive" >&2
            cat "$log" >&2
            return 1
        fi
    elif [[ "$source_mode" == resolved ]]; then
        if [[ "$edition_pattern" == re:* ]]; then
            edition_pattern=${edition_pattern#re:}
            edition_label=$edition_pattern
        else
            edition_pattern=$(printf '%s' "$edition_pattern" | sed 's/[][(){}.^$+?|\\]/\\&/g')
        fi
        local matching_source=0
        local launch_line
        local logged_source
        local logged_archive
        local canonical_archive
        canonical_archive=$(python3 - "$game_archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)
        while IFS= read -r launch_line; do
            logged_source=${launch_line#* source=}
            logged_archive=${logged_source%%::*}
            if [[ ( "$logged_archive" == "$game_archive" ||
                    "$logged_archive" == "$canonical_archive" ) &&
                  -f "$logged_archive" &&
                  $(python3 - "$logged_archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
                  ) == "$canonical_archive" ]]; then
                matching_source=1
                break
            fi
        done < <(grep -E "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition_pattern[[:space:]]+source=.+$" "$log" || true)
        if [[ $matching_source != 1 ]]; then
            printf 'FAIL: no-game main-menu selection did not resolve authentic %s %s media (%s)\n' "$game_id" "$platform" "$edition" >&2
            cat "$log" >&2
            return 1
        fi
    else
        printf 'FAIL: unsupported source assertion mode: %s\n' "$source_mode" >&2
        return 2
    fi
    if [[ $(firestaff_sha256_file "$game_archive") != "$source_hash" ]]; then
        printf 'FAIL: original %s source changed during menu launch\n' "$game_id" >&2
        return 1
    fi
    printf 'PASS: no-game M12 main menu %s selected %s and handed off its authentic %s %s edition\n' \
        "$input_method" "$game_id" "$platform" "$edition_label"
}

run_alternate_platform_picker_case() {
    local game_id=$1
    local source_archive=$2
    local down_count=$3
    local platform=$4
    local edition=$5
    local navigation='wait-scan-complete,wait20'
    local game_card_index=0
    local index
    local overlay_archive
    overlay_archive="$archive_data_root/$game_id/$(basename "$source_archive")"
    if [[ ! -f "$overlay_archive" ]]; then
        printf 'FAIL: authentic archive is not present in the archive-only picker view: %s\n' \
            "$source_archive" >&2
        return 1
    fi
    case "$game_id" in
        dm1) game_card_index=0 ;;
        csb) game_card_index=1 ;;
        dm2) game_card_index=2 ;;
    esac
    for ((index = 0; index < game_card_index; ++index)); do
        navigation+=",key:down,wait20"
    done
    navigation+=",key:enter,wait20"
    for ((index = 0; index < down_count; ++index)); do
        navigation+=",key:down,wait20"
    done
    navigation+=",key:enter,wait20,key:enter,wait20,key:enter"
    run_game_picker_case "$game_id" "$overlay_archive" "$navigation" \
        "$edition" keyboard "$platform" "$archive_data_root" \
        "$game_id-$platform-picker" resolved
    alternate_platform_cases=$((alternate_platform_cases + 1))
}

alternate_platform_cases=0

if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 ]]; then
    run_game_picker_case dm1 "$dm1_archive" \
        'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
    run_game_picker_case csb "$csb_archive" \
        'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en
    run_game_picker_case dm2 "$dm2_archive" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja
fi

# Exercise alternate-platform selection from the no-game main menu for DM1,
# CSB, and DM2 with authentic, independently verified media. The first three
# game cards are DM1/CSB/DM2 respectively; these cases select the
# platform card, original presentation, then launch. Source archives are
# hashed before and after by run_game_picker_case.
if [[ -f "$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip" ]]; then
    run_alternate_platform_picker_case dm1 \
        "$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip" 1 PC pc34-en
fi
if [[ -f "$data_root/dm1/Dungeon-Master_Atari-ST_EN.zip" ]]; then
    run_alternate_platform_picker_case dm1 \
        "$data_root/dm1/Dungeon-Master_Atari-ST_EN.zip" 2 Atari\ ST st10a-en
elif [[ -f "$data_root/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip" ]]; then
    run_alternate_platform_picker_case dm1 \
        "$data_root/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip" 2 Atari\ ST st12-en
fi
if [[ -f "$data_root/dm1/Dungeon-Master_Amiga_EN_Version-20.zip" ]]; then
    run_alternate_platform_picker_case dm1 \
        "$data_root/dm1/Dungeon-Master_Amiga_EN_Version-20.zip" 3 Amiga \
        're:amiga(20-en|20-fr|20-de|21-en|36-multi|demo-en)'
fi
csb_amiga_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"
if [[ -f "$data_root/csb/Chaos Strikes Back (FTL).zip" ]]; then
    csb_amiga_archive="$data_root/csb/Chaos Strikes Back (FTL).zip"
fi
if [[ -f "$csb_amiga_archive" ]]; then
    case "$csb_amiga_archive" in
        "$data_root/csb/Chaos Strikes Back (FTL).zip")
            expected_csb_amiga_sha256=bff24bffd2e07ff8ac4c96b3c35be93ee3fdfe219d1bf833df247726b2ce3abb
            ;;
        "$data_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip")
            expected_csb_amiga_sha256=fa0296c7bf62d806e8b1c45542d8ac225b67f4080026fa7f6fb61c97905f24c0
            ;;
        *)
            printf 'FAIL: unexpected CSB Amiga archive path: %s\n' "$csb_amiga_archive" >&2
            exit 1
            ;;
    esac
    if [[ $(firestaff_sha256_file "$csb_amiga_archive") != "$expected_csb_amiga_sha256" ]]; then
        printf 'FAIL: CSB Amiga archive identity drift: %s\n' "$csb_amiga_archive" >&2
        exit 1
    fi
    run_alternate_platform_picker_case csb \
        "$csb_amiga_archive" 1 Amiga \
        're:amiga(31-en|31-multi|35-en|35-multi)'
fi
if [[ -f "$data_root/csb/chaos_strikes_back_ftl.zip" ]]; then
    run_alternate_platform_picker_case csb \
        "$data_root/csb/chaos_strikes_back_ftl.zip" 2 'Atari ST' \
        're:st20-21(-hd)?-en'
fi
if [[ -f "$data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip" ]]; then
    run_alternate_platform_picker_case dm2 \
        "$data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip" 1 PC pc-en
fi
if [[ -f "$data_root/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip" ]]; then
    run_alternate_platform_picker_case dm2 \
        "$data_root/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip" 2 Amiga amiga-en
fi
if [[ -f "$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip" ]]; then
    run_alternate_platform_picker_case dm2 \
        "$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip" 3 Macintosh mac-en-retail
fi
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} == 1 ]]; then
    if [[ $alternate_platform_cases == 0 ]]; then
        echo 'SKIP: no authentic alternate-platform archives are available'
        exit 77
    fi
    exit 0
fi

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
