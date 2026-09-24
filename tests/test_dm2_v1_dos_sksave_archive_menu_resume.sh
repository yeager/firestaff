#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_dos_sksave_archive_menu_resume.sh <firestaff>}
archive=${FIRESTAFF_DM2_DOS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip"}

# Do not let an optional diagnostic extractor mask the production ZIP reader.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 DOS archive is not staged'
    exit 77
fi

# Keep both the game and its original SKSAVE member in the same read-only ZIP.
# Direct CLI and the menu hand --save to DM2's source GAME_LOAD path; no user
# media may be materialized beside the archive.
save_path="$archive::data/sksave1.dat"
probe_resume_direct() {
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" \
    --boot-probe --boot-probe-frames 5000 \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-map 11 --boot-probe-expect-party 15,10,2 \
    --duration 0 2>&1) || { printf '%s\n' "$output" >&2; exit 1; }

    case "$output" in
        *'assetMd5=25247ede4dabb6a71e5dabdfbcd5907d'*'phase=dm2-runtime'*'levelLoaded=1'*'map=11'*'party=15,10,2'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*'startedFromLauncher=1'*) ;;
        *) printf '%s\n' "$output" >&2; exit 1 ;;
    esac
}

probe_resume_direct --game dm2 --platform pc --data-dir "$archive" --save "$save_path"

# --boot-probe is a direct-launch contract and cannot be combined with M12.
# Exercise the real Quick Resume row through one normal menu Enter and inspect
# M11's runtime receipt instead of asserting a boot-probe from an invalid route.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_probe="$app_dir/dm2-sksave-menu-runtime-$$.json"
trap 'rm -f "$runtime_probe"' EXIT HUP INT TERM
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
    --script enter --duration 3000 >/dev/null 2>&1
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm2" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm2-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"], party["direction"])
        != (11, 15, 10, 2)):
    raise SystemExit(f"FAIL: authentic DM2 SKSAVE1 M12 Quick Resume failed: {probe}")
print("PASS: authentic DM2 SKSAVE1 M12 Quick Resume reached its saved runtime pose")
PY

# Modern/V2.2 changes presentation geometry only here.  With real GDAT
# material present it must retain the source-owned runtime frame and report no
# core fallback draw; locally generated V2.2 cache art is deliberately not a
# substitute for this original-media route.
v22_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform pc --data-dir "$archive" --save "$save_path" \
    --presentation-mode v22 --boot-probe --boot-probe-frames 5000 \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --boot-probe-expect-map 11 --boot-probe-expect-party 15,10,2 \
    --duration 0 2>&1) || { printf '%s\n' "$v22_output" >&2; exit 1; }
case "$v22_output" in
    *'presentationMode=2'*'presentation=640x400'*'phase=dm2-runtime'*'dm2FrameAccepted=1'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$v22_output" >&2; exit 1 ;;
esac

echo 'PASS: native DM2 DOS ZIP CLI and start menu resume archive::SKSAVE in memory'
