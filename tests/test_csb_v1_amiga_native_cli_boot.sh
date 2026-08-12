#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"

# This is an opt-in real-media check.  The generic per-game data directory
# may contain FM Towns, DOS, or unrelated Amiga archives, none of which is
# sufficient evidence for the A31 native startup route.  Do not infer an
# A31 root merely because ~/.firestaff/data/csb exists; the caller must name
# the curated original A31 directory explicitly.
data_dir="${FIRESTAFF_CSB_AMIGA31_DATA_DIR:-}"

if [ -z "$data_dir" ]; then
    echo "SKIP: set FIRESTAFF_CSB_AMIGA31_DATA_DIR to curated original A31 media"
    exit 0
fi

if [ ! -x "$firestaff_cli" ]; then
    echo "SKIP: Firestaff executable is unavailable"
    exit 0
fi
if [ ! -e "$data_dir" ]; then
    echo "SKIP: local CSB Amiga data is unavailable: $data_dir"
    exit 0
fi

# This is an opt-in real-media check.  The scanner remains the only authority
# on the selected A31 package; the test supplies neither fixture bytes nor a
# substitute PC34 asset.  A31M takes APPA.C -> ANIM.C's TITL.DAT phase before
# KAOS.FTL / F0441 enters Prison; A31E instead uses APPB/BJELoad_R's direct
# C03_GAME -> F0441 handoff.  ReDMCSB COMPILE.H:199-213, 246-269.
probe_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --game csb --data-dir "$data_dir" --platform amiga --boot-probe 2>&1)" || {
    printf '%s\n' "$probe_output" >&2
    exit 1
}

case "$probe_output" in
    *"phase=csb-amiga-a31-titl"*"startupActive=1"*"startupAnimation=titl-dat"*"levelLoaded=0"*) ;;
    *"phase=csb-entrance-0"*"startupActive=1"*"startupAnimation=csb-entrance"*"levelLoaded=1"*) ;;
    *)
        echo "FAIL: native Amiga CSB CLI boot did not reach its source startup phase" >&2
        printf '%s\n' "$probe_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB Amiga CLI boot reaches its source startup phase"

# The direct CLI path must also be able to leave the authentic Amiga title
# owner and run the selected campaign.  The two inputs are intentionally
# source-visible input events, rather than a host shortcut: A31M reaches its
# TITL.DAT selector before KAOS/ANIM/APPB and consumes the source English-box
# click at (100,100); A31E reaches C03_GAME through APPB/BJELoad_R, where that
# point is inert.  Both original routes then accept the bounded
# continue/move sequence and must cross into the verified runtime without a
# generated save or a PC34 title substitution.
runtime_output="$(SDL_VIDEODRIVER=dummy "$firestaff_cli" \
    --width 320 --height 200 --game csb --data-dir "$data_dir" --platform amiga --boot-probe \
    --boot-probe-frames 800 --script 'click:100:100,key:enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-startup-active 0 \
    --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-runtime-tick-min 1 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}

case "$runtime_output" in
    *"phase=inactive"*"startupActive=0"*"levelLoaded=1"*"party=9,1,2"*"runtimeTick="*) ;;
    *)
        echo "FAIL: native Amiga CSB CLI route did not consume its first UP movement" >&2
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB Amiga CLI title input reaches verified runtime movement"
