#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_runtime_real_media.sh <firestaff> <data-root>}
selected_game=${FIRESTAFF_MAIN_MENU_GAME:-all}
case "$selected_game" in
    all|dm1|csb|dm2|theron) ;;
    *) echo "FAIL: unsupported FIRESTAFF_MAIN_MENU_GAME=$selected_game" >&2; exit 2 ;;
esac

dm1_archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
csb_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm2_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
theron_us_track02="$data_root/theron/TQUS02.bin"
theron_jp_track02="$data_root/theron/TQJP02.bin"
if [[ ! -x "$app" ]]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi
if { [[ "$selected_game" == all || "$selected_game" == dm1 ]] && [[ ! -f "$dm1_archive" ]]; } || \
   { [[ "$selected_game" == all || "$selected_game" == csb ]] && [[ ! -f "$csb_archive" ]]; } || \
   { [[ "$selected_game" == all || "$selected_game" == dm2 ]] && [[ ! -f "$dm2_archive" ]]; }; then
    echo 'SKIP: authentic selected-game media is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-main-runtime.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

run_main_menu_runtime_case() {
    local game_id=$1
    local archive=$2
    local edition=$3
    local navigation=$4
    local duration_ms=$5
    local platform=${6:-FM Towns}
    local home="$scratch/$game_id-home"
    local log="$scratch/$game_id.log"
    local probe="$scratch/$game_id-runtime.json"
    local archive_hash

    mkdir -p "$home"
    archive_hash=$(firestaff_sha256_file "$archive")
    env -u FIRESTAFF_EXIT_AFTER_LAUNCH \
        FIRESTAFF_CONFIG_PATH="$home/config.toml" \
        FIRESTAFF_DATA="$data_root" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
        HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$data_root" \
            --script "$navigation" --duration "$duration_ms" >"$log" 2>&1 || {
                cat "$log" >&2
                return 1
            }

    if ! grep -Fq 'startup game=none mode=menu' "$log" ||
       ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition source=" "$log"; then
        printf 'FAIL: no-game M12 menu did not select and start authentic %s %s media\n' \
            "$game_id" "$edition" >&2
        cat "$log" >&2
        return 1
    fi

    python3 "$(dirname "$0")/verify_main_menu_runtime_probe.py" "$game_id" "$probe"

    if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
        printf 'FAIL: original %s archive changed during menu startup\n' "$game_id" >&2
        return 1
    fi
}

run_selected_main_menu_runtime_case() {
    local game_id=$1
    shift
    if [[ "$selected_game" == all || "$selected_game" == "$game_id" ]]; then
        run_main_menu_runtime_case "$game_id" "$@"
    fi
}

run_theron_direct_selection_case() {
    local edition=$1
    local track02=$2
    local case_root="$scratch/theron-direct-$edition-data"
    local home="$scratch/theron-direct-$edition-home"
    local log="$scratch/theron-direct-$edition.log"
    local probe="$scratch/theron-direct-$edition-runtime.json"
    local expected_edition=pce-en
    local media_hash
    if [[ "$edition" == jp ]]; then
        expected_edition=pce-jp
    fi
    mkdir -p "$case_root/theron"
    ln -s "$track02" "$case_root/theron/$(basename "$track02")"
    mkdir -p "$home"
    media_hash=$(firestaff_sha256_file "$track02")
    env -u FIRESTAFF_EXIT_AFTER_LAUNCH \
        FIRESTAFF_CONFIG_PATH="$home/config.toml" \
        FIRESTAFF_DATA="$case_root" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
        HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$case_root" \
            --script 'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait:20' \
            --duration 30000 >"$log" 2>&1 || {
                cat "$log" >&2
                return 1
            }
    if ! grep -Fq 'startup game=none mode=menu' "$log" ||
       ! grep -Fq "launch phase=game-handoff mode=menu game=theron platform=PC Engine edition=$expected_edition source=" "$log"; then
        printf 'FAIL: one Theron selection did not start authentic %s Track 02 media\n' \
            "$edition" >&2
        cat "$log" >&2
        return 1
    fi
    python3 "$(dirname "$0")/verify_main_menu_runtime_probe.py" \
        "theron-direct-$edition" "$probe"
    if [[ $(firestaff_sha256_file "$track02") != "$media_hash" ]]; then
        printf 'FAIL: original Theron %s Track 02 BIN changed during direct selection\n' \
            "$edition" >&2
        return 1
    fi
}

run_selected_main_menu_runtime_case dm1 "$dm1_archive" fmtowns-en \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' 60000
run_selected_main_menu_runtime_case csb "$csb_archive" fmtowns-en \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' 150000
run_selected_main_menu_runtime_case dm2 "$dm2_archive" fmtowns-ja \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait:2400,click:115:65,click:100:60' 90000

if [[ "$selected_game" == all || "$selected_game" == theron ]]; then
  if [[ -f "$theron_us_track02" ]]; then
    run_theron_direct_selection_case us "$theron_us_track02"
    run_main_menu_runtime_case theron "$theron_us_track02" pce-en \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait:30,enter,enter,down,down,down,down,down,down,enter,down,enter,tab' \
        30000 'PC Engine'
  else
    echo 'SKIP: authentic Theron US Track 02 is unavailable'
  fi
  if [[ -f "$theron_jp_track02" ]]; then
    run_theron_direct_selection_case jp "$theron_jp_track02"
  else
    echo 'SKIP: authentic Theron JP Track 02 is unavailable'
  fi
fi
