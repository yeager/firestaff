#!/usr/bin/env sh
set -eu

app=${1:?usage: test_m12_dm1_csb_shared_collection_real_media.sh <firestaff> <data-root>}
source_root=${2:?usage: test_m12_dm1_csb_shared_collection_real_media.sh <firestaff> <data-root>}

dm1_towns=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$source_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
dm1_pc=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$source_root/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
csb_towns=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$source_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_amiga=${FIRESTAFF_CSB_AMIGA_ARCHIVE:-"$source_root/csb/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip"}
csb_atari=${FIRESTAFF_CSB_ATARI_ARCHIVE:-"$source_root/csb/chaos_strikes_back_ftl.zip"}

if [ ! -x "$app" ] || [ ! -f "$dm1_towns" ] || [ ! -f "$dm1_pc" ] ||
   [ ! -f "$csb_towns" ] || [ ! -f "$csb_amiga" ] ||
   [ ! -f "$csb_atari" ]; then
    echo 'SKIP: authentic DM1 and CSB multi-platform archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-shared-dm1-csb.XXXXXX")
trap 'find "$scratch" -depth -delete' EXIT HUP INT TERM
data_root="$scratch/data"
mkdir -p "$data_root/dm1" "$data_root/csb" "$scratch/home"
ln -s "$dm1_towns" "$data_root/dm1/$(basename "$dm1_towns")"
ln -s "$dm1_pc" "$data_root/dm1/$(basename "$dm1_pc")"
ln -s "$csb_towns" "$data_root/csb/$(basename "$csb_towns")"
ln -s "$csb_amiga" "$data_root/csb/$(basename "$csb_amiga")"
ln -s "$csb_atari" "$data_root/csb/$(basename "$csb_atari")"

probe="$scratch/runtime.json"
log="$scratch/firestaff.log"
FIRESTAFF_CONFIG_PATH="$scratch/home/config.toml" \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
HOME="$scratch/home" XDG_CONFIG_HOME="$scratch/home" APPDATA="$scratch/home" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --width 320 --height 200 --menu --game dm1 \
    --data-dir "$data_root" --debug \
    --script 'enter,enter,enter,wait:12,key:escape,key:enter,wait:60,key:escape,key:escape,key:down,key:enter,key:enter,key:enter,wait:700,click:52:110,wait:10,click:250:50,wait:1000,back,wait:10,enter,wait:60' \
    --duration 60000 >"$log" 2>&1 || {
        cat "$log" >&2
        exit 1
    }

python3 - "$log" "$probe" "$data_root" <<'PY'
import json
from pathlib import Path
import re
import sys

log_path, probe_path, data_root = sys.argv[1:]
trace = Path(log_path).read_text(encoding="utf-8")
probe = json.loads(Path(probe_path).read_text(encoding="utf-8"))

dm1_towns = f"{data_root}/dm1/Dungeon-Master_FM-Towns_JA-EN.zip::DATA/GRAPHICS.DAT"
csb_towns = f"{data_root}/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm1_launch = (
    "launch phase=game-handoff mode=menu game=dm1 platform=FM Towns "
    f"edition=fmtowns-en source={dm1_towns}"
)
csb_launch = (
    "launch phase=game-handoff mode=menu game=csb platform=FM Towns "
    f"edition=fmtowns-en source={csb_towns}::CDATA/GRAPHICS.DAT"
)
if "dm1 platform=FM Towns edition=fmtowns-en matched" not in trace or \
        "dm1 platform=PC edition=pc34-en matched" not in trace:
    raise SystemExit("FAIL: shared collection did not authenticate both DM1 editions")
if "variant=csb-fmtowns-en" not in trace:
    raise SystemExit("FAIL: CSB handoff did not retain its authenticated FM Towns edition")

dm1_runtime = re.search(
    r"startup-frame game=dm1 elapsed-ms=0 source=dm1 phase=dm1-runtime "
    r"active=0 .*level-loaded=1 map=0 party=1,3 dir=2 champions=0", trace)
csb_runtime = re.search(
    r"startup-frame game=dm1 .*source=csb phase=inactive active=0 .*"
    r"level-loaded=1 map=4 party=22,18 dir=2 champions=1", trace)
rescans = list(re.finditer(
    r"return-menu rescan-complete data=" + re.escape(data_root), trace))
if (dm1_launch not in trace or csb_launch not in trace or
        dm1_runtime is None or csb_runtime is None or len(rescans) != 2 or
        not (trace.index(dm1_launch) < dm1_runtime.start() < rescans[0].start() <
             trace.index(csb_launch) < csb_runtime.start() < rescans[1].start())):
    raise SystemExit(
        "FAIL: M12 did not launch DM1, rescan, launch CSB, return and rescan again in order")
for game in ("dm1", "csb"):
    matches = re.findall(rf"return-menu rescan game={game} available=1", trace)
    if len(matches) != 2:
        raise SystemExit(
            f"FAIL: both return-to-menu scans must rediscover {game} in the shared collection")

if (probe.get("launchedEver") != 1 or probe.get("active") != 0 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: shared session did not finish back at the launcher: {probe}")

print("PASS: same M12 session launched DM1, rescanned, launched CSB, returned, and rescanned again")
print("PASS: both game handoffs reached authentic first runtime states from one shared collection")
PY
