#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_game_selection_real_media.sh <firestaff> <data-root>}
data_root=$(CDPATH= cd -- "$data_root" && pwd -P)

dm1_archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
csb_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm2_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
nexus_dm="$data_root/nexus/DM.BIN"
nexus_cue="$data_root/nexus/Dungeon Master Nexus (Japan).cue"
theron_track="$data_root/theron/TQUS02.bin"

has_authentic_nexus_cue() {
    [[ -f "$nexus_cue" ]] || return 1
    python3 - "$nexus_cue" <<'PY'
import pathlib
import re
import sys

cue = pathlib.Path(sys.argv[1])
payloads = []
for line in cue.read_text(errors="strict").splitlines():
    match = re.match(r'\s*FILE\s+(?:"([^"]+)"|([^\s]+))', line, re.IGNORECASE)
    if match:
        payloads.append(cue.parent / (match.group(1) or match.group(2)))
if not payloads or not all(path.is_file() for path in payloads):
    raise SystemExit(1)
PY
}

if [[ ! -x "$app" ]]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_SCAN_EARLY_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_POINTER_ONLY:-0} != 1 &&
      ( ! -f "$dm1_archive" || ! -f "$csb_archive" || ! -f "$dm2_archive" ) ]]; then
    echo 'SKIP: authentic DM1, CSB and DM2 FM Towns archives are required'
    exit 77
fi
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_SCAN_EARLY_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_POINTER_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} != 1 &&
      ( ( ! -f "$nexus_dm" && ! -f "$nexus_cue" ) || ! -f "$theron_track" ) ]]; then
    echo 'SKIP: authentic Nexus DM.BIN or Japanese CUE/BIN media and Theron TQUS02.bin are required for the five-game selector test'
    exit 77
fi
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_SCAN_EARLY_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_POINTER_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} != 1 &&
      ! -f "$nexus_dm" ]] && ! has_authentic_nexus_cue; then
    echo 'SKIP: authentic Japanese Nexus CUE payloads are incomplete'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-main-game-picker.XXXXXX")
archive_data_root="$scratch/archive-only-data"
declare -a original_sources=()
declare -a original_source_hashes=()

# The normal data root may contain extracted editions beside original
# archives. Build a symlink-only view of top-level ZIP archives and the raw
# CSB Atari STX source so picker ordering and edition availability stay intact
# without exposing extracted trees. Snapshot each source so the test verifies
# it stays intact.
verify_original_sources_unchanged() {
    local source
    local index=0
    if [[ ${#original_sources[@]} == 0 ]]; then
        return 0
    fi
    for source in "${original_sources[@]}"; do
        if [[ $(firestaff_sha256_file "$source") != "${original_source_hashes[$index]}" ]]; then
            printf 'FAIL: original game source changed during picker test: %s\n' "$source" >&2
            return 1
        fi
        index=$((index + 1))
    done
}

cleanup() {
    local test_status=$?
    trap - EXIT HUP INT TERM
    if ! verify_original_sources_unchanged; then
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

for game_id in dm1 csb dm2; do
    source_game_dir="$data_root/$game_id"
    overlay_game_dir="$archive_data_root/$game_id"
    mkdir -p "$overlay_game_dir"
    if [[ ! -d "$source_game_dir" ]]; then
        continue
    fi
    while IFS= read -r -d '' archive; do
        [[ -f "$archive" ]] || continue
        absolute_archive=$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.abspath(sys.argv[1]))
PY
)
        archive_hash=$(firestaff_sha256_file "$absolute_archive")
        ln -s "$absolute_archive" "$overlay_game_dir/$(basename "$archive")"
        original_sources+=("$absolute_archive")
        original_source_hashes+=("$archive_hash")
    done < <(find "$source_game_dir" -maxdepth 1 \( -type f -o -type l \) \
        \( -iname '*.zip' -o -iname '*.stx' \) -print0)
done

main_menu_data_root=$data_root
if [[ ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} == 1 ]]; then
    main_menu_data_root=$archive_data_root
    csb_amiga_multi_archive="$data_root/csb/Game,Chaos_Strikes_Back,Amiga,Software.7z"
    if [[ -f "$csb_amiga_multi_archive" ]]; then
        ln -s "$csb_amiga_multi_archive" "$archive_data_root/csb/$(basename "$csb_amiga_multi_archive")"
        original_sources+=("$csb_amiga_multi_archive")
        original_source_hashes+=("$(firestaff_sha256_file "$csb_amiga_multi_archive")")
    fi
fi

if [[ ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} != 1 ]] && has_authentic_nexus_cue; then
    original_sources+=("$nexus_cue")
    original_source_hashes+=("$(firestaff_sha256_file "$nexus_cue")")
    while IFS= read -r cue_payload; do
        original_sources+=("$cue_payload")
        original_source_hashes+=("$(firestaff_sha256_file "$cue_payload")")
    done < <(python3 - "$nexus_cue" <<'PY'
import pathlib
import re
import sys

cue = pathlib.Path(sys.argv[1])
for line in cue.read_text(errors="strict").splitlines():
    match = re.match(r'\s*FILE\s+(?:"([^"]+)"|([^\s]+))', line, re.IGNORECASE)
    if match:
        print(cue.parent / (match.group(1) or match.group(2)))
PY
    )
fi
if [[ ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} != 1 && -f "$theron_track" ]]; then
    original_sources+=("$theron_track")
    original_source_hashes+=("$(firestaff_sha256_file "$theron_track")")
fi

if [[ ${#original_sources[@]} == 0 ]]; then
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
    elif [[ "$source_mode" == nexus-root ]]; then
        local launch_line
        local logged_source
        local logged_media
        local nexus_data_root="$menu_data_root/nexus"
        if [[ "$(basename "$menu_data_root")" == nexus ]]; then
            nexus_data_root=$menu_data_root
        fi
        launch_line=$(grep -E '^firestaff: launch phase=game-handoff mode=menu game=nexus platform=Saturn edition=(nexus|nexus2|nexus-saturn-(jp|jp-cue|en-extracted|en-fan-v2|en-cue|fr-extracted|fr-fan|fr-cue))[[:space:]]+source=.+' "$log" | head -n 1 || true)
        if [[ -z "$launch_line" ]]; then
            printf 'FAIL: no-game main menu did not hand off an authenticated Nexus Saturn edition\n' >&2
            cat "$log" >&2
            return 1
        fi
        logged_source=${launch_line##* source=}
        logged_media=${logged_source%%::*}
        if ! python3 - "$nexus_data_root" "$logged_media" <<'PY'
import pathlib
import sys

root = pathlib.Path(sys.argv[1]).resolve()
media = pathlib.Path(sys.argv[2]).resolve()
try:
    media.relative_to(root)
except ValueError:
    raise SystemExit("selected Nexus media is outside the selected authentic data root")
if not media.is_file():
    raise SystemExit("selected Nexus media is not an existing original file")
PY
        then
            printf 'FAIL: Nexus menu handoff did not use original media from the selected data root\n' >&2
            cat "$log" >&2
            return 1
        fi
    else
        printf 'FAIL: unsupported source assertion mode: %s\n' "$source_mode" >&2
        return 2
    fi
    if [[ "$game_id" == nexus ]] && {
        ! grep -Fq 'NEXUS STARTUP RECEIPT: status=blocked gameId=nexus' "$log" ||
        ! grep -Fq 'blocker=title-vdp-owner-compositor-capture-required' "$log" ||
        grep -Fq 'launch phase=runtime-entered game=nexus' "$log"
    }; then
        printf 'FAIL: Nexus menu route did not stop at its documented capture-gated title state\n' >&2
        cat "$log" >&2
        return 1
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
    local picker_data_root=${6:-$archive_data_root}
    local navigation='wait-scan-complete,wait20'
    local game_card_index=0
    local index
    local overlay_archive
    overlay_archive="$picker_data_root/$game_id/$(basename "$source_archive")"
    if [[ ! -f "$overlay_archive" ]]; then
        printf 'FAIL: authentic source is not present in the isolated picker view: %s\n' \
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
        "$edition" keyboard "$platform" "$picker_data_root" \
        "$game_id-$platform-picker" resolved
    alternate_platform_cases=$((alternate_platform_cases + 1))
}

alternate_platform_cases=0

if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_SCAN_EARLY_ONLY:-0} != 1 &&
      ${FIRESTAFF_PICKER_PRIORITY_POINTER_ONLY:-0} != 1 ]]; then
    run_game_picker_case dm1 "$main_menu_data_root/dm1/$(basename "$dm1_archive")" \
        'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        fmtowns-en keyboard 'FM Towns' "$main_menu_data_root"
    run_game_picker_case csb "$main_menu_data_root/csb/$(basename "$csb_archive")" \
        'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        fmtowns-en keyboard 'FM Towns' "$main_menu_data_root"
    run_game_picker_case dm2 "$main_menu_data_root/dm2/$(basename "$dm2_archive")" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        fmtowns-ja keyboard 'FM Towns' "$main_menu_data_root"
fi

# Keep authentic pointer routes for DM1, CSB, and DM2 independently
# runnable. The full five-game selector test reaches Nexus/Theron before its
# pointer cases, so a failure owned by either game must not hide these routes.
# Each route clicks the game, platform, and Original-presentation cards.
if [[ ${FIRESTAFF_PICKER_PRIORITY_POINTER_ONLY:-0} == 1 ]]; then
    dm1_pc_archive="$archive_data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
    csb_atari_archive="$archive_data_root/csb/chaos_strikes_back_ftl.zip"
    dm2_amiga_archive="$archive_data_root/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"
    if [[ ! -f "$dm1_pc_archive" || ! -f "$csb_atari_archive" ||
          ! -f "$dm2_amiga_archive" ]]; then
        echo 'SKIP: authentic DM1 PC, CSB Atari ST, and DM2 Amiga archives are required for priority pointer selection'
        exit 77
    fi
    run_game_picker_case dm1 "$dm1_pc_archive" \
        'wait-scan-complete,wait20,click:700:262,wait20,click:934:405,wait20,click:450:405' \
        pc34-en mouse PC "$archive_data_root" dm1-pc-pointer resolved
    run_game_picker_case csb "$csb_atari_archive" \
        'wait-scan-complete,wait20,click:1190:262,wait20,click:1458:405,wait20,click:450:405' \
        st20-21-en mouse 'Atari ST' "$archive_data_root" \
        csb-atari-pointer resolved
    run_game_picker_case dm2 "$dm2_amiga_archive" \
        'wait-scan-complete,wait20,click:1680:262,wait20,click:1458:405,wait20,click:450:405' \
        amiga-en mouse Amiga "$archive_data_root" dm2-amiga-pointer resolved
    exit 0
fi

# Exercise early pointer selection while the authentic game-data scan is
# still running. The first platform click must be safely deferred, and a
# retry after publication must complete the same Original-presentation route.
if [[ ${FIRESTAFF_PICKER_PRIORITY_SCAN_EARLY_ONLY:-0} == 1 ]]; then
    dm1_pc_archive="$archive_data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
    csb_atari_archive="$archive_data_root/csb/chaos_strikes_back_ftl.zip"
    dm2_amiga_archive="$archive_data_root/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"
    if [[ ! -f "$dm1_pc_archive" || ! -f "$csb_atari_archive" ||
          ! -f "$dm2_amiga_archive" ]]; then
        echo 'SKIP: authentic DM1 PC, CSB Atari ST, and DM2 Amiga archives are required for priority scan-time selection'
        exit 77
    fi
    run_early_scan_case() {
        local game_id=$1 archive=$2 navigation=$3 edition=$4 platform=$5 case_id=$6
        run_game_picker_case "$game_id" "$archive" "$navigation" "$edition" \
            mouse "$platform" "$archive_data_root" "$case_id" resolved
        local log="$scratch/$case_id.log"
        local gate_line scan_line
        gate_line=$(grep -n -F "launch gate game=$game_id result=deferred reason=startup-scan-incomplete" "$log" | head -n1 | cut -d: -f1 || true)
        scan_line=$(grep -n -F 'firestaff: scan complete' "$log" | head -n1 | cut -d: -f1 || true)
        if [[ -z "$gate_line" || -z "$scan_line" || "$gate_line" -ge "$scan_line" ]]; then
            printf 'FAIL: %s pointer route did not record an early scan gate before scan completion\n' "$game_id" >&2
            cat "$log" >&2
            return 1
        fi
        printf 'PASS: %s early platform selection was safely gated during scan and succeeded after retry\n' "$game_id"
    }
    run_early_scan_case dm1 "$dm1_pc_archive" \
        'click:700:262,click:934:405,wait-scan-complete,wait20,click:934:405,wait20,click:450:405' \
        pc34-en PC dm1-pc-scan-early
    run_early_scan_case csb "$csb_atari_archive" \
        'click:1190:262,click:1458:405,wait-scan-complete,wait20,click:1458:405,wait20,click:450:405' \
        st20-21-en 'Atari ST' csb-atari-scan-early
    run_early_scan_case dm2 "$dm2_amiga_archive" \
        'click:1680:262,click:1458:405,wait-scan-complete,wait20,click:1458:405,wait20,click:450:405' \
        amiga-en Amiga dm2-amiga-scan-early
    exit 0
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
if [[ -f "$data_root/dm1/Dungeon-Master_Amiga_EN.zip" ]]; then
    # This authentic HD archive contains the launchable nested Amiga image.
    # Keep other Amiga archives out so the selector must use this source.
    dm1_amiga_picker_root="$scratch/dm1-amiga-picker-data"
    mkdir -p "$dm1_amiga_picker_root/dm1"
    ln -s "$data_root/dm1/Dungeon-Master_Amiga_EN.zip" \
        "$dm1_amiga_picker_root/dm1/Dungeon-Master_Amiga_EN.zip"
    run_alternate_platform_picker_case dm1 \
        "$data_root/dm1/Dungeon-Master_Amiga_EN.zip" 0 Amiga \
        're:amiga20-en' \
        "$dm1_amiga_picker_root"
fi
csb_amiga_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"
if [[ -f "$data_root/csb/Chaos Strikes Back (FTL).zip" ]]; then
    csb_amiga_archive="$data_root/csb/Chaos Strikes Back (FTL).zip"
fi
if [[ -f "$csb_amiga_archive" ]]; then
    case "$csb_amiga_archive" in
        "$data_root/csb/Chaos Strikes Back (FTL).zip")
            # Accept the checked-in corpus wrapper and the local ZIP wrapper
            # around the authenticated A31E ADF. The latter's embedded
            # GRAPHICS.DAT identity is asserted by
            # test_csb_v1_amiga_adf_archive_cli_boot.sh.
            expected_csb_amiga_sha256='bff24bffd2e07ff8ac4c96b3c35be93ee3fdfe219d1bf833df247726b2ce3abb|af597a8f45aedecfb7f08086cbea47ae5ad24dae1f691514dcf99eac2b347736'
            ;;
        "$data_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip")
            # The local authentic A33M repack preserves its original archive
            # entries but has a distinct ZIP wrapper hash.
            expected_csb_amiga_sha256='fa0296c7bf62d806e8b1c45542d8ac225b67f4080026fa7f6fb61c97905f24c0|af398ab8ee8516da714859622f779a052c0d97103c3c058e1bf2bcea2ab425be'
            ;;
        *)
            printf 'FAIL: unexpected CSB Amiga archive path: %s\n' "$csb_amiga_archive" >&2
            exit 1
            ;;
    esac
    csb_amiga_sha256=$(firestaff_sha256_file "$csb_amiga_archive")
    if [[ "|$expected_csb_amiga_sha256|" != *"|$csb_amiga_sha256|"* ]]; then
        printf 'FAIL: CSB Amiga archive identity drift: %s\n' "$csb_amiga_archive" >&2
        exit 1
    fi
    run_alternate_platform_picker_case csb \
        "$csb_amiga_archive" 1 Amiga \
        're:amiga(31-en|31-multi|35-en|35-multi)'
fi
csb_amiga_multi_archive="$data_root/csb/Game,Chaos_Strikes_Back,Amiga,Software.7z"
if [[ -f "$csb_amiga_multi_archive" ]]; then
    csb_amiga_multi_picker_archive=$csb_amiga_multi_archive
    if [[ ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} == 1 ]]; then
        csb_amiga_multi_picker_archive="$main_menu_data_root/csb/$(basename "$csb_amiga_multi_archive")"
    fi
    # Exercise actual SDL keyboard routing through Game Options. Right on the
    # version row changes the authentic A31 English selection to the matching
    # A31 multilingual edition before launch.
    run_game_picker_case csb "$csb_amiga_multi_picker_archive" \
        'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:down,wait20,key:enter,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:down,wait20,key:right,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter' \
        amiga31-multi keyboard Amiga "$main_menu_data_root" \
        csb-amiga-version-right-arrow resolved
fi
if [[ -f "$data_root/csb/Chaos Strikes Back.stx" ]]; then
    csb_atari_picker_root="$scratch/csb-atari-stx-picker-data"
    for game_id in dm1 csb dm2; do
        mkdir -p "$csb_atari_picker_root/$game_id"
        while IFS= read -r -d '' source_archive; do
            source_basename=$(basename "$source_archive")
            # Keep the other game cards and CSB platforms available so the
            # scripted selection follows the same menu route, while removing
            # only the Atari ZIP that could win over this authentic raw STX.
            if [[ "$game_id" == csb &&
                  ( "$source_basename" == 'Chaos Strikes Back (FTL).zip' ||
                    "$source_basename" == chaos_strikes_back_ftl.zip ) ]]; then
                continue
            fi
            ln -s "$source_archive" \
                "$csb_atari_picker_root/$game_id/$source_basename"
        done < <(find "$archive_data_root/$game_id" -maxdepth 1 -type l -print0)
    done
    run_alternate_platform_picker_case csb \
        "$data_root/csb/Chaos Strikes Back.stx" 2 'Atari ST' \
        're:st20-21(-hd)?-en' "$csb_atari_picker_root"
elif [[ -f "$data_root/csb/chaos_strikes_back_ftl.zip" ]]; then
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
dm2_mac_archive=""
# Authenticate the Macintosh payload through Firestaff's catalogued retail
# Graphics.dat identity. The original files may be repacked into ZIP without
# changing that game-data identity, so the outer ZIP hash is not authoritative.
dm2_mac_archive_present=0
dm2_mac_retail_asset_md5=5cab25f6b975957eae4a203174e7f2a6
for candidate in \
    "$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip" \
    "$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"; do
    if [[ -f "$candidate" ]]; then
        dm2_mac_archive_present=1
        candidate_probe=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm2 --platform mac --data-dir "$candidate" --boot-probe \
            --boot-probe-frames 1 --duration 0 2>&1) || candidate_probe=""
        if [[ "$candidate_probe" == *"assetMd5=$dm2_mac_retail_asset_md5"* ]]; then
            dm2_mac_archive=$candidate
            break
        fi
    fi
done
if [[ -n "$dm2_mac_archive" ]]; then
    run_alternate_platform_picker_case dm2 \
        "$dm2_mac_archive" 3 Macintosh mac-en-retail
elif [[ $dm2_mac_archive_present == 1 ]]; then
    echo 'FAIL: no DM2 Macintosh ZIP contains the catalogued retail Graphics.dat' >&2
    exit 1
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
    local platform=${5:-FM Towns}
    local picker_data_root=${6:-$data_root}
    local case_id=${7:-$game_id}
    local home="$scratch/default-root-$case_id-home"
    local data_home="$scratch/default-data-$case_id-home"
    local log="$scratch/default-root-$case_id.log"
    local source_member=''
    local source_path
    local source_hash
    local launch_line
    local logged_source
    local logged_media

    case "$game_id" in
        dm1|dm2) source_member='::DATA/GRAPHICS.DAT' ;;
        csb) source_member='::CDATA/GRAPHICS.DAT' ;;
        nexus|theron) ;;
        *) printf 'FAIL: unsupported default-root game: %s\n' "$game_id" >&2; return 2 ;;
    esac
    source_path="$data_home/.firestaff/data/$game_id/$(basename "$game_archive")$source_member"
    source_hash=$(firestaff_sha256_file "$game_archive")
    mkdir -p "$home" "$data_home/.firestaff"
    if [[ ! -e "$data_home/.firestaff/data" ]]; then
        ln -s "$picker_data_root" "$data_home/.firestaff/data"
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
    if ! grep -Fq 'startup game=none mode=menu' "$log"; then
        printf 'FAIL: default .firestaff/data menu did not launch authentic %s %s media\n' "$game_id" "$platform" >&2
        cat "$log" >&2
        return 1
    fi
    if [[ "$game_id" == nexus ]]; then
        # A complete Nexus data folder can contain extracted Japanese retail
        # resources alongside English/French translated ISOs and CUE/BIN
        # media. The authenticated menu chooses the selected owner's best
        # complete disc; do not hard-code Japanese RLOWFIX.BIN as the launch
        # source when a different real edition wins the source-priority pass.
        launch_line=$(grep -E '^firestaff: launch phase=game-handoff mode=menu game=nexus platform=Saturn edition=(nexus|nexus2|nexus-saturn-(jp|jp-cue|en-extracted|en-fan-v2|en-cue|fr-extracted|fr-fan|fr-cue))[[:space:]]+source=.+' "$log" | head -n 1 || true)
        if [[ -z "$launch_line" ]]; then
            printf 'FAIL: default .firestaff/data menu did not hand off an authenticated Nexus Saturn edition\n' >&2
            cat "$log" >&2
            return 1
        fi
        logged_source=${launch_line##* source=}
        logged_media=${logged_source%%::*}
        if ! python3 - "$data_home/.firestaff/data/nexus" "$logged_media" <<'PY'
import pathlib
import sys

root = pathlib.Path(sys.argv[1])
media = pathlib.Path(sys.argv[2])
try:
    media.relative_to(root)
except ValueError:
    raise SystemExit("selected Nexus media is outside the authentic default data root")
if not media.is_file():
    raise SystemExit("selected Nexus media is not an existing original file")
PY
        then
            printf 'FAIL: default-root Nexus launch did not use original media from its authentic game-data folder\n' >&2
            cat "$log" >&2
            return 1
        fi
    elif ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition source=$source_path" "$log"; then
        printf 'FAIL: default .firestaff/data menu did not launch authentic %s %s media\n' "$game_id" "$platform" >&2
        cat "$log" >&2
        return 1
    fi
    if [[ $(firestaff_sha256_file "$game_archive") != "$source_hash" ]]; then
        printf 'FAIL: default .firestaff/data menu changed the original %s archive\n' "$game_id" >&2
        return 1
    fi
    if [[ "$game_id" == nexus ]] && {
        ! grep -Fq 'NEXUS STARTUP RECEIPT: status=blocked gameId=nexus' "$log" ||
        ! grep -Fq 'blocker=title-vdp-owner-compositor-capture-required' "$log" ||
        grep -Fq 'launch phase=runtime-entered game=nexus' "$log"
    }; then
        printf 'FAIL: default-root Nexus route did not stop at its documented capture-gated title state\n' >&2
        cat "$log" >&2
        return 1
    fi
    printf 'PASS: default .firestaff/data menu launched authentic %s %s\n' "$game_id" "$platform"
}

nexus_source="$data_root/nexus/RLOWFIX.BIN"
if [[ ! -f "$nexus_source" ]]; then
    nexus_source="$nexus_dm"
fi

run_default_root_game_picker_case dm1 "$dm1_archive" \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en \
    'FM Towns' \
    "${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:+$archive_data_root}"
run_default_root_game_picker_case csb "$csb_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-en \
    'FM Towns' \
    "${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:+$archive_data_root}"
run_default_root_game_picker_case dm2 "$dm2_archive" \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' fmtowns-ja \
    'FM Towns' \
    "${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:+$archive_data_root}"

# Keep the focused priority-game test bounded to DM1, CSB and DM2. The full
# five-game picker test below continues to cover Nexus and Theron when those
# games are in scope; this variant never starts either game.
if [[ ${FIRESTAFF_PICKER_PRIORITY_DM1_CSB_DM2_ONLY:-0} == 1 ]]; then
    echo 'PASS: DM1, CSB and DM2 default-root picker routes completed with authentic media'
    exit 0
fi

# The other two games must resolve from the same documented default root.
# Their authentic launch owners may be loose binaries, CUE/BIN sets or full
# disc images depending on the user's real collection and selected profile.
if [[ -f "$nexus_source" ]]; then
    run_default_root_game_picker_case nexus "$nexus_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        authenticated-saturn-edition Saturn
elif has_authentic_nexus_cue; then
    run_default_root_game_picker_case nexus "$nexus_cue" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        authenticated-saturn-edition Saturn
else
    echo 'SKIP: authentic Nexus Saturn media is unavailable for default-root startup'
fi
if [[ -f "$theron_track" ]]; then
    run_default_root_game_picker_case theron "$theron_track" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en 'PC Engine'
else
    echo 'SKIP: authentic Theron PC Engine TQUS02.bin is unavailable for default-root startup'
fi

# Also exercise the default-root selector with the authentic Japanese Saturn
# CUE/BIN as the only Nexus owner. Keep the other four authentic game sources
# available so the menu still selects Nexus from the normal five-card list.
if [[ ${FIRESTAFF_PICKER_PLATFORM_ONLY:-0} != 1 ]] && has_authentic_nexus_cue; then
    nexus_cue_only_root="$scratch/nexus-cue-only-five-game-root"
    mkdir -p "$nexus_cue_only_root"/{dm1,csb,dm2,nexus,theron}
    ln -s "$dm1_archive" "$nexus_cue_only_root/dm1/$(basename "$dm1_archive")"
    ln -s "$csb_archive" "$nexus_cue_only_root/csb/$(basename "$csb_archive")"
    ln -s "$dm2_archive" "$nexus_cue_only_root/dm2/$(basename "$dm2_archive")"
    ln -s "$theron_track" "$nexus_cue_only_root/theron/$(basename "$theron_track")"
    python3 - "$nexus_cue" "$nexus_cue_only_root/nexus" <<'PY'
import pathlib
import re
import sys

cue = pathlib.Path(sys.argv[1])
destination = pathlib.Path(sys.argv[2])
for line in cue.read_text(errors="strict").splitlines():
    match = re.match(r'\s*FILE\s+(?:"([^"]+)"|([^\s]+))', line, re.IGNORECASE)
    if not match:
        continue
    source = cue.parent / (match.group(1) or match.group(2))
    if not source.is_file():
        raise SystemExit(f"Missing authentic Nexus CUE payload: {source}")
    target = destination / source.name
    if not target.exists():
        target.symlink_to(source)
target = destination / cue.name
if not target.exists():
    target.symlink_to(cue)
PY
    run_default_root_game_picker_case nexus \
        "$nexus_cue_only_root/nexus/$(basename "$nexus_cue")" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        nexus-saturn-jp-cue Saturn "$nexus_cue_only_root" nexus-jp-cue-only-default-root
fi

# Nexus and Theron use original media in this data root. The Nexus selector
# may choose any authenticated regional ISO/CUE or extracted owner present;
# these cases exercise the same five-game launcher selector without requiring
# external archive tools or extracting/copying game data.
theron_source="$theron_track"
if [[ -f "$nexus_source" ]]; then
    run_game_picker_case nexus "$nexus_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        authenticated-saturn-edition keyboard Saturn "$data_root" \
        nexus-keyboard-authenticated-owner nexus-root
    # A game-specific data directory is also a supported collection root.
    # Keep this route separate from the all-games root case so path promotion
    # and authenticated media ownership are verified together.
    run_game_picker_case nexus "$nexus_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        authenticated-saturn-edition keyboard Saturn "$data_root/nexus" \
        nexus-keyboard-authenticated-leaf nexus-root
else
    echo 'SKIP: authentic Nexus Saturn DM.BIN is unavailable'
fi
if [[ -f "$theron_source" ]]; then
    run_game_picker_case theron "$theron_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en keyboard 'PC Engine'
    run_game_picker_case theron "$theron_source" \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en keyboard 'PC Engine' "$data_root/theron" \
        theron-keyboard-authenticated-leaf resolved
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
        authenticated-saturn-edition mouse Saturn "$data_root" \
        nexus-mouse-authenticated-owner nexus-root
fi
if [[ -f "$theron_source" ]]; then
    run_game_picker_case theron "$theron_source" \
        'wait-scan-complete,wait20,click:1190:728,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        pce-en mouse 'PC Engine'
fi
