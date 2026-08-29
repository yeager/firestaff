#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
media_path="${FIRESTAFF_CSB_ATARI_STX:-$HOME/.firestaff/data/csb/Chaos Strikes Back.stx}"

if [ ! -x "$firestaff_cli" ] || [ ! -e "$media_path" ]; then
    echo "SKIP: CSB Atari ST campaign media or Firestaff executable is unavailable"
    exit 77
fi

title_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 60 2>&1)" || {
    printf '%s\n' "$title_output" >&2
    exit 1
}
case "$title_output" in
    # The original STX route first owns the retained ANIMATE.SCR title
    # sequence. It must expose the real title as ready, but must not invent a
    # loaded dungeon before the user accepts it.
    *sourceId=csb*assetMd5=ebf6a57af3f27782e358c0490bfd2f2e*phase=csb-atari-st-animation*startupAnimation=animate-scr*titleReady=1*levelLoaded=0*) ;;
    *)
        echo "FAIL: native CSB Atari ST media did not reach its source title"
        printf '%s\n' "$title_output" >&2
        exit 1
        ;;
esac

runtime_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script enter \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}
case "$runtime_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*runtimeTick=*)
        if ! printf '%s\n' "$runtime_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'; then
            echo "FAIL: native CSB Atari ST runtime did not publish its source viewport receipt"
            printf '%s\n' "$runtime_output" >&2
            exit 1
        fi
        ;;
    *)
        echo "FAIL: native CSB Atari ST title Enter did not reach runtime"
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac

movement_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 800 --script 'enter,key:enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
    printf '%s\n' "$movement_output" >&2
    exit 1
}
case "$movement_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*party=9,1,2*runtimeTick=*)
        if ! printf '%s\n' "$movement_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'; then
            echo "FAIL: native CSB Atari ST movement did not retain its source viewport receipt"
            printf '%s\n' "$movement_output" >&2
            exit 1
        fi
        ;;
    *)
        echo "FAIL: native CSB Atari ST start menu/title input did not consume first UP movement"
        printf '%s\n' "$movement_output" >&2
        exit 1
        ;;
esac

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --platform atari-st --data-dir "$media_path" \
    --script enter --duration 2000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
case "$menu_output" in
    *"CSB READY: gameId=csb"*csb-st20-21-en*route=startup*) ;;
    *)
        echo "FAIL: CSB Atari ST start-menu Enter did not retain campaign media"
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac

echo "PASS: native CSB Atari ST campaign title, runtime movement, and start-menu launch"
