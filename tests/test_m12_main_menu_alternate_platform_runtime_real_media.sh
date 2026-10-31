#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_alternate_platform_runtime_real_media.sh <firestaff> <data-root> <route>}
data_root=${2:?usage: test_m12_main_menu_alternate_platform_runtime_real_media.sh <firestaff> <data-root> <route>}
route=${3:?usage: test_m12_main_menu_alternate_platform_runtime_real_media.sh <firestaff> <data-root> <route>}
data_root=$(CDPATH= cd -- "$data_root" && pwd -P)

dm1_pc="$data_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
dm1_atari="$data_root/dm1/Dungeon-Master_Atari-ST_EN.zip"
dm1_atari_st11="$data_root/dm1/Game,Dungeon_Master,Atari_ST,Software.7z"
dm1_atari_st12="$data_root/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip"
dm1_atari_st12_de="$data_root/dm1/Dungeon-Master_Atari-ST_DE_Version-12.zip"
dm1_atari_st13="$data_root/dm1/Dungeon-Master_Atari-ST_FR_Version-13.zip"
csb_atari="$data_root/csb/chaos_strikes_back_ftl.zip"
csb_atari_france="$data_root/csb/chaos_strikes_back_ftl_france.zip"
dm2_pc="$data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"

case "$route" in
    dm1-pc)
        selected_game=dm1; game_down=0; platform_down=1
        selected_archive=$dm1_pc; edition=pc34-en; platform=PC
        width=320; height=200; duration=45000
        game_input='wait20,key:enter,wait20,key:enter,wait20,key:enter'
        ;;
    dm1-atari-st)
        selected_game=dm1; game_down=0; platform_down=2
        selected_archive=$dm1_atari; edition=st10a-en; platform='Atari ST'
        width=960; height=540; duration=90000
        game_input='wait30,enter,wait60,enter'
        ;;
    dm1-atari-st-11)
        selected_game=dm1; game_down=0; platform_down=0
        selected_archive=$dm1_atari_st11; edition=st11-en; platform='Atari ST'
        width=960; height=540; duration=45000
        game_input='wait30,enter,wait60,enter'
        ;;
    dm1-atari-st-12)
        selected_game=dm1; game_down=0; platform_down=0
        selected_archive=$dm1_atari_st12; edition=st12-en; platform='Atari ST'
        width=960; height=540; duration=90000
        game_input='wait30,enter,wait60,enter'
        ;;
    dm1-atari-st-12-de)
        selected_game=dm1; game_down=0; platform_down=0
        selected_archive=$dm1_atari_st12_de; edition=st12-de; platform='Atari ST'
        width=960; height=540; duration=90000
        game_input='wait30,enter,wait60,enter'
        ;;
    dm1-atari-st-13)
        selected_game=dm1; game_down=0; platform_down=0
        selected_archive=$dm1_atari_st13; edition=st13-fr; platform='Atari ST'
        width=960; height=540; duration=90000
        game_input='wait30,enter,wait60,enter'
        ;;
    csb-atari-st)
        selected_game=csb; game_down=1; platform_down=2
        selected_archive=$csb_atari; edition='re:st20-21(-hd)?-en'; platform='Atari ST'
        width=960; height=600; duration=120000
        game_input='wait-game-runtime,key:up'
        ;;
    csb-atari-st-france)
        selected_game=csb; game_down=1; platform_down=0
        selected_archive=$csb_atari_france; edition=st20-21-en; platform='Atari ST'
        width=960; height=600; duration=120000
        game_input='wait-game-runtime,key:up'
        ;;
    dm2-pc)
        selected_game=dm2; game_down=2; platform_down=1
        selected_archive=$dm2_pc; edition=pc-en; platform=PC
        width=320; height=200; duration=90000
        game_input='wait:1800,key:enter,key:enter'
        ;;
    *)
        printf 'FAIL: unsupported M12 alternate runtime route: %s\n' "$route" >&2
        exit 2
        ;;
esac

if [[ ! -x "$app" || ! -f "$selected_archive" ]]; then
    printf 'SKIP: authentic %s media or Firestaff executable is unavailable\n' "$route"
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-alt-runtime-$route.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

print_diagnostics() {
    local log_file=$1
    local matched
    matched=$(grep -E '(^| )(FAIL:|CSB READY:|DM1 READY:|DM2 READY:|firestaff: launch phase=|firestaff: csb-atari-animation|firestaff: dm2-render-debug|firestaff: dm2-render-item|FIRESTAFF BOOT PROBE READY:)' "$log_file" | tail -25 || true)
    if [[ -n "$matched" ]]; then
        printf '%s\n' "$matched" >&2
    else
        tail -80 "$log_file" >&2
    fi
}

# Expose only DM1, CSB and DM2 to the menu. Symlink original top-level
# archives so platform ordering is representative without copying game data
# or scanning games owned by other work.
menu_data_root="$scratch/data"
if [[ "$route" == dm1-atari-st-11 ]]; then
    extractor=${FIRESTAFF_7ZZ:-7zz}
    member='Floppy Disks STX/Dungeon Master for Atati ST v1.1 (English).stx'
    if ! command -v "$extractor" >/dev/null 2>&1; then
        echo 'SKIP: 7zz is required to extract the original DM1 Atari ST 1.1 disk'
        exit 77
    fi
    extracted_stx="$scratch/Dungeon Master Atari ST 1.1.stx"
    "$extractor" e -so "$selected_archive" "$member" >"$extracted_stx" 2>/dev/null
    if [[ ! -s "$extracted_stx" ]]; then
        echo 'FAIL: original DM1 Atari ST 1.1 STX extraction was empty' >&2
        exit 1
    fi
fi
for game_id in dm1 csb dm2; do
    if [[ "$route" == csb-atari-st-france && "$game_id" != csb ]]; then
        continue
    fi
    source_game_dir="$data_root/$game_id"
    menu_game_dir="$menu_data_root/$game_id"
    mkdir -p "$menu_game_dir"
    [[ -d "$source_game_dir" ]] || continue
    while IFS= read -r -d '' archive; do
        [[ -f "$archive" ]] || continue
        if [[ "$route" == dm1-atari-st-11 && "$game_id" == dm1 ]]; then
            continue
        fi
        if [[ "$route" == csb-atari-st-france && "$game_id" == csb &&
              "$(basename "$archive")" != "$(basename "$selected_archive")" ]]; then
            continue
        fi
        if [[ ( "$route" == dm1-atari-st-12 || "$route" == dm1-atari-st-12-de ||
              "$route" == dm1-atari-st-13 ) &&
              "$game_id" == dm1 &&
              $(basename "$archive") != $(basename "$selected_archive") ]]; then
            continue
        fi
        ln -s "$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)" "$menu_game_dir/$(basename "$archive")"
    done < <(find "$source_game_dir" -maxdepth 1 \( -type f -o -type l \) \
        \( -iname '*.zip' -o -iname '*.stx' \) -print0)
    if [[ "$game_id" == csb ]]; then
        if [[ "$route" == csb-atari-st-france ]]; then
            continue
        fi
        while IFS= read -r -d '' archive; do
            [[ -f "$archive" ]] || continue
            ln -s "$(python3 - "$archive" <<'PY'
import os
import sys

print(os.path.realpath(sys.argv[1]))
PY
)" "$menu_game_dir/$(basename "$archive")"
        done < <(find "$source_game_dir" -maxdepth 1 \( -type f -o -type l \) \
            -iname '*.7z' -print0)
    fi
done
if [[ "$route" == dm1-atari-st-11 ]]; then
    ln -s "$extracted_stx" "$menu_data_root/dm1/$(basename "$extracted_stx")"
fi

home="$scratch/home"
log="$scratch/firestaff.log"
probe="$scratch/runtime.json"
archive_hash=$(firestaff_sha256_file "$selected_archive")
navigation='wait-scan-complete,wait20'
for ((index = 0; index < game_down; ++index)); do
    navigation+=',key:down,wait20'
done
navigation+=',key:enter,wait20'
for ((index = 0; index < platform_down; ++index)); do
    navigation+=',key:down,wait20'
done
navigation+=",key:enter,wait20,key:enter,wait20,key:enter,$game_input"

mkdir -p "$home"
env -u FIRESTAFF_EXIT_AFTER_LAUNCH \
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_DATA="$menu_data_root" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width "$width" --height "$height" \
        --data-dir "$menu_data_root" --script "$navigation" \
        --duration "$duration" >"$log" 2>&1 || {
            print_diagnostics "$log"
            exit 1
        }

if ! grep -Fq 'startup game=none mode=menu' "$log"; then
    printf 'FAIL: %s did not start at the no-game menu\n' "$route" >&2
    print_diagnostics "$log"
    exit 1
fi
if [[ "$edition" == re:* ]]; then
    edition_pattern=${edition#re:}
    if ! grep -E "launch phase=game-handoff mode=menu game=$selected_game platform=$platform edition=$edition_pattern[[:space:]]+source=" "$log" >/dev/null; then
        printf 'FAIL: %s no-game menu did not hand off its selected original edition\n' "$route" >&2
        cat "$log" >&2
        exit 1
    fi
else
    if ! grep -Fq "launch phase=game-handoff mode=menu game=$selected_game platform=$platform edition=$edition " "$log"; then
        printf 'FAIL: %s no-game menu did not hand off its selected original edition\n' "$route" >&2
        cat "$log" >&2
        exit 1
    fi
fi

if ! python3 "$(dirname "$0")/verify_m12_alternate_platform_runtime.py" "$route" "$probe"; then
    cat "$log" >&2
    exit 1
fi
if [[ "$route" == dm2-pc ]]; then
    if ! grep -Eq '^firestaff: dm2-render-debug .*item-traces:[1-9][0-9]*([[:space:]]|$)' "$log" ||
       ! grep -Eq '^firestaff: dm2-render-item game=dm2 platform=pc-en .* gdat=[0-9a-f]+ .* rect=-?[0-9]+,-?[0-9]+,[1-9][0-9]*,[1-9][0-9]*$' "$log"; then
        printf 'FAIL: authentic DM2 PC startup did not produce a populated source item render trace\n' >&2
        print_diagnostics "$log"
        exit 1
    fi
fi
if [[ $(firestaff_sha256_file "$selected_archive") != "$archive_hash" ]]; then
    printf 'FAIL: original %s source changed during M12 runtime selection\n' "$route" >&2
    exit 1
fi
printf 'PASS: no-game M12 menu reached authentic %s runtime\n' "$route"
