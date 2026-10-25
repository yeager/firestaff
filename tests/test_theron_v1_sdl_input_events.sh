#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQUS02.bin"
expected_md5=f23601102138f87c33025877767ebf76
scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/build/test-scratch"}

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron USA raw Track 02 BIN is not staged\n'
    exit 77
fi

mkdir -p "$scratch_root"
held_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-held.XXXXXX")
released_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-released.XXXXXX")
held_output=$(mktemp "$scratch_root/firestaff-theron-sdl-held-output.XXXXXX")
released_output=$(mktemp "$scratch_root/firestaff-theron-sdl-released-output.XXXXXX")
trap 'rm -f "$held_probe" "$released_probe" "$held_output" "$released_output"' EXIT

startup_script='enter,enter,down,down,down,down,down,down,enter,down,enter,tab'
run_event_route() {
    local probe=$1
    local output=$2
    local gameplay_events=$3
    FIRESTAFF_AUTOTEST=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --width 320 --height 224 \
        --game theron \
        --theron-native us \
        --data-dir "$data_root" \
        --debug \
        --duration 8000 \
        --script "$startup_script,wait-game-runtime,$gameplay_events" \
        >"$output" 2>&1
}

# The right-button events and key-down events enter the normal SDL poll loop.
# At the source-owned starting tile, the Button II chord turns right without
# moving; after button release, the same key side-steps right to (2,1).
run_event_route "$held_probe" "$held_output" \
    'down,rightdown:40:40,key:right,wait:8'
run_event_route "$released_probe" "$released_output" \
    'down,rightdown:40:40,rightup:40:40,key:right,wait:8'

python3 - "$held_probe" "$released_probe" "$expected_md5" <<'PY'
import json
import sys

held_path, released_path, expected_md5 = sys.argv[1:]
with open(held_path, encoding="utf-8") as stream:
    held = json.load(stream)
with open(released_path, encoding="utf-8") as stream:
    released = json.load(stream)

for label, probe in (("held", held), ("released", released)):
    startup = probe.get("startup", {})
    if not startup.get("levelLoaded"):
        raise SystemExit(f"FAIL: {label} SDL event route did not load a dungeon")
    if probe.get("sourceId") != "theron":
        raise SystemExit(f"FAIL: {label} SDL event route selected {probe.get('sourceId')!r}")

held_party = held["party"]
released_party = released["party"]
if (held_party["mapX"], held_party["mapY"], held_party["direction"]) != (1, 1, 1):
    raise SystemExit(f"FAIL: held Button II + Right did not turn in place: {held_party}")
if (released_party["mapX"], released_party["mapY"], released_party["direction"]) != (2, 1, 0):
    raise SystemExit(f"FAIL: Right after Button II release did not side-step: {released_party}")
if held.get("bootAssetMd5") != expected_md5 or released.get("bootAssetMd5") != expected_md5:
    raise SystemExit("FAIL: runtime event route did not use authenticated USA Track 02")

print("PASS: authentic Theron SDL events route Button II turn and post-release side-step")
PY
