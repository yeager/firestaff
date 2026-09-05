#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR:-$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip}"
language="${FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE:-en}"
user_save="${FIRESTAFF_CSB_FMTOWNS_USER_SAVE:-}"
edition_arg=""
expected_media="$data_dir"

# The F31 package is admitted by Firestaff's native ZIP/CD readers.  This
# regression must not inherit the optional external archive scan facility.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

# This remains opt-in because the F31 CD image is licensed game material.
# Do not infer a FM Towns package from a generic CSB root: the scanner must
# authenticate the selected original media itself.
if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 77
fi
if [ ! -e "$data_dir" ]; then
    echo "SKIP: local CSB FM Towns data is unavailable: $data_dir"
    exit 77
fi
if [ -f "$data_dir" ]; then
    # The F31 title/game package is consumed directly from this archive.  Do
    # not let a native start, input, or menu route rewrite supplied media.
    media_hash_before=$(sha256sum "$data_dir")
else
    media_hash_before=""
fi
if [ "$language" = "ja" ]; then
    edition_arg="--csb-fmtowns-ja"
elif [ "$language" != "en" ]; then
    echo "SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE must be en or ja"
    exit 77
fi

# F31's AUTOEXEC route owns TITLE.ANM, SWITCHTW and the Game selection before
# CHTWE enters the original MINI.DAT campaign.  The first receipt proves the
# direct CLI route did not fall back to a PC/Amiga title; the second advances
# the real native sequence without injecting a synthetic save or input.
title_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg --boot-probe \
    --boot-probe-frames 2 --boot-probe-expect-startup-active 1 \
    --boot-probe-expect-runtime-tick-max 0 --duration 0 2>&1)" || {
    printf '%s\n' "$title_output" >&2
    exit 1
}

case "$title_output" in
    *"dataDir="*"$expected_media"*"phase=csb-fmtowns-title"*"startupActive=1"*"startupAnimation=title-anm"*"levelLoaded=0"*) ;;
    *)
        echo "FAIL: native FM Towns CSB CLI boot did not reach TITLE.ANM" >&2
        printf '%s\n' "$title_output" >&2
        exit 1
        ;;
esac

switch_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg --boot-probe \
    --boot-probe-frames 700 --boot-probe-expect-phase csb-fmtowns-switch \
    --boot-probe-expect-startup-active 1 \
    --duration 0 2>&1)" || {
    printf '%s\n' "$switch_output" >&2
    exit 1
}

case "$switch_output" in
    *"phase=csb-fmtowns-switch"*"startupActive=1"*"levelLoaded=0"*"party=-1,-1,-1"*"champions=-1"*) ;;
    *)
        echo "FAIL: native FM Towns CSB CLI mislabeled modal SWITCHTW as a dungeon" >&2
        printf '%s\n' "$switch_output" >&2
        exit 1
        ;;
esac

# Drive the actual 320x200 SWITCHTW Game rectangle and C004 Enter rectangle.
# Only that source path may promote the checksum-verified MINI.DAT state to a
# live dungeon.  The retail seed is HALK at map 4/(22,18), not the map-0
# bootstrap dungeon which used to leak through the CLI receipt.
for mode in v1 v21; do
case "$mode" in v1) expected_mode=0;; v21) expected_mode=2;; esac
runtime_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --presentation-mode "$mode" \
    --width 320 --height 200 --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
    --boot-probe --boot-probe-frames 1200 \
    --script 'wait700,click:52:110,wait10,click:250:50,wait240' \
    --boot-probe-expect-phase inactive --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --boot-probe-expect-map 4 \
    --boot-probe-expect-party 22,18,2 --duration 0 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}

case "$runtime_output" in
    *"phase=inactive"*"levelLoaded=1"*"map=4"*"party=22,18,2"*"champions=1"*"csbViewportHash="*) ;;
    *)
        echo "FAIL: native FM Towns CSB CLI did not retain authentic MINI.DAT state" >&2
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac

if ! printf '%s\n' "$runtime_output" | grep -Fq "presentationMode=$expected_mode "; then
    printf '%s\n' "$runtime_output" >&2; exit 1
fi
done

# An explicit F31 save is a distinct C03/F0435 route.  It must not replay
# TITLE.ANM or pass the bytes to the Atari/CSBWin importer merely because the
# launcher was started from a generic --game csb invocation.  Keep this opt-in
# because a save is language-owned and the test may not manufacture one.
if [ -n "$user_save" ]; then
    if [ ! -f "$user_save" ]; then
        echo "FAIL: requested F31 user save is unavailable: $user_save" >&2
        exit 1
    fi
    resume_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
        --save "$user_save" --boot-probe --boot-probe-expect-phase inactive \
        --boot-probe-expect-runtime --boot-probe-expect-startup-active 0 \
        --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
        printf '%s\n' "$resume_output" >&2
        exit 1
    }
    case "$resume_output" in
        *"phase=inactive"*"startupActive=0"*"levelLoaded=1"*) ;;
        *)
            echo "FAIL: explicit F31 save did not resume through C03/F0435" >&2
            printf '%s\n' "$resume_output" >&2
            exit 1
            ;;
    esac
    echo "PASS: native CSB FM Towns CLI resumes the selected F0435 save"
fi

# boot-probe intentionally runs the direct launch path, so it cannot prove the
# regular launcher route. Exercise that route separately: --menu retains the
# selected CSB row, and Enter must admit the source-owned F31 TITLE.ANM
# boundary. SWITCHTW/MINI.DAT is a later, separately tested input route.
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
    --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}

case "$menu_output" in
    *"CSB READY: gameId=csb"*"dataDir="*"$expected_media"*"variant=csb-fmtowns-$language"*"route=startup"*"handoff=f31-title-anm"*"handoffHash="*)
        if ! printf '%s\n' "$menu_output" | grep -Eq 'handoffHash=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]'; then
            echo "FAIL: CSB FM Towns start-menu Enter did not publish TITLE.ANM source receipt" >&2
            printf '%s\n' "$menu_output" >&2
            exit 1
        fi
        ;;
    *)
        echo "FAIL: CSB FM Towns start-menu Enter did not request native launch" >&2
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac

# Exercise the visible mouse-only card flow as well as the keyboard script.
# CSB's catalogue lists FM Towns before Amiga and Atari, so the first platform
# card is the authenticated F31 package.  This proves that an explicit F31J
# request survives game card -> FM Towns card -> Original card navigation;
# no keyboard token chooses the platform or presentation here.
mouse_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --width 1920 --height 1080 --menu --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
    --script 'click:1000:200,click:400:400,click:300:400' --duration 2000 2>&1)" || {
    printf '%s\n' "$mouse_output" >&2
    exit 1
}
case "$mouse_output" in
    *"CSB READY: gameId=csb"*"dataDir="*"$expected_media"*"variant=csb-fmtowns-$language"*"route=startup"*"handoff=f31-title-anm"*) ;;
    *)
        echo "FAIL: CSB FM Towns mouse card flow did not preserve the selected native edition" >&2
        printf '%s\n' "$mouse_output" >&2
        exit 1
        ;;
esac

if [ -n "$user_save" ]; then
    menu_resume_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy "$firestaff_cli" \
        --menu --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
        --save "$user_save" --script enter,enter,enter --duration 1000 2>&1)" || {
        printf '%s\n' "$menu_resume_output" >&2
        exit 1
    }
    case "$menu_resume_output" in
        *"CSB READY: gameId=csb"*"variant=csb-fmtowns-$language"*"route=f0435-resume"*"handoff=f31-f0435-resume"*"handoffHash="*)
            if ! printf '%s\n' "$menu_resume_output" | grep -Eq 'handoffHash=[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]'; then
                echo "FAIL: start-menu resume did not publish its F0435 source receipt" >&2
                printf '%s\n' "$menu_resume_output" >&2
                exit 1
            fi
            ;;
        *)
            echo "FAIL: start-menu launch dropped the explicit F31 save" >&2
            printf '%s\n' "$menu_resume_output" >&2
            exit 1
            ;;
    esac
    echo "PASS: native CSB FM Towns start menu resumes the selected F0435 save"
fi

if [ -n "$media_hash_before" ] && [ "$media_hash_before" != "$(sha256sum "$data_dir")" ]; then
    echo "FAIL: native FM Towns routes modified supplied game media" >&2
    exit 1
fi

echo "PASS: native CSB FM Towns CLI title, MINI.DAT runtime/movement, and start-menu launch"
