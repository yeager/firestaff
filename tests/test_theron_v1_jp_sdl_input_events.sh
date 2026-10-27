#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_JP_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQJP02.bin"
if [[ ! -f "$track02" ]]; then
    track02="$data_root/Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin"
fi
expected_md5=b7afb338ad31be1025b53f9aff12d73a
scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$PWD/build/test-scratch"}

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron Japan Rev. 1 raw Track 02 BIN is not staged\n'
    exit 77
fi

mkdir -p "$scratch_root"
keyboard_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-keyboard.XXXXXX")
button_ii_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-button-ii.XXXXXX")
side_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-side-steps.XXXXXX")
movement_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-panel-movement.XXXXXX")
action_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-panel-actions.XXXXXX")
touch_forward_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-touch-forward.XXXXXX")
touch_turn_probe=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-touch-turn.XXXXXX")
keyboard_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-keyboard-output.XXXXXX")
button_ii_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-button-ii-output.XXXXXX")
side_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-side-steps-output.XXXXXX")
movement_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-panel-movement-output.XXXXXX")
action_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-panel-actions-output.XXXXXX")
touch_forward_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-touch-forward-output.XXXXXX")
touch_turn_output=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-touch-turn-output.XXXXXX")
touch_config=$(mktemp "$scratch_root/firestaff-theron-jp-sdl-touch-config.XXXXXX")
printf 'touch_controls_index = 1\ninput_mode_index = 0\n' >"$touch_config"
trap 'rm -f "$keyboard_probe" "$button_ii_probe" "$side_probe" "$movement_probe" "$action_probe" "$touch_forward_probe" "$touch_turn_probe" "$keyboard_output" "$button_ii_output" "$side_output" "$movement_output" "$action_output" "$touch_forward_output" "$touch_turn_output" "$touch_config"' EXIT

startup_script='enter,enter,enter,action,tab'
run_event_route() {
    local probe=$1
    local output=$2
    local gameplay_events=$3
    FIRESTAFF_AUTOTEST=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe" \
    FIRESTAFF_CONFIG_PATH="$touch_config" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --width 320 --height 224 \
        --game theron \
        --theron-native jp \
        --data-dir "$data_root" \
        --debug \
        --lang en \
        --duration 5000 \
        --script "$startup_script,wait-game-runtime,$gameplay_events" \
        >"$output" 2>&1
}

# Exercise the regular SDL event loop against the authentic Japanese level 0.
run_event_route "$keyboard_probe" "$keyboard_output" \
    'key:down,wait:32'
run_event_route "$button_ii_probe" "$button_ii_output" \
    'rightdown:40:40,key:right,rightup:40:40,wait:32'
# Start from the source-backed open route, sidestep right, then return left.
run_event_route "$side_probe" "$side_output" \
    'key:down,click:292:163,click:204:163,wait:32'
run_event_route "$movement_probe" "$movement_output" \
    'click:204:144,click:248:144,wait:32'
run_event_route "$action_probe" "$action_output" \
    'click:204:182,click:292:201,wait:32'
run_event_route "$touch_forward_probe" "$touch_forward_output" \
    'touch-swipe-up,wait:32'
run_event_route "$touch_turn_probe" "$touch_turn_output" \
    'touch-swipe-left,wait:32'

python3 - "$keyboard_probe" "$button_ii_probe" "$side_probe" \
    "$movement_probe" "$action_probe" "$touch_forward_probe" \
    "$touch_turn_probe" "$expected_md5" <<'PY'
import json
import sys

keyboard_path, button_ii_path, side_path, movement_path, action_path, \
    touch_forward_path, touch_turn_path, expected_md5 = sys.argv[1:]
probes = {}
for label, path in (("keyboard", keyboard_path), ("button-ii", button_ii_path),
                    ("side-steps", side_path), ("movement", movement_path),
                    ("actions", action_path), ("touch-forward", touch_forward_path),
                    ("touch-turn", touch_turn_path)):
    with open(path, encoding="utf-8") as stream:
        probes[label] = json.load(stream)
    probe = probes[label]
    if probe.get("sourceId") != "theron":
        raise SystemExit(f"FAIL: {label} SDL route selected {probe.get('sourceId')!r}")
    if not probe.get("startup", {}).get("levelLoaded"):
        raise SystemExit(f"FAIL: {label} SDL route did not load a dungeon")
    if probe.get("bootAssetMd5") != expected_md5:
        raise SystemExit(f"FAIL: {label} SDL route did not use authentic Japan Track 02")

expected_poses = {
    "keyboard": (1, 1, 0),
    "button-ii": (1, 0, 1),
    "side-steps": (1, 1, 0),
    "movement": (2, 3, 3),
}
for label, expected in expected_poses.items():
    party = probes[label]["party"]
    actual = (party["mapX"], party["mapY"], party["direction"])
    if actual != expected:
        raise SystemExit(f"FAIL: {label} SDL route pose {actual} != {expected}")

if probes["actions"].get("gameTick", 0) < 1:
    raise SystemExit("FAIL: mouse-panel WAIT did not advance an authentic gameplay tick")
if probes["touch-forward"].get("lastAction") != "MOVE":
    raise SystemExit("FAIL: Japanese Theron swipe-up did not dispatch MOVE")
if (probes["touch-turn"].get("lastAction") != "TURN" or
        probes["touch-turn"]["party"]["direction"] != 3):
    raise SystemExit("FAIL: Japanese Theron swipe-left did not turn west")
PY

grep -Eq 'game-action game=theron tick=[0-9]+ input=backward result=1 action="MOVE" outcome="THERON STEPPED BACK"' "$keyboard_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=turn-right result=1 action="TURN" outcome="RIGHT"' "$button_ii_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=292,151 result=1 action="MOVE" outcome="THERON STEPPED RIGHT"' "$side_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,151 result=1 action="MOVE" outcome="THERON STEPPED LEFT"' "$side_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,132 result=1 action="TURN" outcome="LEFT"' "$movement_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=248,132 result=1 action="MOVE" outcome="THERON ADVANCED"' "$movement_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,170 result=1 action="CHAMPION" outcome="ACTIVE CHAMPION CHANGED"' "$action_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=292,189 result=1 action="WAIT" outcome="THERON TICK"' "$action_output"

printf '%s\n' 'PASS: authentic Theron JP SDL routes keyboard, Button II, mouse-panel movement, touch gestures, champion selection, and wait'
