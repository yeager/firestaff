#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
data_dir="${FIRESTAFF_CSB_AMIGA31_DATA_DIR:-$HOME/.firestaff/data/csb}"

if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 0
fi
if [ ! -d "$data_dir" ]; then
    echo "SKIP: local CSB Amiga data is unavailable: $data_dir"
    exit 0
fi

# This is an opt-in real-media check.  The scanner remains the only authority
# on the selected A31 package; the test supplies neither fixture bytes nor a
# substitute PC34 asset.  ReDMCSB APPA.C -> ANIM.C owns the expected TITL.DAT
# phase before KAOS.FTL / F0441 may enter Prison.
probe_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform amiga --boot-probe 2>&1)" || {
    printf '%s\n' "$probe_output" >&2
    exit 1
}

case "$probe_output" in
    *"phase=csb-amiga-a31-titl"*"startupActive=1"*"startupAnimation=titl-dat"*"levelLoaded=0"*) ;;
    *)
        echo "FAIL: native Amiga CSB CLI boot did not reach TITL.DAT" >&2
        printf '%s\n' "$probe_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB Amiga A31 CLI boot reaches TITL.DAT"
