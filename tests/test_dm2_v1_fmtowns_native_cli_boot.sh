#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_fmtowns_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}

# ZIP-contained Towns media is admitted in process, never through 7z/bsdtar.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 FM Towns archive is not staged'
    exit 77
fi

archive_hash_before=$(sha256sum "$archive")

FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform fm-towns --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

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
    --width 320 --height 200 --game dm2 --platform fm-towns \
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

# Exercise one uninterrupted normal-loop route through M12, the authentic
# 225-frame TWANIM startup, New Game, and the first champion. The Towns source
# clock is 16.632 ms per Timer-A tick; wait:8000 needs a longer wall timeout
# than the bounded fast-forward probe above. SDL's dummy window is resized to
# the requested logical size so the scripted launcher and game clicks use the
# same window coordinates as their visible rectangles.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_probe="$app_dir/test-dm2-fmtowns-normal-loop.json"
rm -f "$runtime_probe"
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --scale-mode 0 --menu --game dm2 \
    --platform fm-towns --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:410:405,wait20,click:450:405,wait:8000,key:enter,click:900:500' \
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
print("PASS: DM2 FM Towns M12, authentic TWANIM, New Game and first champion reach the normal-loop runtime")
PY

if [ "$archive_hash_before" != "$(sha256sum "$archive")" ]; then
    echo 'FAIL: DM2 FM Towns archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 FM Towns ZIP start menu and platform-card routes'
