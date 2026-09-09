#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}

# The retail DOS ZIP is consumed by the native bounded-memory reader; do not
# let a developer's diagnostic extractor opt-in affect this runtime proof.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 DOS archive is not staged'
    exit 77
fi

# The MVE introduction is a separate owner, but once its bounded source frame
# sequence completes the native launcher must present SKULL's real menu rather
# than a black surface or a prematurely loaded dungeon.  Keep a real 320x200
# presentation capture here: boot receipts alone cannot catch the reported
# "intro ended with no menu" regression.
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
menu_capture=$(mktemp -d "$scratch_root/dm2-dos-menu.XXXXXX")
cleanup_menu_capture() {
    find "$menu_capture" -depth -delete
}
trap cleanup_menu_capture EXIT HUP INT TERM
menu_output=$(FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$menu_capture" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm2 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 5000 --duration 0 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
case "$menu_output" in
    *'phase=dm2-startup-menu'*'levelLoaded=0'*) ;;
    *) printf '%s\n' "$menu_output" >&2; exit 1 ;;
esac
python3 - "$menu_capture" <<'PY'
from pathlib import Path
import struct
import sys

frames = list(Path(sys.argv[1]).glob("*.bmp"))
if len(frames) != 1:
    raise SystemExit("FAIL: expected one DM2 DOS startup-menu presentation capture")
blob = frames[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: DM2 DOS startup-menu capture is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if (width, height, bits) != (320, 200, 24) or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid DM2 DOS startup-menu capture geometry")

# Measure a source-owned visible surface, rather than storing copyrighted
# pixels.  The title/menu has a dense multi-colour body; a missing post-MVE
# menu historically left this frame black despite a superficially valid boot.
pixels = []
for y in range(height):
    row = offset + y * stride
    pixels.extend(tuple(blob[row + x * 3:row + x * 3 + 3]) for x in range(width))
nonblack = sum(pixel != (0, 0, 0) for pixel in pixels)
colours = len(set(pixels))
if nonblack < 10000 or colours < 32:
    raise SystemExit(
        f"FAIL: DM2 DOS startup menu is not visibly presented "
        f"(nonblack={nonblack}, colours={colours})")
print(f"PASS: DM2 DOS startup menu visible nonblack={nonblack} colours={colours}")
PY

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# DM2's second platform card is PC.  This is the public mouse-only card path
# to Original, using the authenticated DOS archive directly in memory.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform pc --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:934:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

probe_input() {
    input=$1
    expected_party=$2
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm2 --platform pc --data-dir "$archive" --boot-probe \
        --boot-probe-frames 5000 --script "key:enter,key:enter,key:enter,$input" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }
    case "$output" in
        *'assetMd5=25247ede4dabb6a71e5dabdfbcd5907d'*'phase=dm2-runtime'*'levelLoaded=1'*"party=$expected_party"*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
}

# These positions/directions are observed from the retail DOS new-game route.
# Each invocation begins a fresh source-owned session, so one input cannot
# mask another through state carried from a previous command.
for case_item in up:1,7,0 down:1,9,2 left:1,8,3 right:1,8,1 \
                 strafe-left:0,8,3 strafe-right:2,8,1 action:1,8,0; do
    probe_input "${case_item%%:*}" "${case_item#*:}"
done
echo 'PASS: native DM2 DOS ZIP start menu -> MVE -> SKULL -> New Game accepts the complete observed input matrix in memory'
