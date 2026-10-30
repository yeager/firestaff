#!/bin/sh
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
media_path="${FIRESTAFF_CSB_ATARI_STX:-$HOME/.firestaff/data/csb/Chaos Strikes Back.stx}"
archive_path="${FIRESTAFF_CSB_ATARI_STX_ARCHIVE:-$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z}"
archive_data_root="${FIRESTAFF_CSB_ATARI_ARCHIVE_DATA_ROOT:-}"
scratch_root="${FIRESTAFF_TEST_SCRATCH:-$HOME/.codex-scratch}"
menu_probe=""
runtime_screenshot_dir=""
hud_screenshot_dir=""

cleanup() {
    if [ -n "$menu_probe" ]; then rm -f "$menu_probe"; fi
    if [ -n "$runtime_screenshot_dir" ]; then rm -rf "$runtime_screenshot_dir"; fi
    if [ -n "$hud_screenshot_dir" ]; then rm -rf "$hud_screenshot_dir"; fi
}
trap cleanup EXIT HUP INT TERM

# STX is decoded by Firestaff's native reader.  Do not allow an inherited
# diagnostic external-archive opt-in to mask that production contract.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -e "$media_path" ] && [ -e "$archive_path" ]; then
    media_path="$archive_path"
fi

if [ ! -x "$firestaff_cli" ] || [ ! -e "$media_path" ]; then
    echo "SKIP: CSB Atari ST campaign media/archive or Firestaff executable is unavailable"
    exit 77
fi

# "atari" is the public spelling for the Atari ST source route.
title_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 2 2>&1)" || {
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

mkdir -p "$scratch_root"
runtime_screenshot_dir="$(mktemp -d "$scratch_root/firestaff-csb-atari-runtime.XXXXXX")"
runtime_output="$(FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$runtime_screenshot_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script enter \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 2>&1)" || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}
case "$runtime_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*map=0*party=9,0,2*runtimeTick=*)
        if ! printf '%s\n' "$runtime_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*'; then
            echo "FAIL: native CSB Atari ST runtime did not publish its source viewport receipt"
            printf '%s\n' "$runtime_output" >&2
            exit 1
        fi
        ;;
    *)
        echo "FAIL: native CSB Atari ST title Enter did not reach its original map-0 start pose"
        printf '%s\n' "$runtime_output" >&2
        exit 1
        ;;
esac
python3 - "$runtime_screenshot_dir" <<'PY'
import glob
import os
import struct
import sys

screenshots = glob.glob(os.path.join(sys.argv[1], "*.bmp"))
if len(screenshots) != 1:
    raise SystemExit("FAIL: Atari runtime screenshot was not captured exactly once")
with open(screenshots[0], "rb") as image:
    header = image.read(54)
    if len(header) != 54 or header[:2] != b"BM":
        raise SystemExit("FAIL: Atari runtime screenshot is not a BMP")
    pixel_offset = struct.unpack_from("<I", header, 10)[0]
    width, signed_height = struct.unpack_from("<ii", header, 18)
    bits_per_pixel = struct.unpack_from("<H", header, 28)[0]
    compression = struct.unpack_from("<I", header, 30)[0]
    height = abs(signed_height)
    if (width, height, bits_per_pixel, compression) != (320, 200, 24, 0):
        raise SystemExit("FAIL: Atari runtime screenshot has an unexpected pixel layout")
    stride = (width * 3 + 3) & ~3
    image.seek(pixel_offset)
    rows = [image.read(stride) for _ in range(height)]
    if any(len(row) != stride for row in rows):
        raise SystemExit("FAIL: Atari runtime screenshot is truncated")

    def pixel(x, y):
        storage_y = height - 1 - y if signed_height > 0 else y
        offset = x * 3
        b, g, r = rows[storage_y][offset:offset + 3]
        return (r, g, b)

    # The Atari viewport is (48,33)..(271,168). This upper-left strip is
    # outside both F0128 and the verified C232 HUD; it must not retain the
    # colorful final ANIMATE.SCR page after the FTLCODE handoff.
    background = pixel(0, 0)
    if any(pixel(x, y) != background for y in range(33) for x in range(48)):
        raise SystemExit("FAIL: Atari title pixels leaked outside the runtime viewport")
    viewport_colors = {pixel(x, y) for y in range(33, 169) for x in range(48, 272)}
    if len(viewport_colors) < 4:
        raise SystemExit("FAIL: Atari runtime viewport was not drawn in the screenshot")
print("PASS: Atari handoff clears the retained title outside the original viewport")
PY

movement_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game csb --platform atari-st --data-dir "$media_path" \
    --boot-probe --boot-probe-frames 180 --script 'enter,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 --duration 0 2>&1)" || {
    printf '%s\n' "$movement_output" >&2
    exit 1
}
case "$movement_output" in
    *phase=inactive*startupActive=0*levelLoaded=1*map=0*party=9,1,2*runtimeTick=*)
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

# The installed-library route is the normal user-facing --data-dir shape:
# select Atari from the shared CSB root, where the campaign STX is nested in
# the bilingual ZIP and ANIMATE.SCR/DAT is in the sibling retail archive.
# Require the original script identity and its source-owned 2,036-VBlank
# handoff, so a generic two-second fallback can never pass this regression.
if [ -n "$archive_data_root" ] && [ -d "$archive_data_root" ]; then
    data_root_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --debug --game csb --platform atari \
        --data-dir "$archive_data_root" \
        --boot-probe --boot-probe-frames 2 --script 'wait:2036' \
        --boot-probe-expect-runtime --duration 0 2>&1)" || {
        printf '%s\n' "$data_root_output" >&2
        exit 1
    }
    printf '%s\n' "$data_root_output" | grep -Fq \
        'csb-atari-animation' || {
        echo "FAIL: CSB Atari shared data-root launch did not bind its source animation" >&2
        printf '%s\n' "$data_root_output" >&2
        exit 1
    }
    printf '%s\n' "$data_root_output" | grep -Fq 'waited-vbl=2036' || {
        echo "FAIL: CSB Atari shared data-root launch lost the original 2,036-VBlank duration" >&2
        printf '%s\n' "$data_root_output" >&2
        exit 1
    }
    case "$data_root_output" in
        *assetMd5=ebf6a57af3f27782e358c0490bfd2f2e*phase=inactive*startupActive=0*levelLoaded=1*map=0*party=9,0,2*) ;;
        *)
            echo "FAIL: CSB Atari shared data-root launch did not hand off to the original runtime" >&2
            printf '%s\n' "$data_root_output" >&2
            exit 1
            ;;
    esac

    # A normal direct launch from the same shared library must run only the
    # selected Atari startup. The CSB data root also contains an Amiga ADF;
    # its APPA.C SWSH.FTL prelude must not be inserted before Atari ANIMATE.SCR.
    direct_intro_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout 15s \
        "$firestaff_cli" --debug --game csb --platform atari \
        --data-dir "$archive_data_root" --duration 2500 2>&1)" || {
        printf '%s\n' "$direct_intro_output" >&2
        exit 1
    }
    printf '%s\n' "$direct_intro_output" | grep -Fq \
        'csb-atari-animation' || {
        echo "FAIL: normal CSB Atari launch did not select ANIMATE.SCR" >&2
        printf '%s\n' "$direct_intro_output" >&2
        exit 1
    }
    if printf '%s\n' "$direct_intro_output" | grep -Fq \
        'phase=amiga-swsh-ftl'; then
        echo "FAIL: CSB Atari launch incorrectly played the sibling Amiga SWSH.FTL prelude" >&2
        printf '%s\n' "$direct_intro_output" >&2
        exit 1
    fi
fi

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

# Keep the authenticated first-champion route in the production CLI loop.
# Explicit wait:32 tokens advance host loop frames between commands, including
# after title acceptance; no pose or save state is injected by the test.
atari_two_champion_route='enter,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,strafe-left,wait:32,right,wait:32,right,wait:32,click:320:164,wait:32,click:260:230,wait:32,strafe-left,wait:32,right,wait:32,up,wait:32,up,wait:32,right,wait:32,up,wait:32,left,wait:32,click:320:164,wait:32,click:260:230'
if [ -e "$archive_path" ]; then
    champion_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --debug --width 640 --height 400 --game csb --platform atari-st --data-dir "$archive_path" \
        --boot-probe --boot-probe-frames 2 \
        --script "$atari_two_champion_route,key:f1,key:f2" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1)" || {
        printf '%s\n' "$champion_output" >&2
        exit 1
    }
    case "$champion_output" in
        *assetMd5=ebf6a57af3f27782e358c0490bfd2f2e*phase=inactive*levelLoaded=1*'party=11,9,1'*'champions=2'*dm1HocCandidatePanel=0*dm1HocCandidateOrdinal=-1*dm1InventoryPanel=1*) ;;
        *)
            echo "FAIL: authentic Atari CLI route did not recruit both C127 champions, resume movement, and open the F1/F2 inventory"
            printf '%s\n' "$champion_output" >&2
            exit 1
            ;;
    esac
    # The final receipt is taken after both champions have joined and the
    # candidate panel has closed, so its ordinal is correctly -1. The
    # original-media M11 handoff test checks C127 ordinal 4 while that panel
    # is actually open; this CLI path checks the completed transaction.
    printf '%s\n' "$champion_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*' || {
        echo "FAIL: authentic Atari CLI route lost its viewport receipt after second-recruit movement" >&2
        printf '%s\n' "$champion_output" >&2
        exit 1
    }
    # The transaction above ends with the two inventory panels open. Capture
    # the champion row in a separate authentic run before opening inventory;
    # a post-F1/F2 screenshot cannot prove the underlying F0292 hand slots.
    hud_screenshot_dir="$(mktemp -d "$scratch_root/firestaff-csb-atari-hud.XXXXXX")"
    hud_output="$(FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$hud_screenshot_dir" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --debug --width 640 --height 400 --game csb --platform atari-st --data-dir "$archive_path" \
        --boot-probe --boot-probe-frames 2 \
        --script "$atari_two_champion_route" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1)" || {
        printf '%s\n' "$hud_output" >&2
        exit 1
    }
    case "$hud_output" in
        *assetMd5=ebf6a57af3f27782e358c0490bfd2f2e*phase=inactive*levelLoaded=1*'party=11,9,1'*'champions=2'*dm1InventoryPanel=0*) ;;
        *)
            echo "FAIL: authentic Atari champion HUD capture did not preserve the two-champion runtime"
            printf '%s\n' "$hud_output" >&2
            exit 1
            ;;
    esac
    printf '%s\n' "$hud_output" | grep -Eq 'csbViewportHash=[1-9][0-9]*' || {
        echo "FAIL: authentic Atari champion HUD capture lost its source viewport receipt" >&2
        printf '%s\n' "$hud_output" >&2
        exit 1
    }
    screenshot_check_status=0
    python3 - "$hud_screenshot_dir" <<'PY' || screenshot_check_status=$?
import glob
import os
import struct
import sys

screenshots = glob.glob(os.path.join(sys.argv[1], "*.bmp"))
if len(screenshots) != 1:
    raise SystemExit("FAIL: Atari two-champion HUD screenshot was not captured exactly once")
with open(screenshots[0], "rb") as image:
    header = image.read(54)
    if len(header) != 54 or header[:2] != b"BM":
        raise SystemExit("FAIL: Atari two-champion HUD screenshot is not a BMP")
    pixel_offset = struct.unpack_from("<I", header, 10)[0]
    width, signed_height = struct.unpack_from("<ii", header, 18)
    bits_per_pixel = struct.unpack_from("<H", header, 28)[0]
    compression = struct.unpack_from("<I", header, 30)[0]
    height = abs(signed_height)
    if (width, height, bits_per_pixel, compression) != (320, 200, 24, 0):
        raise SystemExit("FAIL: Atari two-champion HUD screenshot has an unexpected pixel layout")
    stride = (width * 3 + 3) & ~3
    image.seek(pixel_offset)
    rows = [image.read(stride) for _ in range(height)]
    if any(len(row) != stride for row in rows):
        raise SystemExit("FAIL: Atari two-champion HUD screenshot is truncated")

    def pixel(x, y):
        storage_y = height - 1 - y if signed_height > 0 else y
        offset = x * 3
        b, g, r = rows[storage_y][offset:offset + 3]
        return (r, g, b)

    # ReDMCSB CHAMDRAW.C F0287 and DATA.C G0055/G0056 put four-pixel
    # champion bars at x=46/53/60 and x=115/122/129 respectively, y=2..26.
    # The original-media CLI route recruits two champions with full stats.
    first = [pixel(x, 2) for x in (46, 53, 60)]
    second = [pixel(x, 2) for x in (115, 122, 129)]
    background = pixel(50, 2)
    if any(color == background for color in first + second):
        raise SystemExit("FAIL: a live Atari champion bar is missing from the HUD screenshot")
    if len(set(first)) != 1 or len(set(second)) != 1 or first[0] == second[0]:
        raise SystemExit("FAIL: Atari champion bar masks or source champion colors are wrong")
    for x in (46, 53, 60, 115, 122, 129):
        if pixel(x, 26) != pixel(x, 2):
            raise SystemExit("FAIL: a full Atari champion bar does not span source rows 2 through 26")

def colors_in(x0, y0, width, height):
    return {pixel(x, y)
            for y in range(y0, y0 + height)
            for x in range(x0, x0 + width)}

# CHAMDRAW.C F0292 uses fixed S20E status boxes at x=champion*69, y=0,
# width=67, height=29. Name text starts at x+1, y=5. Two champions were
# recruited by the original-media route above; require their separate boxes
# to contain rendered source content, not just a viewport receipt or bars.
for slot in (0, 1):
    x = slot * 69
    if len(colors_in(x, 0, 43, 29)) < 3:
        raise SystemExit("FAIL: an Atari champion status box/name is missing from the HUD")
for slot in (0, 1):
    x = slot * 69 + 1
    strip = [pixel(px, py)
             for py in range(5, 12)
             for px in range(x, x + 42)]
    if sum(1 for color in strip if color != strip[0]) < 8:
        raise SystemExit(
            f"FAIL: Atari F0292 champion {slot} name is not visible in its status box")

# F0291 draws the original C033/C034 slot frames at C211..C218: ready/action
# hands start at x+4/x+24, y=10 and are 16x16. Empty slots still have their
# source border, so at least one frame in each live champion row must vary.
for slot in (0, 1):
    x = slot * 69
    ready_colors = len(colors_in(x + 4, 10, 16, 16))
    action_colors = len(colors_in(x + 24, 10, 16, 16))
    if max(ready_colors, action_colors) < 2:
        raise SystemExit(
            f"FAIL: Atari F0291 hand-slot frame is missing from live champion {slot} "
            f"(ready colors={ready_colors}, action colors={action_colors})")
print("PASS: original-media Atari HUD draws champion names, hand slots, and all six live stat bars")
PY
    if [ "$screenshot_check_status" -ne 0 ]; then
        printf '%s\n' "$champion_output" >&2
        exit 1
    fi

    # Continue a fresh original campaign through the retail button door and
    # stairs using ordinary CLI input. The source map starts the party at
    # (9,0), places the first two C127 mirrors on the route, and has a button
    # door at (18,6) before the map-1 stair at (19,7). Let the real map-1
    # groups act after the transition so debug proves movement and damage
    # from authentic C04/C37 records rather than a controlled test pose.
    campaign_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --debug --width 640 --height 400 --game csb --platform atari-st \
        --data-dir "$archive_path" --boot-probe --boot-probe-frames 2 \
        --script "$atari_two_champion_route,wait:32,strafe-left,wait:32,down,wait:32,down,wait:32,strafe-left,wait:32,strafe-left,wait:32,up,wait:32,up,wait:32,up,wait:32,strafe-right,wait:32,up,wait:32,up,wait:32,strafe-left,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,up,wait:32,click:432:162,wait:64,up,wait:32,up,wait:32,strafe-right,wait:500" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1)" || {
        printf '%s\n' "$campaign_output" >&2
        exit 1
    }
    printf '%s\n' "$campaign_output" | grep -Fq \
        'input=pointer-click point=216,81 result=1 last-ui-action="CSB" last-ui-outcome="DOOR SWITCH"' || {
        echo "FAIL: authentic Atari campaign did not activate the source button door" >&2
        printf '%s\n' "$campaign_output" >&2
        exit 1
    }
    printf '%s\n' "$campaign_output" | grep -Fq \
        'level=1 party=18,4,0 dir=2 champions=2 leader=0 active-monsters=12' || {
        echo "FAIL: authentic Atari stair transition did not load map-1 monster groups" >&2
        printf '%s\n' "$campaign_output" >&2
        exit 1
    }
    printf '%s\n' "$campaign_output" | grep -Fq \
        'type=monster-moved' || {
        echo "FAIL: authentic Atari map-1 groups did not move during the real-time wait" >&2
        printf '%s\n' "$campaign_output" >&2
        exit 1
    }
    printf '%s\n' "$campaign_output" | grep -Fq \
        'target=champion index=' || {
        echo "FAIL: authentic Atari map-1 combat did not report champion health changes" >&2
        printf '%s\n' "$campaign_output" >&2
        exit 1
    }
else
    echo "SKIP: authentic Atari ST 7z archive unavailable for champion CLI regression"
fi

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

# Exercise the ordinary CSB default route against the complete installed
# library as well. Platform-forced Atari launches above deliberately point at
# the exact original 7z. Automatic selection now prefers the authenticated FM
# Towns CD; its source title remains active until its original input handoff.
# The separate FM Towns CLI test covers title acceptance and dungeon startup.
if [ -n "$archive_data_root" ] && [ -e "$archive_path" ]; then
    default_output="$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$archive_data_root" --boot-probe \
        --boot-probe-frames 2 --duration 0 2>&1)" || {
        printf '%s\n' "$default_output" >&2
        exit 1
    }
    case "$default_output" in
        *'CSB READY: gameId=csb'*variant=csb-fmtowns-en*route=startup*'FIRESTAFF BOOT PROBE READY: gameId=csb'*sourceId=csb*phase=csb-fmtowns-title*titleReady=1*levelLoaded=0*) ;;
        *)
            echo "FAIL: default CSB library route did not reach an authenticated game startup"
            printf '%s\n' "$default_output" >&2
            exit 1
            ;;
    esac
fi

# With the nonexistent PC edition removed, Atari ST is the third card on the
# platform picker's first row. Select
# CSB, that card, and Original solely through pointer events on the explicit
# launcher canvas, then require the ordinary native launch exit.
# Allow enough software-rendered frames on loaded hosts; the launch exit still
# occurs immediately after FIRESTAFF_EXIT_AFTER_LAUNCH observes the handoff.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --width 1920 --height 1080 --menu --game csb --platform atari-st \
    --data-dir "$media_path" \
    --script 'wait20,click:1173:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 15000 >/dev/null 2>&1

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
# dungeon entrance. Its animation clock is source-owned at 50 Hz; this full
# menu-loop case stays separate from the accelerated boot probes above.
case "$firestaff_cli" in
    */*) app_dir=${firestaff_cli%/*} ;;
    *) app_dir=. ;;
esac
menu_probe="$app_dir/csb-atari-menu-runtime-$$.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --width 960 --height 600 --menu --game csb --platform atari-st \
    --data-dir "$media_path" \
    --script 'enter,enter,enter,wait:3600,click:813:156' \
    --duration 120000 >/dev/null 2>&1
python3 - "$menu_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "csb" or
        startup["receiptReady"] != 1 or startup["phase"] != "inactive" or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 9, 0, 2, 0)):
    raise SystemExit(f"FAIL: authentic CSB Atari start menu did not accept source C200 and reach runtime: {probe}")
print("PASS: authentic CSB Atari start menu accepted source C200 and reached runtime")
PY
echo "PASS: native CSB Atari ST campaign title, input matrix, Original/Modern CLI, and menu-to-runtime C200 route"
