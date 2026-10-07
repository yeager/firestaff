#!/usr/bin/env sh
set -eu

app=${1:?usage: test_csb_v1_fmtowns_m12_auto_mixed_real_media.sh <firestaff> <csb-data-root>}
collection_root=${2:?usage: test_csb_v1_fmtowns_m12_auto_mixed_real_media.sh <firestaff> <csb-data-root>}
towns_archive="$collection_root/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
amiga_archive="$collection_root/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"
atari_archive="$collection_root/chaos_strikes_back_ftl.zip"

if [ ! -x "$app" ] || [ ! -f "$towns_archive" ] ||
   [ ! -f "$amiga_archive" ] || [ ! -f "$atari_archive" ]; then
    echo 'SKIP: authentic CSB FM Towns, Amiga, and Atari ST archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/csb-fmtowns-auto-mixed.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM

# Run the normal M12 path against a mixed collection. Deliberately omit
# --platform so the production scanner and AUTO edition ranking choose the
# documented FM Towns default while authentic Amiga and Atari ST editions are
# also installed. The scripted title, SWITCHTW Game row, and Prison Enter
# clicks then reach the first real MINI.DAT party without a synthetic save.
FIRESTAFF_CONFIG_PATH="$scratch/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$scratch/runtime.json" \
HOME="$scratch" XDG_CONFIG_HOME="$scratch" APPDATA="$scratch" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game csb \
    --data-dir "$collection_root" --debug \
    --script 'enter,enter,enter,wait700,click:52:110,wait10,click:250:50,wait240' \
    --duration 30000 >"$scratch/firestaff.log" 2>&1 || {
        cat "$scratch/firestaff.log" >&2
        exit 1
    }

python3 - "$scratch/firestaff.log" "$scratch/runtime.json" \
    "$towns_archive" <<'PY'
import json
import re
import sys

log_path, probe_path, towns_archive = sys.argv[1:]
with open(log_path, encoding="utf-8") as stream:
    trace = stream.read()
with open(probe_path, encoding="utf-8") as stream:
    probe = json.load(stream)

for platform, edition in (("Amiga", "amiga31-en"),
                           ("Atari ST", "st20-21-en")):
    if re.search(rf"csb platform={re.escape(platform)} edition={edition} "
                 r"matched source=", trace) is None:
        raise SystemExit(
            f"FAIL: mixed CSB collection did not authenticate {platform} {edition}")

ready = re.search(
    r"CSB READY: gameId=csb dataDir=" + re.escape(towns_archive) +
    r" variant=csb-fmtowns-en route=startup handoff=f31-title-anm "
    r"handoffHash=[0-9a-f]{8}", trace)
launch = re.search(
    r"launch phase=game-handoff mode=menu game=csb platform=FM Towns "
    r"edition=fmtowns-en source=" + re.escape(towns_archive), trace)
if ready is None or launch is None:
    raise SystemExit(
        "FAIL: M12 AUTO did not select and launch the authentic FM Towns edition")

startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("sourceId") != "csb" or
        startup.get("phase") != "inactive" or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("csbViewportHash", 0) == 0 or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1)):
    raise SystemExit(
        f"FAIL: CSB M12 AUTO did not reach the authentic MINI.DAT runtime: {probe}")

print("PASS: mixed authentic CSB M12 AUTO selected FM Towns and reached "
      "the original MINI.DAT party")
PY
