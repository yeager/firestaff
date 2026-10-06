#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_fmtowns_native_cli_boot.sh <firestaff>}
source_rgb=${2:?usage: test_dm2_v1_fmtowns_native_cli_boot.sh <firestaff> <source-rgb-helper>}
archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

# ZIP-contained Towns media is admitted in process, never through 7z/bsdtar.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -x "$source_rgb" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 FM Towns archive is not staged'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform fm-towns --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# The unqualified menu route must resolve the authenticated FM Towns edition
# before applying the same platform-card and New Game inputs.
auto_menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --data-dir "$archive" --verbose \
    --script 'key:enter,key:enter,key:enter' --duration 1000 2>&1) || {
    printf '%s\n' "$auto_menu_output" >&2
    exit 1
}
printf '%s\n' "$auto_menu_output" | grep -q \
    'selected game=dm2 platform=FM Towns edition=fmtowns-ja'

# FM Towns is the first DM2 platform card.  This asserts that the launcher
# admits the authentic disc solely through mouse selection before the source
# title and New Game input path below takes over.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform fm-towns --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# The direct CLI route must also finish the authentic Towns TWANIM before
# its source-coordinate New Game and mirror clicks. Keep the probe surface at
# the original 320x200 coordinates and require the first real map-0 party.
probe_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 \
    --data-dir "$archive" --boot-probe --boot-probe-frames 0 \
    --script 'wait:8000,click:115:65,click:100:60' \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-map 0 --boot-probe-expect-party 1,8,0 \
    --boot-probe-expect-champions 1 --duration 0 2>&1) || {
    printf '%s\n' "$probe_output" >&2
    exit 1
}
printf '%s\n' "$probe_output" | grep -q \
    'dm2FrameAccepted=1 dm2RealAssets=1 dm2NoCoreFallbacks=1 dm2FallbackDraws=0'

# Exercise the reported bare --game dm2 route through AUTO selection, the
# authentic 225-frame TWANIM startup, New Game, and the first champion. The
# launcher paths are covered above. The Towns source clock is 16.632 ms per
# Timer-A tick; wait:8000 needs a longer wall timeout than the bounded
# fast-forward probe above.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
title_probe="$app_dir/test-dm2-fmtowns-bare-title.json"
title_capture="$app_dir/test-dm2-fmtowns-bare-menu-capture"
title_config="$app_dir/test-dm2-fmtowns-bare-menu-isolated.toml"
source_digest=$("$source_rgb" "$archive")
rm -f "$title_probe" "$title_config"
mkdir -p "$title_capture"
rm -f "$title_capture"/*.bmp
FIRESTAFF_CONFIG_PATH="$title_config" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$title_probe" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$title_capture" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --data-dir "$archive" \
    --duration 40000 >/dev/null 2>&1
python3 - "$title_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
towns = probe["dm2FmtownsStartup"]
if (probe["sourceId"] != "dm2" or
        towns["titleFinished"] != 1 or towns["titleRejected"] != 0 or
        towns["titleBound"] != 0 or towns["frameIndex"] != 225 or
        probe["startup"]["phase"] != "dm2-startup-menu"):
    raise SystemExit(f"FAIL: bare DM2 did not reach the original FM Towns menu by 40 seconds: {probe}")
print("PASS: bare DM2 reaches the FM Towns New Game menu within 40 seconds")
PY
python3 - "$title_capture" "$source_digest" <<'PY'
from pathlib import Path
import struct
import sys

frames = list(Path(sys.argv[1]).glob("*.bmp"))
if len(frames) != 1:
    raise SystemExit("FAIL: expected one presented DM2 FM Towns menu frame")
blob = frames[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: presented DM2 FM Towns menu frame is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if (width, height, bits) != (320, 200, 24) or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid DM2 FM Towns menu frame geometry")

# The phase receipt alone cannot prove that SKULL's source-owned GDAT image
# reached the screen. Compare the actual post-present RGB pixels with the
# TITLE/0/4 image and palette independently decoded from this original disc.
digest = 0xcbf29ce484222325
nonblack = 0
for y in range(height):
    row = offset + y * stride
    for x in range(width):
        b, g, r = blob[row + x * 3:row + x * 3 + 3]
        nonblack += (r, g, b) != (0, 0, 0)
        for channel in (r, g, b):
            digest = ((digest ^ channel) * 0x100000001b3) & 0xffffffffffffffff
if nonblack < 10000 or f"{digest:016x}" != sys.argv[2]:
    raise SystemExit(
        "FAIL: presented DM2 FM Towns menu differs from source TITLE/0/4 "
        f"(nonblack={nonblack}, expected={sys.argv[2]}, actual={digest:016x})")
print(f"PASS: original DM2 FM Towns menu RGB matches TITLE/0/4 digest={digest:016x}")
PY
runtime_probe="$app_dir/test-dm2-fmtowns-normal-loop.json"
rm -f "$runtime_probe"
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --data-dir "$archive" \
    --script 'wait:8000,click:115:65,click:100:60' \
    --duration 210000 >/dev/null 2>&1
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
towns = probe["dm2FmtownsStartup"]
script = probe["script"]
runtime_frame = probe["dm2RuntimeFrame"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["phase"] != "dm2-runtime" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        towns["titleFinished"] != 1 or towns["titleRejected"] != 0 or
        script != {"waitFramesRemaining": 0, "pending": 0} or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1) or
        runtime_frame != {"accepted": 1, "realAssets": 1,
                          "noCoreFallbacks": 1, "fallbackDraws": 0}):
    raise SystemExit(f"FAIL: DM2 FM Towns M12 normal loop did not reach a real runtime frame: {probe}")
print("PASS: bare DM2 CLI AUTO, authentic FM Towns TWANIM, New Game and first champion reach the normal-loop runtime")
PY

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 FM Towns archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 FM Towns ZIP start menu and platform-card routes'
