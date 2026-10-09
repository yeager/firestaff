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

# Use the complete DM2 data directory for every unqualified launch. This
# keeps AUTO selection under test when competing editions are installed and
# still exercises the bare CLI route when only FM Towns media is available.
archive_dir=$(cd "$(dirname "$archive")" && pwd)
default_data_root=$(cd "$archive_dir/.." && pwd)
default_probe_output=$(FIRESTAFF_DATA="$default_data_root" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --debug --boot-probe --boot-probe-frames 1 --duration 0 2>&1) || {
    printf '%s\n' "$default_probe_output" >&2
    exit 1
}
printf '%s\n' "$default_probe_output" | grep -q \
    'game=dm2 platform=FM Towns edition=fmtowns-ja' || {
    printf '%s\n' 'FAIL: bare --game dm2 did not select FM Towns from the complete DM2 data directory' \
        "$default_probe_output" >&2
    exit 1
}
echo 'PASS: bare --game dm2 selects FM Towns from the complete DM2 data directory'

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

# Exercise AUTO selection from the documented per-user data directory. This
# has no FIRESTAFF_DATA or --data-dir override, so the selected source must be
# discovered under ~/.firestaff/data/dm2 itself.
test_scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$test_scratch"
default_home=$(mktemp -d "$test_scratch/dm2-default-data.XXXXXX")
default_data_root="$default_home/.firestaff/data"
default_archive="$default_data_root/dm2/$(basename "$archive")"
default_log="$default_home/menu.log"
mkdir -p "$(dirname "$default_archive")"
ln -s "$archive" "$default_archive"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    HOME="$default_home" XDG_CONFIG_HOME="$default_home/.config" \
    APPDATA="$default_home" FIRESTAFF_CONFIG_PATH="$default_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm2 --verbose --script \
        'key:enter,key:enter,key:enter' --duration 1000
) >"$default_log" 2>&1 || {
    cat "$default_log" >&2
    exit 1
}
if ! grep -Fq \
    "selected game=dm2 platform=FM Towns edition=fmtowns-ja source=$default_archive" \
    "$default_log"; then
    printf '%s\n' 'FAIL: DM2 AUTO menu did not discover FM Towns media from ~/.firestaff/data/dm2 without --data-dir' >&2
    cat "$default_log" >&2
    exit 1
fi
echo 'PASS: DM2 AUTO start menu discovers FM Towns media from ~/.firestaff/data/dm2 without --data-dir'
find "$default_home" -depth -delete

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
runtime_probe="$app_dir/test-dm2-fmtowns-normal-loop.json"
runtime_capture="$app_dir/test-dm2-fmtowns-normal-loop-capture"
runtime_log="$app_dir/test-dm2-fmtowns-menu-normal-loop.log"
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
rm -f "$runtime_probe"
mkdir -p "$runtime_capture"
rm -f "$runtime_capture"/*.bmp
runtime_data_root=$default_data_root
bare_cli_runtime_probe="$app_dir/test-dm2-fmtowns-bare-cli-runtime.json"
bare_cli_runtime_log="$app_dir/test-dm2-fmtowns-bare-cli-runtime.log"
rm -f "$bare_cli_runtime_probe" "$bare_cli_runtime_log"
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$bare_cli_runtime_probe" \
FIRESTAFF_DATA="$runtime_data_root" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --data-dir "$runtime_data_root" \
    --debug --script 'wait:2400,click:115:65,click:100:60' \
    --duration 70000 >"$bare_cli_runtime_log" 2>&1 || {
        cat "$bare_cli_runtime_log" >&2
        exit 1
    }
python3 - "$bare_cli_runtime_probe" "$bare_cli_runtime_log" <<'PY'
import json
from pathlib import Path
import sys

probe_path, log_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
towns = probe["dm2FmtownsStartup"]
trace = Path(log_path).read_text(encoding="utf-8")
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["phase"] != "dm2-runtime" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        towns["titleFinished"] != 1 or towns["titleRejected"] != 0 or
        towns["frameIndex"] != 225 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1) or
        probe["script"] != {"waitFramesRemaining": 0, "pending": 0} or
        probe["dm2RuntimeFrame"] != {"accepted": 1, "realAssets": 1,
                                     "noCoreFallbacks": 1,
                                     "fallbackDraws": 0} or
        "game=dm2 platform=FM Towns edition=fmtowns-ja" not in trace):
    raise SystemExit(
        "FAIL: bare DM2 CLI did not follow Towns title -> New Game -> first "
        f"champion through the normal loop: {probe}")
print("PASS: bare --game dm2 reaches the authentic FM Towns first party "
      "without M12 or boot-probe fast-forward")
PY
rm -f "$bare_cli_runtime_probe" "$bare_cli_runtime_log"
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture" \
FIRESTAFF_DATA="$runtime_data_root" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --menu --game dm2 \
    --data-dir "$runtime_data_root" --verbose \
    --script 'key:enter,key:enter,key:enter,wait:1800,click:115:65,click:100:60' \
    --duration 120000 >"$runtime_log" 2>&1 || {
        cat "$runtime_log" >&2
        exit 1
    }
python3 - "$runtime_probe" "$runtime_capture" "$runtime_log" "$archive" <<'PY'
import json
from pathlib import Path
import struct
import sys

probe_path, capture_dir, log_path, archive_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
towns = probe["dm2FmtownsStartup"]
script = probe["script"]
runtime_frame = probe["dm2RuntimeFrame"]
captures = list(Path(capture_dir).glob("*.bmp"))
trace = Path(log_path).read_text(encoding="utf-8")
selected_edition = (
    "selected game=dm2 platform=FM Towns edition=fmtowns-ja source=" +
    archive_path
)
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["phase"] != "dm2-runtime" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        towns["titleFinished"] != 1 or towns["titleRejected"] != 0 or
        script != {"waitFramesRemaining": 0, "pending": 0} or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1) or
        runtime_frame != {"accepted": 1, "realAssets": 1,
                          "noCoreFallbacks": 1, "fallbackDraws": 0} or
        len(captures) != 1 or selected_edition not in trace):
    raise SystemExit(f"FAIL: DM2 FM Towns M12 normal loop did not reach a real runtime frame: {probe}")

blob = captures[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: DM2 FM Towns runtime screenshot is not a BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
bits = struct.unpack_from("<H", blob, 28)[0]
height = abs(signed_height)
stride = ((width * bits + 31) // 32) * 4
if ((width, height, bits) != (320, 200, 24) or
        offset + stride * height > len(blob)):
    raise SystemExit("FAIL: DM2 FM Towns runtime screenshot has invalid geometry")
nonblack = 0
right_panel = 0
dungeon_scene = 0
for y in range(height):
    row = offset + y * stride
    for x in range(width):
        if any(blob[row + x * 3:row + x * 3 + 3]):
            nonblack += 1
            if x >= 224 and 40 <= y < 176:
                right_panel += 1
            if x < 224 and 40 <= y < 176:
                dungeon_scene += 1
if nonblack < 1000 or right_panel < 2000 or dungeon_scene < 20000:
    raise SystemExit(
        "FAIL: DM2 FM Towns runtime frame is incomplete "
        f"(total={nonblack}, right-panel={right_panel}, "
        f"dungeon={dungeon_scene} lit pixels)")
print("PASS: M12-selected FM Towns edition, authentic TWANIM, New Game and first champion reach the normal-loop runtime")
print(f"PASS: runtime screenshot captured at 320x200 with {nonblack} nonblack pixels, {right_panel} right-panel pixels and {dungeon_scene} dungeon pixels")
PY

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 FM Towns archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 FM Towns ZIP start-menu, platform-card, CLI and M12 runtime routes'
