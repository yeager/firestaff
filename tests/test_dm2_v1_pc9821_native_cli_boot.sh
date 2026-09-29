#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_pc9821_native_cli_boot.sh <firestaff>}
archive=${FIRESTAFF_DM2_PC9821_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_PC-9821_JA.zip"}

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

file_sha256() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | awk '{print $1}'
    else
        shasum -a 256 "$1" | awk '{print $1}'
    fi
}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 PC-9821 archive is not staged'
    exit 77
fi

archive_hash_before=$(file_sha256 "$archive")

# Exercise the selected-media route through the start menu. This proves that
# M12 hands the selected authentic PC-9821 ZIP to the game launcher.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm2 --platform pc98 --data-dir "$archive" \
    --script 'key:enter,key:enter,key:enter' --duration 1000 >/dev/null 2>&1

# Exercise the scaled start-menu selection route at a HiDPI-sized output.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm2 --platform pc98 \
    --data-dir "$archive" \
    --script 'wait20,click:1645:262,wait20,click:934:679,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# Exercise the ordinary CLI title route through the PC-9821 New Game rectangle,
# STARTEND's first champion mirror, and the source-owned first runtime party.
# The title rectangle is read from the authentic GDAT in the companion media
# test; its center is (115,65), and the original viewport confirmation is at
# (100,100). No party or map state is synthesized by this test.
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
probe_dir=$(mktemp -d "$scratch_root/dm2-pc9821.XXXXXX")
cleanup_probe_dir() {
    if [ -d "$probe_dir" ]; then
        find "$probe_dir" -depth -delete
    fi
}
trap cleanup_probe_dir EXIT HUP INT TERM
runtime_probe="$probe_dir/runtime.json"
runtime_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --platform pc98 \
    --data-dir "$archive" --boot-probe --boot-probe-frames 0 \
    --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
    --script 'click:115:65,click:100:100' --duration 0 2>&1) || {
    printf '%s\n' "$runtime_output" >&2
    exit 1
}
case "$runtime_output" in
    *'assetMd5=a80c555a858ef7770e1d7f3d2e37fec3'*'phase=dm2-runtime'*'levelLoaded=1'*'party=1,8,0'*'dm2RealAssets=1'*'dm2NoCoreFallbacks=1'*'dm2FallbackDraws=0'*) ;;
    *) printf '%s\n' "$runtime_output" >&2; exit 1 ;;
esac
python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["sourceId"] != "dm2" or
        startup["phase"] != "dm2-runtime" or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 8, 0, 1)):
    raise SystemExit(f"FAIL: authentic PC-9821 New Game route did not reach the first runtime party: {probe}")
PY
echo 'PASS: authentic PC-9821 CLI New Game reached the first real runtime party'

probe_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 320 --height 200 --game dm2 --platform pc98 \
    --data-dir "$archive" --boot-probe --boot-probe-frames 0 \
    --boot-probe-expect-phase dm2-startup-menu \
    --boot-probe-expect-startup-active 1 --boot-probe-expect-title-ready 1 \
    --duration 0 2>&1) || {
    printf '%s\n' "$probe_output" >&2
    exit 1
}
printf '%s\n' "$probe_output" | grep -q \
    'dm2SceneReady=1'
printf '%s\n' "$probe_output" | grep -Fq "assetMd5=a80c555a858ef7770e1d7f3d2e37fec3"
printf '%s\n' "$probe_output" | grep -Fq \
    'Dungeon-Master-II-Skullkeep_PC-9821_JA.zip'

if [ "$archive_hash_before" != "$(file_sha256 "$archive")" ]; then
    echo 'FAIL: DM2 PC-9821 archive changed during native launch' >&2
    exit 1
fi
echo 'PASS: native DM2 PC-9821 ZIP start menu and original title assets'
