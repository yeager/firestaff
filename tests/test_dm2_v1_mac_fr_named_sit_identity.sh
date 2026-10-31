#!/usr/bin/env sh
set -eu
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_dm2_v1_mac_fr_named_sit_identity.sh <firestaff>}
mac_root=${FIRESTAFF_DM2_MAC_FR_DIR:-"$HOME/.firestaff/data/dm2-extras/mac-fr/Dungeon Master II"}
archive=${FIRESTAFF_DM2_MAC_FR_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_FR.sit"}
dmfiles="$mac_root/DMFiles"
if [ ! -x "$app" ]; then
    echo 'SKIP: Firestaff executable is unavailable'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"${TMPDIR:-/tmp}"}
mkdir -p "$scratch_root"
scratch=""
cleanup() {
    rc=$?
    if [ "$rc" -ne 0 ] && [ -n "$scratch" ] && [ -f "$scratch/startup.log" ]; then
        cat "$scratch/startup.log" >&2
    fi
    if [ -n "$scratch" ] && [ -d "$scratch" ]; then
        find "$scratch" -depth -delete
    fi
}
trap cleanup EXIT HUP INT TERM

if [ ! -f "$dmfiles/Graphics.dat" ] || [ ! -f "$dmfiles/Dungeon.dat" ] ||
   [ ! -f "$dmfiles/md.dat" ] || [ ! -f "$dmfiles/qstrat.dat" ]; then
    if [ -f "$archive" ] && command -v unar >/dev/null 2>&1; then
        scratch=$(mktemp -d "$scratch_root/dm2-mac-fr-startup.XXXXXX")
        if unar -q -o "$scratch" "$archive" >/dev/null 2>&1; then
            extracted_root="$scratch/Dungeon Master II"
            extracted_files="$extracted_root/DMFiles"
            if [ -f "$extracted_files/Graphics.dat" ] &&
               [ -f "$extracted_files/Dungeon.dat" ] &&
               [ -f "$extracted_files/md.dat" ] &&
               [ -f "$extracted_files/qstrat.dat" ]; then
                mac_root=$extracted_root
                dmfiles=$extracted_files
            fi
        fi
    fi
fi

if [ ! -f "$dmfiles/Graphics.dat" ] || [ ! -f "$dmfiles/Dungeon.dat" ] ||
   [ ! -f "$dmfiles/md.dat" ] || [ ! -f "$dmfiles/qstrat.dat" ]; then
    echo 'SKIP: Macintosh files from the French-named SIT or its source archive are not staged'
    exit 77
fi

python3 - "$dmfiles" <<'PY'
import hashlib
import os
import sys

root = sys.argv[1]
expected = {
    "Graphics.dat": "5cab25f6b975957eae4a203174e7f2a6",
    "Dungeon.dat": "719ae78bc124027806c65491a256827d",
    "md.dat": "a1692c182344952b6bffcb82e2e2f06e",
    "qstrat.dat": "535bcbd371347cbcadf61e35d51c1d46",
}
for name, wanted in expected.items():
    path = os.path.join(root, name)
    with open(path, "rb") as stream:
        actual = hashlib.md5(stream.read()).hexdigest()
    if actual != wanted:
        raise SystemExit(
            f"FAIL: {name} is not the known Macintosh English runtime payload"
        )
print("PASS: French-named SIT contains the known Macintosh English four-file tuple")
PY

graphics_before=$(firestaff_sha256_file "$dmfiles/Graphics.dat")
dungeon_before=$(firestaff_sha256_file "$dmfiles/Dungeon.dat")
music_before=$(firestaff_sha256_file "$dmfiles/md.dat")
strings_before=$(firestaff_sha256_file "$dmfiles/qstrat.dat")
unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
if ! FIRESTAFF_CONFIG_PATH="$scratch/config.toml" \
     FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
     FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/runtime.json" \
     SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
     --game dm2 --platform mac --data-dir "$mac_root" \
     --width 320 --height 200 \
     --script 'wait:1200,key:enter,wait:30,click:115:65,wait:30,click:112:130,wait-game-runtime,back' \
     --duration 70000 --debug >"$scratch/startup.log" 2>&1; then
    echo 'FAIL: DM2 Macintosh files from the French-named SIT did not start' >&2
    exit 1
fi
python3 - "$scratch/runtime.json" "$scratch/startup.log" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as stream:
    probe = json.load(stream)
startup = probe.get("startup", {})
movie = probe.get("dm2Startup", {})
party = probe.get("party", {})
script = probe.get("script", {})
if (
    probe.get("sourceId") != "dm2"
    or movie.get("platform") != 4
    or startup.get("phase") != "dm2-runtime"
    or startup.get("startupActive") != 0
    or startup.get("levelLoaded") != 1
    or party.get("championCount") != 1
    or script.get("waitFramesRemaining") != 0
    or script.get("pending") != 0
):
    raise SystemExit(f"FAIL: authentic Mac data did not reach runtime: {probe}")

with open(sys.argv[2], encoding="utf-8", errors="replace") as stream:
    render_lines = [line for line in stream if
                    "dm2-render-debug game=dm2 platform=mac-en " in line]
accepted_frame = any(
    all(marker in line for marker in (
        "accepted=1", "real-assets=1", "no-core-fallbacks=1",
        "blocked-materials=0", "hud={required:1", "complete:1",
    ))
    for line in render_lines
)
if not accepted_frame:
    raise SystemExit(
        "FAIL: no accepted original-asset Macintosh viewport/HUD frame "
        "was logged before returning to the launcher"
    )
print("PASS: French-named SIT reaches Macintosh English runtime with an accepted original-asset viewport/HUD frame; this is not French-edition evidence")
PY

[ "$graphics_before" = "$(firestaff_sha256_file "$dmfiles/Graphics.dat")" ]
[ "$dungeon_before" = "$(firestaff_sha256_file "$dmfiles/Dungeon.dat")" ]
[ "$music_before" = "$(firestaff_sha256_file "$dmfiles/md.dat")" ]
[ "$strings_before" = "$(firestaff_sha256_file "$dmfiles/qstrat.dat")" ]
echo 'PASS: extracted files from the French-named SIT remain byte-identical'
