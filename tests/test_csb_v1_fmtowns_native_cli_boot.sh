#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR:-$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip}"
language="${FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE:-en}"
user_save="${FIRESTAFF_CSB_FMTOWNS_USER_SAVE:-}"
edition_arg=""
expected_media="$data_dir"

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

runtime_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg --boot-probe \
    --boot-probe-frames 2000 --boot-probe-expect-phase inactive \
    --boot-probe-expect-runtime --boot-probe-expect-party 9,0,2 \
    --duration 0 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}

case "$runtime_output" in
    *"phase=inactive"*"startupActive=0"*"levelLoaded=1"*"party=9,0,2"*) ;;
    *)
        echo "FAIL: native FM Towns CSB CLI boot did not reach MINI.DAT runtime" >&2
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac

# F31's native MINI.DAT route must leave the title/utility program chain and
# feed its first real movement command into the shared CSB runtime. The stock
# English corpus starts at (9,0), facing south; one source UP command reaches
# (9,1) without a fabricated save or a PC/Atari fallback.
movement_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --width 320 --height 200 --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
    --boot-probe --boot-probe-frames 2000 --script up \
    --boot-probe-expect-phase inactive --boot-probe-expect-runtime \
    --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
    printf '%s\n' "$movement_output" >&2
    exit 1
}

case "$movement_output" in
    *"phase=inactive"*"levelLoaded=1"*"party=9,1,2"*"dm1WorldTick="*) ;;
    *)
        echo "FAIL: native FM Towns CSB runtime did not consume its first UP command" >&2
        printf '%s\n' "$movement_output" >&2
        exit 1
        ;;
esac

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

echo "PASS: native CSB FM Towns CLI title, MINI.DAT runtime/movement, and start-menu launch"
