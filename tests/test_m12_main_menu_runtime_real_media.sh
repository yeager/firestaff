#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_runtime_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_runtime_real_media.sh <firestaff> <data-root>}

dm1_archive="$data_root/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"
csb_archive="$data_root/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"
dm2_archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
theron_track02="$data_root/theron/TQUS02.bin"
if [[ ! -x "$app" || ! -f "$dm1_archive" || ! -f "$csb_archive" || ! -f "$dm2_archive" ]]; then
    echo 'SKIP: authentic DM1, CSB and DM2 FM Towns archives are required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-main-runtime.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

run_main_menu_runtime_case() {
    local game_id=$1
    local archive=$2
    local edition=$3
    local navigation=$4
    local duration_ms=$5
    local platform=${6:-FM Towns}
    local home="$scratch/$game_id-home"
    local log="$scratch/$game_id.log"
    local probe="$scratch/$game_id-runtime.json"
    local archive_hash

    mkdir -p "$home"
    archive_hash=$(firestaff_sha256_file "$archive")
    env -u FIRESTAFF_EXIT_AFTER_LAUNCH \
        FIRESTAFF_CONFIG_PATH="$home/config.toml" \
        FIRESTAFF_DATA="$data_root" \
        FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
        FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
        HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 --data-dir "$data_root" \
            --script "$navigation" --duration "$duration_ms" >"$log" 2>&1 || {
                cat "$log" >&2
                return 1
            }

    if ! grep -Fq 'startup game=none mode=menu' "$log" ||
       ! grep -Fq "launch phase=game-handoff mode=menu game=$game_id platform=$platform edition=$edition source=" "$log"; then
        printf 'FAIL: no-game M12 menu did not select and start authentic %s %s media\n' \
            "$game_id" "$edition" >&2
        cat "$log" >&2
        return 1
    fi

    python3 - "$game_id" "$probe" <<'PY'
import json
import sys

game_id, probe_path = sys.argv[1:]
with open(probe_path, encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != game_id or startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or startup.get("levelLoaded") != 1 or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: {game_id} menu selection did not reach active runtime: {probe}")

if game_id == "dm1":
    if (startup.get("phase") != "dm1-runtime" or
            startup.get("dm1StartupHoCFirstFrameReady") != 1):
        raise SystemExit(f"FAIL: DM1 menu start missed its HoC runtime frame: {probe}")
elif game_id == "csb":
    if (startup.get("phase") != "inactive" or not probe.get("csbViewportHash") or
            (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
             party.get("direction"), party.get("championCount")) != (4, 22, 18, 2, 1)):
        raise SystemExit(f"FAIL: CSB menu start missed its original MINI.DAT party: {probe}")
elif game_id == "dm2":
    title = probe.get("dm2FmtownsStartup", {})
    frame = probe.get("dm2RuntimeFrame", {})
    if (startup.get("phase") != "dm2-runtime" or
            title.get("titleFinished") != 1 or title.get("frameIndex") != 225 or
            frame != {"accepted": 1, "realAssets": 1,
                      "noCoreFallbacks": 1, "fallbackDraws": 0} or
            (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
             party.get("direction"), party.get("championCount")) != (0, 1, 8, 0, 1)):
        raise SystemExit(f"FAIL: DM2 menu start missed its authentic first party: {probe}")
elif game_id == "theron":
    if (startup.get("phase") != "theron-runtime" or
            probe.get("bootAssetMd5") != "f23601102138f87c33025877767ebf76" or
            (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
             party.get("direction"), party.get("championCount")) != (0, 1, 0, 0, 2)):
        raise SystemExit(f"FAIL: Theron menu start missed its authenticated Track 02 runtime: {probe}")
else:
    raise SystemExit(f"FAIL: unsupported game in real-media menu test: {game_id}")

print(f"PASS: no-game M12 menu {game_id} reached its authentic first runtime")
PY

    if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
        printf 'FAIL: original %s archive changed during menu startup\n' "$game_id" >&2
        return 1
    fi
}

run_main_menu_runtime_case dm1 "$dm1_archive" fmtowns-en \
    'wait-scan-complete,wait20,key:enter,wait20,key:enter,wait20,key:enter' 60000
run_main_menu_runtime_case csb "$csb_archive" fmtowns-en \
    'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:52:110,wait10,click:250:50,wait240' 150000
run_main_menu_runtime_case dm2 "$dm2_archive" fmtowns-ja \
    'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait:2400,click:115:65,click:100:60' 90000

if [[ -f "$theron_track02" ]]; then
    run_main_menu_runtime_case theron "$theron_track02" pce-en \
        'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait30,enter,enter,down,down,down,down,down,down,enter,down,enter,tab' \
        90000 'PC Engine'
else
    echo 'SKIP: authentic Theron US Track 02 is unavailable'
fi
