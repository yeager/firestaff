#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_mac_native_cli_boot.sh <firestaff>}
data_root=${FIRESTAFF_DM2_DATA_ROOT:-"$HOME/.firestaff/data"}

# The CUE/BIN archive is a production-native reader path, never an external
# extractor wrapper.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ]; then
    echo 'SKIP: authentic DM2 Macintosh retail archive is not staged'
    exit 77
fi

if [ "${FIRESTAFF_DM2_MAC_ARCHIVE+x}" = x ]; then
    archive=$FIRESTAFF_DM2_MAC_ARCHIVE
else
    archive=''
    canonical="$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN.zip"
    duplicate="$data_root/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"
    try_candidate() {
        candidate=$1
        [ -f "$candidate" ] || return 1
        if output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
            --game dm2 --platform mac --data-dir "$candidate" --boot-probe \
            --boot-probe-frames 1 --duration 0 2>&1); then
            case "$output" in
                *'sourceId=dm2'*'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*)
                    archive=$candidate
                    return 0
                    ;;
            esac
        fi
        return 1
    }
    if ! try_candidate "$canonical" && ! try_candidate "$duplicate"; then
        if [ ! -f "$canonical" ] && [ ! -f "$duplicate" ]; then
            echo 'SKIP: authentic DM2 Macintosh retail archive is not staged'
            exit 77
        fi
        echo 'FAIL: no staged DM2 Macintosh archive passed the retail hash gate' >&2
        exit 1
    fi
fi

if [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Macintosh retail archive is not staged'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

if [ "$(uname -s)" = Darwin ]; then
    # A normal direct launch on macOS must choose authenticated Macintosh
    # retail media automatically even when DOS data is also installed.
    auto_probe_output=$(FIRESTAFF_DATA="$data_root" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm2 --boot-probe \
        --boot-probe-frames 1 --duration 0 2>&1) || {
        printf '%s\n' 'FAIL: DM2 macOS AUTO boot probe failed' "$auto_probe_output" >&2
        exit 1
    }
    case "$auto_probe_output" in
        *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*) ;;
        *)
            printf '%s\n' 'FAIL: DM2 macOS AUTO did not select authenticated Macintosh retail assets' "$auto_probe_output" >&2
            exit 1
            ;;
    esac
fi

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac --data-dir "$archive" \
    --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60' --duration 1000 >/dev/null 2>&1

# The normal install layout keeps originals under dm2/ below the shared data
# root. Verify that menu launch discovers the authenticated Mac retail archive
# from that root even when the user's file manager added a duplicate suffix.
FIRESTAFF_DATA="$data_root" \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60' --duration 1000 >/dev/null 2>&1

# Macintosh is the first card on the second platform row.  This remains a
# launcher-only pointer sequence; the native movie and mirror clicks are
# separately covered below at their original 320x200 coordinate space.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform mac --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:410:679,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# Exercise the ordinary game loop rather than the deterministic boot probe:
# Title.MooV must keep requesting presents until its authentic QuickTime
# duration expires, then accept New Game and enter the selected mirror.
runtime_probe="${app}.mac-normal-start-$$.json"
runtime_capture="${app}.mac-normal-start-capture-$$"
mkdir -p "$runtime_capture"
cleanup_runtime_probe() {
    rm -f "$runtime_probe"
    if [ -d "$runtime_capture" ]; then
        find "$runtime_capture" -depth -delete
    fi
}
trap cleanup_runtime_probe EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
normal_start_output=$(FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$archive" \
    --width 320 --height 200 \
    --script 'wait:700,key:enter,click:100:60' --duration 24000 2>&1) || {
    printf '%s\n' "$normal_start_output" >&2
    exit 1
}
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

startup = probe.get("startup", {})
movie = probe.get("dm2Startup", {})
party = probe.get("party", {})
if (
    probe.get("sourceId") != "dm2"
    or movie.get("platform") != 4
    or movie.get("movieActive") != 0
    or movie.get("movieComplete") != 1
    or movie.get("movieRejected") != 0
    or startup.get("phase") != "dm2-runtime"
    or startup.get("levelLoaded") != 1
    or party.get("championCount") != 2
):
    raise SystemExit("FAIL: normal Macintosh Title.MooV-to-runtime route did not complete")
PY
echo 'PASS: normal DM2 Macintosh Title.MooV loop reaches authentic runtime'

# Join the scaled M12 pointer path to the ordinary Macintosh startup loop.
# Scale mode 0 leaves the 320x200 source surface at 1x, centered in the
# 1920x1080 host window, so source New Game at (100,60) maps to (900,500).
# The script must outlive M12, the retail Title.MooV, New Game and the first
# mirror; a launch receipt alone cannot prove that the selected retail Mac
# package reaches a presented playable frame.
menu_runtime_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --scale-mode 0 --width 1920 --height 1080 \
    --menu --game dm2 --platform mac --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:410:679,wait20,click:450:405,wait20,wait:700,key:enter,click:900:500' \
    --duration 36000 2>&1) || {
    printf '%s\n' "$menu_runtime_output" >&2
    exit 1
}
python3 - "$runtime_probe" "$runtime_capture" <<'PY'
import json
from pathlib import Path
import struct
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

startup = probe.get("startup", {})
movie = probe.get("dm2Startup", {})
party = probe.get("party", {})
runtime_frame = probe.get("dm2RuntimeFrame", {})
script = probe.get("script", {})
if (
    probe.get("sourceId") != "dm2"
    or movie.get("platform") != 4
    or movie.get("movieActive") != 0
    or movie.get("movieComplete") != 1
    or movie.get("movieRejected") != 0
    or startup.get("phase") != "dm2-runtime"
    or startup.get("startupActive") != 0
    or startup.get("levelLoaded") != 1
    or party.get("mapIndex") != 0
    or party.get("mapX") != 1
    or party.get("mapY") != 8
    or party.get("direction") != 0
    or party.get("championCount") != 2
    or runtime_frame != {"accepted": 1, "realAssets": 1,
                         "noCoreFallbacks": 1, "fallbackDraws": 0}
    or script.get("waitFramesRemaining") != 0
    or script.get("pending") != 0
):
    raise SystemExit(
        "FAIL: scaled M12 Macintosh route did not finish Title.MooV, New Game, "
        f"and mirror startup: {probe}")

frames = list(Path(sys.argv[2]).glob("*.bmp"))
if len(frames) != 1:
    raise SystemExit("FAIL: expected one presented DM2 Macintosh runtime frame")
blob = frames[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: DM2 Macintosh runtime screenshot is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if ((width, height, bits) != (320, 200, 24) or
        offset + stride * height != len(blob)):
    raise SystemExit("FAIL: invalid DM2 Macintosh runtime screenshot geometry")
pixels = [blob[offset + y * stride + x * 3:offset + y * stride + x * 3 + 3]
          for y in range(height) for x in range(width)]
if sum(pixel != b"\0\0\0" for pixel in pixels) < 10000 or len(set(pixels)) < 8:
    raise SystemExit("FAIL: DM2 Macintosh runtime frame was not visibly presented")
print("PASS: scaled M12 Macintosh launch reaches and presents authentic runtime")
PY

# Retail Mac owns a title movie before its source New Game action.  The first
# Enter dismisses that movie; the second is the authenticated title-menu
# action.  Keep the host window at 320x200: --script pointer coordinates are
# mapped through the current presentation rectangle, so this preserves the
# original GDAT/mirror coordinates.  The viewport click selects the
# File_header-backed mirror through the native preselection owner, and Up is
# then normal runtime movement.
probe_input() {
    input=$1
    expected_party=$2
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm2 --platform mac --data-dir "$archive" --boot-probe \
        --boot-probe-frames 2000 --width 320 --height 200 \
        --script "key:enter,key:enter,click:100:60,$input" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }
    case "$output" in
        *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*'phase=dm2-runtime'*'levelLoaded=1'*"party=$expected_party"*'champions=2'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
}

default_root_output=$(FIRESTAFF_DATA="$data_root" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --boot-probe --boot-probe-frames 2000 \
    --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$default_root_output" >&2; exit 1; }
case "$default_root_output" in
    *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*) ;;
    *) printf '%s\n' "$default_root_output" >&2; exit 1 ;;
esac

for case_item in up:1,7,0 down:1,9,2 left:1,8,3 right:1,8,1 \
                 strafe-left:0,8,3 strafe-right:2,8,1 action:1,8,0; do
    probe_input "${case_item%%:*}" "${case_item#*:}"
done
if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 Macintosh retail archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 Macintosh ZIP start menu, title, mirror selection, and complete observed input matrix run in memory'
