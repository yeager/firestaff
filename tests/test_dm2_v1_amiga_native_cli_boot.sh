#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_amiga_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_AMIGA_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Amiga_EN.zip"}

# ZIP -> native Amiga media admission must not depend on an external tool.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Amiga archive is not staged'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform amiga --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# DM2's third platform card is Amiga; retain the real archive through a
# pointer-only game -> platform -> Original selection.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform amiga --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:1458:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# Amiga startup is owned by the original SWSH.DAT -> TITL.DAT streams.
# Their source tick waits total 1,345 50 Hz VBlanks for this archive.  The
# retained GDAT then owns New Game at source point (115,65).  At the explicit
# 1x scale in a 960x600 window, the 320x200 game surface is centered at
# (320,200), so the SDL click must be sent at window point (435,265).  The
# host loop also has work between source ticks, so reserve 2,000 loop frames
# and a 90-second process limit rather than equating script frames with VBlanks.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_dir="$app_dir/test-dm2-amiga-startup"
runtime_probe="$runtime_dir/runtime.json"
runtime_capture="$runtime_dir/capture"
mkdir -p "$runtime_capture"
rm -f "$runtime_probe" "$runtime_capture"/*.bmp
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 960 --height 600 --scale-mode 0 --menu --game dm2 --platform amiga \
    --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter,wait:2000,click:435:265' \
    --duration 90000 >/dev/null 2>&1
python3 - "$runtime_probe" "$runtime_capture" <<'PY'
import json
from pathlib import Path
import struct
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm2-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1)):
    raise SystemExit(f"FAIL: authentic DM2 Amiga start menu did not reach runtime: {probe}")

frames = list(Path(sys.argv[2]).glob("*.bmp"))
if len(frames) != 1:
    raise SystemExit("FAIL: expected one presented DM2 Amiga runtime frame")
blob = frames[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: DM2 Amiga runtime screenshot is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if ((width, height, bits) != (320, 200, 24) or
        offset + stride * height != len(blob)):
    raise SystemExit("FAIL: invalid DM2 Amiga runtime screenshot geometry")
pixels = [blob[offset + y * stride + x * 3:offset + y * stride + x * 3 + 3]
          for y in range(height) for x in range(width)]
if sum(pixel != b"\0\0\0" for pixel in pixels) < 10000 or len(set(pixels)) < 8:
    raise SystemExit("FAIL: DM2 Amiga runtime frame was not visibly presented")
print("PASS: authentic DM2 Amiga start menu reached and presented a runtime frame")
PY

probe_input() {
    input=$1
    expected_party=$2
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm2 --platform amiga --data-dir "$archive" --boot-probe \
        --boot-probe-frames 2000 --script "key:enter,key:enter,key:enter,$input" \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }
    case "$output" in
        *'assetMd5=1c940ea95703eaea0ecdf84d17e954b9'*'phase=dm2-runtime'*'levelLoaded=1'*"party=$expected_party"*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
}
for case_item in up:1,7,0 down:1,9,2 left:1,8,3 right:1,8,1 \
                 strafe-left:0,8,3 strafe-right:2,8,1 action:1,8,0; do
    probe_input "${case_item%%:*}" "${case_item#*:}"
done
if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 Amiga archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 Amiga ZIP start menu accepts the complete observed input matrix in memory'
