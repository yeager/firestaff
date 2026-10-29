#!/usr/bin/env bash
set -euo pipefail
. "$(dirname "$0")/lib/portable_sha256.sh"

app=${1:?usage: test_m12_main_menu_dm2_only_real_media.sh <firestaff> <data-root>}
data_root=${2:?usage: test_m12_main_menu_dm2_only_real_media.sh <firestaff> <data-root>}
archive="$data_root/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    echo 'SKIP: authentic DM2 FM Towns archive is required'
    exit 77
fi

scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/m12-dm2-only.XXXXXX")
cleanup() {
    if [[ ${FIRESTAFF_KEEP_TEST_SCRATCH:-0} == 1 ]]; then
        echo "TEST SCRATCH PRESERVED: $scratch"
    else
        rm -rf "$scratch"
    fi
}
trap cleanup EXIT HUP INT TERM

home="$scratch/home"
dm2_root="$home/.firestaff/data/dm2"
mkdir -p "$dm2_root"
ln -s "$archive" "$dm2_root/$(basename "$archive")"
archive_hash=$(firestaff_sha256_file "$archive")
probe="$scratch/runtime.json"
log="$scratch/firestaff.log"

# With only authentic DM2 media in the documented data tree and no --game,
# the startup menu must discover DM2, allow selection, and reach its FM Towns
# runtime without silently falling back to another game's data. Wait for the
# actual FM Towns title-complete receipt before sending the New Game clicks;
# host loop speed must not decide whether those clicks arrive during the title.
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    HOME="$home" XDG_CONFIG_HOME="$home" APPDATA="$home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait-dm2-title,click:115:65,click:100:60' \
        --duration 75000
) >"$log" 2>&1 || {
    cat "$log" >&2
    exit 1
}

expected_source="$dm2_root/$(basename "$archive")"
for required in \
    'startup game=none mode=menu' \
    'scan complete elapsed-ms=' \
    'games=dm1:no,csb:no,dm2:yes,nexus:no,theron:no original-candidate=yes' \
    "launch phase=game-handoff mode=menu game=dm2 platform=FM Towns edition=fmtowns-ja source=$expected_source"; do
    if ! grep -Fq "$required" "$log"; then
        printf 'FAIL: DM2-only no-game menu is missing %s\n' "$required" >&2
        grep -E 'startup game=|scan complete|game-handoff|error|failed' "$log" >&2 || true
        exit 1
    fi
done

python3 - "$probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
title = probe.get("dm2FmtownsStartup", {})
frame = probe.get("dm2RuntimeFrame", {})
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm2" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "dm2-runtime" or
        title.get("titleFinished") != 1 or title.get("frameIndex") != 225 or
        frame != {"accepted": 1, "realAssets": 1,
                  "noCoreFallbacks": 1, "fallbackDraws": 0} or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 1, 8, 0, 1) or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: DM2-only no-game menu missed authentic runtime: {probe}")
print("PASS: DM2-only no-game M12 menu reached the original FM Towns first runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original DM2 archive changed during the M12 menu launch' >&2
    exit 1
fi
echo 'PASS: default .firestaff/data/dm2 discovery launched authentic DM2 without CLI game selection'

# Backing out of the platform picker must return to the game list. Reopening
# DM2 must still bind the real FM Towns edition and complete the normal runtime
# handoff; a stale picker stage or selection must not launch anything early.
back_home="$scratch/back-home"
back_dm2_root="$back_home/.firestaff/data/dm2"
mkdir -p "$back_dm2_root"
ln -s "$archive" "$back_dm2_root/$(basename "$archive")"
back_probe="$scratch/back-runtime.json"
back_log="$scratch/back-firestaff.log"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$back_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$back_probe" \
    HOME="$back_home" XDG_CONFIG_HOME="$back_home" APPDATA="$back_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 320 --height 200 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:down,wait20,key:enter,wait20,key:escape,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait-dm2-title,click:115:65,click:100:60' \
        --duration 85000
) >"$back_log" 2>&1 || {
    cat "$back_log" >&2
    exit 1
}

if [[ $(grep -Fc 'launch phase=game-handoff' "$back_log") != 1 ]] ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=dm2 platform=FM Towns edition=fmtowns-ja source=$back_dm2_root/$(basename "$archive")" "$back_log"; then
    echo 'FAIL: backing out of DM2 platform selection did not return cleanly to the game list and launch FM Towns after reselection' >&2
    grep -E 'startup game=|game-handoff|error|failed' "$back_log" >&2 || true
    exit 1
fi

python3 - "$back_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
title = probe.get("dm2FmtownsStartup", {})
frame = probe.get("dm2RuntimeFrame", {})
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "dm2" or
        startup.get("phase") != "dm2-runtime" or
        startup.get("levelLoaded") != 1 or
        title.get("titleFinished") != 1 or title.get("frameIndex") != 225 or
        frame != {"accepted": 1, "realAssets": 1,
                  "noCoreFallbacks": 1, "fallbackDraws": 0} or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 1, 8, 0, 1)):
    raise SystemExit(f"FAIL: Back/reselect route missed authentic FM Towns runtime: {probe}")
print("PASS: backing out of DM2 platform choice and reselecting reaches authentic FM Towns runtime")
PY

if [[ $(firestaff_sha256_file "$archive") != "$archive_hash" ]]; then
    echo 'FAIL: original DM2 archive changed during Back/reselect menu launch' >&2
    exit 1
fi
