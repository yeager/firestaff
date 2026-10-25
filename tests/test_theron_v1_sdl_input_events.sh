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
panel_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-panel.XXXXXX")
item_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-item.XXXXXX")
side_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-side-steps.XXXXXX")
action_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-mouse-actions.XXXXXX")
left_turn_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-left-turn.XXXXXX")
keyboard_action_probe=$(mktemp "$scratch_root/firestaff-theron-sdl-keyboard-actions.XXXXXX")
held_output=$(mktemp "$scratch_root/firestaff-theron-sdl-held-output.XXXXXX")
released_output=$(mktemp "$scratch_root/firestaff-theron-sdl-released-output.XXXXXX")
panel_output=$(mktemp "$scratch_root/firestaff-theron-sdl-panel-output.XXXXXX")
item_output=$(mktemp "$scratch_root/firestaff-theron-sdl-item-output.XXXXXX")
side_output=$(mktemp "$scratch_root/firestaff-theron-sdl-side-steps-output.XXXXXX")
action_output=$(mktemp "$scratch_root/firestaff-theron-sdl-mouse-actions-output.XXXXXX")
left_turn_output=$(mktemp "$scratch_root/firestaff-theron-sdl-left-turn-output.XXXXXX")
keyboard_action_output=$(mktemp "$scratch_root/firestaff-theron-sdl-keyboard-actions-output.XXXXXX")
trap 'rm -f "$held_probe" "$released_probe" "$panel_probe" "$item_probe" "$side_probe" "$action_probe" "$left_turn_probe" "$keyboard_action_probe" "$held_output" "$released_output" "$panel_output" "$item_output" "$side_output" "$action_output" "$left_turn_output" "$keyboard_action_output"' EXIT

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
        --lang en \
        --duration 8000 \
        --script "$startup_script,wait-game-runtime,$gameplay_events" \
        >"$output" 2>&1
}

# The mouse and key events enter the normal SDL poll loop.
# At the source-owned starting tile, the Button II chord turns right without
# moving; after button release, the same key side-steps right to (2,1).
# Two TURN R panel clicks and a FORWARD click cross the next source-backed tile.
run_event_route "$held_probe" "$held_output" \
    'down,rightdown:40:40,key:right,wait:8'
run_event_route "$released_probe" "$released_output" \
    'down,rightdown:40:40,rightup:40:40,key:right,wait:8'
run_event_route "$panel_probe" "$panel_output" \
    'click:292:144,click:292:144,click:248:144,wait:8'
# Walk to the authentic level-0 sandals, then use the visible mouse controls
# to pick them up, reselect the exact source slot, and drop it.
run_event_route "$item_probe" "$item_output" \
    'key:down,key:down,key:q,click:248:182,click:292:182,click:204:201,wait:8'
# The left cell beside the initial spawn is a wall in authentic Track 02.
# Move to an open tile, step right, then return left along the same passage.
run_event_route "$side_probe" "$side_output" \
    'key:down,click:292:163,click:204:163,wait:8'
run_event_route "$action_probe" "$action_output" \
    'click:204:182,click:292:201,wait:8'
run_event_route "$left_turn_probe" "$left_turn_output" \
    'click:204:144,wait:8'
# Tab, Button I's Z binding, Space, and Button II's X binding take the same
# normal SDL keydown path used by a physical keyboard. Verify these
# non-movement actions independently of the host mouse panel on the
# hash-verified original runtime. Button II must be received but must not
# advance the dungeon clock when no source-owned cancel surface is active.
run_event_route "$keyboard_action_probe" "$keyboard_action_output" \
    'key:tab,key:z,key:space,key:x,wait:8'

python3 - "$held_probe" "$released_probe" "$panel_probe" "$item_probe" \
    "$side_probe" "$action_probe" "$left_turn_probe" "$keyboard_action_probe" \
    "$expected_md5" <<'PY'
import json
import sys

held_path, released_path, panel_path, item_path, side_path, action_path, \
    left_turn_path, keyboard_action_path, expected_md5 = sys.argv[1:]
with open(held_path, encoding="utf-8") as stream:
    held = json.load(stream)
with open(released_path, encoding="utf-8") as stream:
    released = json.load(stream)
with open(panel_path, encoding="utf-8") as stream:
    panel = json.load(stream)
with open(item_path, encoding="utf-8") as stream:
    item = json.load(stream)
with open(side_path, encoding="utf-8") as stream:
    side = json.load(stream)
with open(action_path, encoding="utf-8") as stream:
    actions = json.load(stream)
with open(left_turn_path, encoding="utf-8") as stream:
    left_turn = json.load(stream)
with open(keyboard_action_path, encoding="utf-8") as stream:
    keyboard_actions = json.load(stream)

for label, probe in (("held", held), ("released", released), ("panel", panel),
                     ("item", item), ("side", side), ("actions", actions),
                     ("left-turn", left_turn), ("keyboard-actions", keyboard_actions)):
    startup = probe.get("startup", {})
    if not startup.get("levelLoaded"):
        raise SystemExit(f"FAIL: {label} SDL event route did not load a dungeon")
    if probe.get("sourceId") != "theron":
        raise SystemExit(f"FAIL: {label} SDL event route selected {probe.get('sourceId')!r}")

held_party = held["party"]
released_party = released["party"]
panel_party = panel["party"]
item_party = item["party"]
if (held_party["mapX"], held_party["mapY"], held_party["direction"]) != (1, 1, 1):
    raise SystemExit(f"FAIL: held Button II + Right did not turn in place: {held_party}")
if (released_party["mapX"], released_party["mapY"], released_party["direction"]) != (2, 1, 0):
    raise SystemExit(f"FAIL: Right after Button II release did not side-step: {released_party}")
if (panel_party["mapX"], panel_party["mapY"], panel_party["direction"]) != (1, 1, 2):
    raise SystemExit(f"FAIL: live SDL mouse-panel turn-and-forward clicks did not move: {panel_party}")
if (item_party["mapX"], item_party["mapY"], item_party["direction"]) != (1, 2, 3):
    raise SystemExit(f"FAIL: live SDL mouse-panel item actions changed the expected pose: {item_party}")
side_party = side["party"]
if (side_party["mapX"], side_party["mapY"], side_party["direction"]) != (1, 1, 0):
    raise SystemExit(f"FAIL: mouse-panel side steps did not return along the authentic open path: {side_party}")
if (left_turn["party"]["mapX"], left_turn["party"]["mapY"],
        left_turn["party"]["direction"]) != (1, 0, 3):
    raise SystemExit(f"FAIL: mouse-panel left turn did not update the source-backed pose: {left_turn['party']}")
if any(probe.get("bootAssetMd5") != expected_md5 for probe in
       (held, released, panel, item, side, actions, left_turn, keyboard_actions)):
    raise SystemExit("FAIL: runtime event route did not use authenticated USA Track 02")
if keyboard_actions.get("gameTick", 0) < 2:
    raise SystemExit("FAIL: keyboard Z and Space did not advance authentic gameplay ticks")

PY
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=[0-9]+,[0-9]+ result=1 action="ITEM" outcome="ITEM PICKED UP"' "$item_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=[0-9]+,[0-9]+ result=1 action="ITEM" outcome="SANDALS"' "$item_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=[0-9]+,[0-9]+ result=1 action="ITEM" outcome="ITEM DROPPED"' "$item_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=292,151 result=1 action="MOVE" outcome="THERON STEPPED RIGHT"' "$side_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,151 result=1 action="MOVE" outcome="THERON STEPPED LEFT"' "$side_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,132 result=1 action="TURN" outcome="LEFT"' "$left_turn_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=204,170 result=1 action="CHAMPION" outcome="ACTIVE CHAMPION CHANGED"' "$action_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=pointer-left point=292,189 result=1 action="WAIT" outcome="THERON TICK"' "$action_output"
grep -Eq 'game-action game=theron tick=[0-9]+ input=cycle-champion result=1 action="CHAMPION" outcome="ACTIVE CHAMPION CHANGED"' "$keyboard_action_output"
[[ $(grep -Ec 'game-action game=theron tick=[0-9]+ input=action result=1 action="WAIT" outcome="THERON TICK"' "$keyboard_action_output") -ge 2 ]]
grep -Eq 'game-action game=theron tick=[0-9]+ input=back result=0 action="THERON" outcome="UNKNOWN INPUT"' "$keyboard_action_output"
printf '%s\n' 'PASS: authentic Theron SDL routes Button II, keyboard, mouse-panel movement, champion/wait, and source-backed item actions'
