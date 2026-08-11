#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR:-}"

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

# F31's AUTOEXEC route owns TITLE.ANM, SWITCHTW and the Game selection before
# CHTWE enters the original MINI.DAT campaign.  The first receipt proves the
# direct CLI route did not fall back to a PC/Amiga title; the second advances
# the real native sequence without injecting a synthetic save or input.
title_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform fm-towns --boot-probe \
    --boot-probe-frames 2 --boot-probe-expect-startup-active 1 \
    --boot-probe-expect-runtime-tick-max 0 --duration 0 2>&1)" || {
    printf '%s\n' "$title_output" >&2
    exit 1
}

case "$title_output" in
    *"phase=csb-fmtowns-title"*"startupActive=1"*"startupAnimation=title-anm"*"levelLoaded=0"*) ;;
    *)
        echo "FAIL: native FM Towns CSB CLI boot did not reach TITLE.ANM" >&2
        printf '%s\n' "$title_output" >&2
        exit 1
        ;;
esac

runtime_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform fm-towns --boot-probe \
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

echo "PASS: native CSB FM Towns CLI title and MINI.DAT runtime boot"
