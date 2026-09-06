#!/bin/sh
set -eu

# A diagnostic developer opt-in must not turn this native archive test
# into a test of external extraction tools.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

firestaff_cli="${1:?Firestaff executable is required}"
media_path="${FIRESTAFF_CSB_ATARI_NESTED_ZIP:-$HOME/.firestaff/data/csb/chaos_strikes_back_ftl.zip}"

if [ ! -x "$firestaff_cli" ] || [ ! -f "$media_path" ]; then
    echo "SKIP: nested CSB Atari ST campaign archive or Firestaff executable is unavailable"
    exit 77
fi

title_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1)" || {
    printf '%s\n' "$title_output" >&2
    exit 1
}
case "$title_output" in
    # The nested ZIP preserves the same Atari ST ANIMATE.SCR title owner as
    # the loose STX dump.  It is not a generic already-loaded CSB title.
    *sourceId=csb*assetMd5=ebf6a57af3f27782e358c0490bfd2f2e*phase=csb-atari-st-animation*startupAnimation=animate-scr*titleReady=1*levelLoaded=0*) ;;
    *)
        echo "FAIL: nested CSB Atari archive did not reach the source title"
        printf '%s\n' "$title_output" >&2
        exit 1
        ;;
esac

runtime_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script enter \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}
case "$runtime_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*runtimeTick=*) ;;
    *)
        echo "FAIL: nested CSB Atari title Enter did not reach runtime"
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac

movement_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script 'enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
    printf '%s\n' "$movement_output" >&2
    exit 1
}
case "$movement_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*party=9,1,2*runtimeTick=*) ;;
    *)
        echo "FAIL: nested CSB Atari start menu/title input did not consume first UP movement"
        printf '%s\n' "$movement_output" >&2
        exit 1
        ;;
esac

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --platform atari-st --data-dir "$media_path" \
    --script enter,enter,enter --duration 2000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
case "$menu_output" in
    *"CSB READY: gameId=csb"*csb-st20-21-en*route=startup*) ;;
    *)
        echo "FAIL: nested CSB Atari start-menu launch did not retain media"
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB ZIP -> ZIP -> STX title, runtime movement, and start-menu launch"
