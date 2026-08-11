#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR:-}"
language="${FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE:-en}"
edition_arg=""
expected_cache="csb-fmtowns-en"

# This remains opt-in because the F31 CD image is licensed game material.
# Do not infer a FM Towns package from a generic CSB root: the scanner must
# authenticate the selected original media itself.
if [ -z "$data_dir" ]; then
    echo "SKIP: set FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR to original F31 media"
    exit 0
fi
if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 0
fi
if [ ! -e "$data_dir" ]; then
    echo "SKIP: local CSB FM Towns data is unavailable: $data_dir"
    exit 0
fi
if [ "$language" = "ja" ]; then
    edition_arg="--csb-fmtowns-ja"
    expected_cache="csb-fmtowns-ja"
elif [ "$language" != "en" ]; then
    echo "SKIP: FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE must be en or ja"
    exit 0
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
    *"dataDir="*"$expected_cache"*"phase=csb-fmtowns-title"*"startupActive=1"*"startupAnimation=title-anm"*"levelLoaded=0"*) ;;
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

# boot-probe intentionally runs the direct launch path, so it cannot prove the
# regular launcher route. Exercise that route separately: --menu retains the
# selected CSB row, and Enter must request the native F31 handoff.
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --data-dir "$data_dir" --platform fm-towns $edition_arg \
    --script enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}

case "$menu_output" in
    *"CSB READY: gameId=csb"*"dataDir="*"$expected_cache"*) ;;
    *)
        echo "FAIL: CSB FM Towns start-menu Enter did not request native launch" >&2
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB FM Towns CLI title, MINI.DAT runtime, and start-menu launch"
