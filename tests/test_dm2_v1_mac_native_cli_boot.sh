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

# Keep the end-to-end M12 sessions representative while avoiding repeated
# scans of unrelated DM1/CSB/DM2 archives in the user's shared data root. The
# bytes remain the same authenticated retail ZIP; the temporary install uses
# only a symlink under the normal .firestaff/data/<game> layout.
menu_data_root=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-dm2-mac-menu.XXXXXX")
mkdir -p "$menu_data_root/dm2"
archive_dir=$(cd "$(dirname "$archive")" && pwd)
archive_name=${archive##*/}
ln -s "$archive_dir/$archive_name" "$menu_data_root/dm2/$archive_name"
auto_data_archive="$menu_data_root/dm2/$archive_name"
dos_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"
if [ -f "$dos_archive" ]; then
    ln -s "$dos_archive" "$menu_data_root/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"
fi
cleanup_menu_data() {
    if [ -d "$menu_data_root" ]; then
        rm -rf "$menu_data_root"
    fi
}
trap cleanup_menu_data EXIT

archive_hash_before=$(sha256sum "$archive")

if [ "$(uname -s)" = Darwin ]; then
    # An explicit Macintosh direct launch must retain its authenticated
    # source even when FM Towns and DOS media are also installed.
    auto_probe_output=$(FIRESTAFF_DATA="$data_root" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm2 --platform mac --boot-probe \
        --boot-probe-frames 1 --duration 0 2>&1) || {
        printf '%s\n' 'FAIL: explicit DM2 Macintosh boot probe failed' "$auto_probe_output" >&2
        exit 1
    }
    case "$auto_probe_output" in
        *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*) ;;
        *)
            printf '%s\n' 'FAIL: explicit DM2 Macintosh launch did not select retail assets' "$auto_probe_output" >&2
            exit 1
            ;;
    esac
fi

FIRESTAFF_DATA="$menu_data_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform mac \
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
FIRESTAFF_DATA="$menu_data_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform mac \
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
    cleanup_menu_data
}
trap cleanup_runtime_probe EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
normal_start_output=$(FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$archive" \
    --width 320 --height 200 \
    --script 'wait:1200,key:enter,wait:30,click:115:65,wait:30,click:112:130,wait:30,key:right,wait:30,key:s,wait:30,key:up' --duration 45000 2>&1) || {
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
output_size = probe.get("outputSize", {})
if (
    probe.get("sourceId") != "dm2"
    or movie.get("platform") != 4
    or movie.get("movieActive") != 0
    or movie.get("movieComplete") != 1
    or movie.get("movieRejected") != 0
    or startup.get("phase") != "dm2-runtime"
    or startup.get("levelLoaded") != 1
    or party.get("mapIndex") != 0
    or party.get("mapX") != 3
    or party.get("mapY") != 8
    or party.get("direction") != 1
    or party.get("championCount") != 1
    or output_size.get("valid") != 1
    or (output_size.get("windowWidth"), output_size.get("windowHeight"),
        output_size.get("drawableWidth"), output_size.get("drawableHeight"))
       != (320, 200, 320, 200)
):
    raise SystemExit(
        "FAIL: normal Macintosh Title.MooV-to-runtime route did not turn east "
        "and move two tiles from the authentic (1,8) start after SDL keys: "
        f"{probe}")
PY
echo 'PASS: normal DM2 Macintosh Title.MooV loop turns and moves twice in authentic runtime'

# Join the scaled M12 pointer path to the ordinary Macintosh startup loop.
# Scale mode 0 leaves the 320x200 source surface at 1x, centered in the
# 1920x1080 host window, so source New Game at (100,60) maps to (900,500).
# The script must outlive M12, the 17.5-second retail Title.MooV, New Game and the first
# mirror; then click the retail Mac forward arrow through the scaled pointer
# route, step north again through the authentic open corridor, and issue real
# eastward turn/move commands after the menu handoff. A launch
# receipt or a changed mirror position alone cannot prove that gameplay input
# reaches the selected retail Mac runtime.
menu_runtime_output=$(FIRESTAFF_DATA="$menu_data_root" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --scale-mode 0 --width 1920 --height 1080 \
    --menu --game dm2 --platform mac \
    --script 'wait20,click:1645:262,wait20,click:410:679,wait20,click:450:405,wait20,wait:1200,key:enter,click:900:500,wait:20,click:1074:580,wait:30,key:up,wait:30,key:right,wait:30,key:up,wait:30,key:up' \
    --duration 44000 2>&1) || {
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
output_size = probe.get("outputSize", {})
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
    or party.get("mapX") != 3
    or party.get("mapY") != 6
    or party.get("direction") != 1
    or party.get("championCount") != 1
    or runtime_frame != {"accepted": 1, "realAssets": 1,
                         "noCoreFallbacks": 1, "fallbackDraws": 0}
    or script.get("waitFramesRemaining") != 0
    or script.get("pending") != 0
    or output_size.get("valid") != 1
    or (output_size.get("windowWidth"), output_size.get("windowHeight"),
        output_size.get("drawableWidth"), output_size.get("drawableHeight"))
       != (1920, 1080, 1920, 1080)
):
    raise SystemExit(
        "FAIL: scaled M12 Macintosh route did not finish Title.MooV, New Game, "
        "mirror startup, and two source runtime moves: "
        f"{probe}")

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
mac_hud_pixels = [pixels[y * width + x]
                  for y in range(28) for x in range(width)]
if sum(pixel != b"\0\0\0" for pixel in mac_hud_pixels) < 100:
    raise SystemExit(
        "FAIL: authentic Mac champion portraits and status rows are missing "
        "from the RAW4-owned top HUD area")
viewport_pixels = [pixels[y * width + x]
                   for y in range(40, 176) for x in range(224)]
if sum(pixel != b"\0\0\0" for pixel in viewport_pixels) < 22000:
    raise SystemExit(
        "FAIL: authentic DM2 Macintosh dungeon viewport is mostly unpainted; "
        "expected the RECT_7 224x136 aperture at (0,40) to carry the runtime scene")
lower_floor_pixels = [pixels[y * width + x]
                      for y in range(136, 175) for x in range(224)]
if sum(pixel != b"\0\0\0" for pixel in lower_floor_pixels) < 5000:
    raise SystemExit(
        "FAIL: authentic DM2 Macintosh floor is missing from the lower "
        "RECT_7 aperture; the previous origin-aligned scene hid this band")
arrow_rects = ((229, 129), (291, 129), (260, 129),
               (291, 153), (260, 153), (229, 153))
for arrow, (x0, y0) in enumerate(arrow_rects):
    arrow_pixels = [pixels[y * width + x]
                    for y in range(y0, y0 + 23)
                    for x in range(x0, x0 + 29)]
    if (sum(pixel != b"\0\0\0" for pixel in arrow_pixels) < 400 or
            len(set(arrow_pixels)) < 8):
        raise SystemExit(
            f"FAIL: Mac retail movement image {arrow} was not presented "
            f"inside its RAW4 destination {(x0, y0, 29, 23)}")
print("PASS: scaled M12 Macintosh launch turns and moves twice in authentic runtime")
PY

# The same original-media route must be chosen by a plain `--game dm2` on a
# macOS host, even when the shared root also contains a complete DOS install.
auto_root_output=$(FIRESTAFF_DATA="$menu_data_root" \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --boot-probe --boot-probe-frames 500 \
    --width 320 --height 200 \
    --script 'key:enter,key:enter,click:100:60,up' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --duration 0 2>&1) || { printf '%s\n' "$auto_root_output" >&2; exit 1; }
case "$auto_root_output" in
    *'assetMd5=5cab25f6b975957eae4a203174e7f2a6'*"dataDir=$auto_data_archive"*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,7,0'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$auto_root_output" >&2; exit 1 ;;
esac
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)

# DM2_PLATFORM_MAC_EN is enum value 4 in include/dm2_v1_boot.h.  The
# graphics hash is shared by multiple releases, so it cannot identify Mac.
actual_platform = probe.get("dm2Startup", {}).get("platform")
if actual_platform != 4:
    raise SystemExit(
        f"FAIL: DM2 AUTO selected platform {actual_platform}, expected "
        "DM2_PLATFORM_MAC_EN (4)")
print("PASS: DM2 AUTO selected authenticated Macintosh platform (4)")
PY

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 Macintosh retail archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 Macintosh ZIP AUTO start menu, title, mirror selection, and post-launch movement run in memory'
