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
trap 'rm -f "$runtime_probe"' EXIT
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
