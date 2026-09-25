#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
media_path="${FIRESTAFF_CSB_ATARI_STX:-$HOME/.firestaff/data/csb/Chaos Strikes Back.stx}"

# STX is decoded by Firestaff's native reader.  Do not allow an inherited
# diagnostic external-archive opt-in to mask that production contract.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$firestaff_cli" ] || [ ! -e "$media_path" ]; then
    echo "SKIP: CSB Atari ST campaign media or Firestaff executable is unavailable"
    exit 77
fi

# "atari" is the public spelling for the Atari ST source route.
title_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari --data-dir "$media_path" \
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
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script 'enter,up' \
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
        echo "FAIL: native CSB Atari ST title/runtime input did not consume first UP movement"
        printf '%s\n' "$movement_output" >&2
        exit 1
        ;;
esac

# Every input probe starts from the same verified STX title/runtime sequence.
# This preserves the original starting cell as the comparison anchor instead
# of letting a preceding command change the context for a later assertion.
probe_runtime_input() {
    input=$1
    expected_party=$2
    input_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --platform atari-st --data-dir "$media_path" \
        --boot-probe --boot-probe-frames 180 --script "enter,$input" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1)" || {
        printf '%s\n' "$input_output" >&2
        exit 1
    }
    case "$input_output" in
        *phase=inactive*startupActive=0*levelLoaded=1*"map=0 party=$expected_party"*runtimeTick=*)
            if printf '%s\n' "$input_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'; then
                return 0
            fi
            ;;
    esac
    printf '%s\n' "$input_output" >&2
    printf 'FAIL: native CSB Atari ST %s input did not preserve its source runtime receipt\n' "$input" >&2
    exit 1
}

# The stock original campaign begins at (9,0) facing south.  These are
# observed first-command results from independent original STX sessions.  The
# unchanged strafe/action positions are intentional evidence, never a prompt
# to fabricate a nearby object, door, or save state.
probe_runtime_input down         9,0,2
probe_runtime_input left         9,0,1
probe_runtime_input right        9,0,3
probe_runtime_input strafe-left  9,0,2
probe_runtime_input strafe-right 9,0,2
probe_runtime_input action       9,0,2

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
        echo "FAIL: CSB Atari ST start-menu Enter did not retain campaign media"
        printf '%s\n' "$menu_output" >&2
        exit 1
        ;;
esac
if printf '%s\n' "$menu_output" | grep -q 'handoffHash=00000000'; then
    echo "FAIL: CSB Atari ST start-menu launch did not retain a source package identity"
    printf '%s\n' "$menu_output" >&2
    exit 1
fi

# With the nonexistent PC edition removed, Atari ST is the third card on the
# platform picker's first row. Select
# CSB, that card, and Original solely through pointer events on the explicit
# launcher canvas, then require the ordinary native launch exit.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --width 1920 --height 1080 --menu --game csb --platform atari-st \
    --data-dir "$media_path" \
    --script 'wait20,click:1173:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

for mode in v1 v21; do
    case "$mode" in v1) expected_mode=0;; v21) expected_mode=2;; esac
    mode_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --platform atari-st --data-dir "$media_path" --script enter \
        --presentation-mode "$mode" --boot-probe --boot-probe-frames 180 \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1)" || {
        printf '%s\n' "$mode_output" >&2; exit 1;
    }
    if ! printf '%s\n' "$mode_output" | grep -Fq "presentationMode=$expected_mode "; then
        printf '%s\n' "$mode_output" >&2
        printf 'FAIL: CSB Atari CLI did not preserve %s\n' "$mode" >&2
        exit 1
    fi
done

# --boot-probe intentionally rejects --menu. Verify that the normal M12 ->
# M11 path advances through the authentic Atari ST animation and reaches its
# dungeon entrance. The retained ANIMATE.SCR sequence takes about 30 seconds
# in the normal loop; a short probe would only prove that the title appeared.
case "$firestaff_cli" in
    */*) app_dir=${firestaff_cli%/*} ;;
    *) app_dir=. ;;
esac
menu_probe="$app_dir/csb-atari-menu-runtime-$$.json"
trap 'rm -f "$menu_probe"' EXIT HUP INT TERM
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --menu --game csb --platform atari-st --data-dir "$media_path" \
    --script 'enter,enter,enter' --duration 30000 >/dev/null 2>&1
python3 - "$menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or startup["receiptReady"] != 1 or
        startup["phase"] != "csb-entrance-4" or startup["active"] != 1 or
        startup["startupActive"] != 1 or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 0, 2, 0)):
    raise SystemExit(f"FAIL: authentic CSB Atari start menu did not reach its source dungeon entrance: {probe}")
print("PASS: authentic CSB Atari start menu advanced through ANIMATE.SCR to its source dungeon entrance")
PY
echo "PASS: native CSB Atari ST campaign title, input matrix, Original/Modern CLI, and menu dungeon entrance"
