#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def redmcsb_source_root() -> Path:
    candidates = []
    if os.environ.get('FIRESTAFF_REDMCSB_SOURCE'):
        candidates.append(Path(os.environ['FIRESTAFF_REDMCSB_SOURCE']).expanduser())
    candidates.extend([
        Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source',
        Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser(),
    ])
    for candidate in candidates:
        if (candidate / 'DEFS.H').exists() and (candidate / 'COMMAND.C').exists():
            return candidate
    raise SystemExit('error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE')


SOURCE = redmcsb_source_root()
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_touch_controller_affordance_source_lock.json'

REQUIRED = [
    (SOURCE / 'DEFS.H', 'typedef struct {', 197),
    (SOURCE / 'DEFS.H', 'KEYBOARD_INPUT', 200),
    (SOURCE / 'DEFS.H', 'MOUSE_INPUT', 211),
    (SOURCE / 'DEFS.H', 'C001_COMMAND_TURN_LEFT', 238),
    (SOURCE / 'DEFS.H', 'C006_COMMAND_MOVE_LEFT', 243),
    (SOURCE / 'COMMAND.C', 'void F0380_COMMAND_ProcessQueue_CPSC', 2045),
    (SOURCE / 'COMMAND.C', 'F0365_COMMAND_ProcessTypes1To2_TurnParty', 2151),
    (SOURCE / 'COMMAND.C', 'F0366_COMMAND_ProcessTypes3To6_MoveParty', 2155),
    (SOURCE / 'CLIKMENU.C', 'void F0365_COMMAND_ProcessTypes1To2_TurnParty', 142),
    (SOURCE / 'CLIKMENU.C', 'F0284_CHAMPION_SetPartyDirection', 172),
    (SOURCE / 'CLIKMENU.C', 'void F0366_COMMAND_ProcessTypes3To6_MoveParty', 180),
    (SOURCE / 'CLIKMENU.C', 'G0465_ai_Graphic561_MovementArrowToStepForwardCount', 224),
    (SOURCE / 'CLIKMENU.C', 'G0466_ai_Graphic561_MovementArrowToStepRightCount', 229),
    (SOURCE / 'CLIKMENU.C', 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 269),
    (SOURCE / 'GAMELOOP.C', 'G0321_B_StopWaitingForPlayerInput', 164),
    (SOURCE / 'GAMELOOP.C', 'F0380_COMMAND_ProcessQueue_CPSC', 215),
    (SOURCE / 'GAMELOOP.C', 'G0301_B_GameTimeTicking', 219),
]

FIRESTAFF_REQUIRED = [
    (ROOT / 'include/dm1_v2_touch_controller_affordance_pc34.h', 'DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP'),
    (ROOT / 'include/dm1_v2_touch_controller_affordance_pc34.h', 'DM1_V2_AFFORDANCE_TOUCH_PINCH_ZOOM_IN'),
    (ROOT / 'include/dm1_v2_touch_controller_affordance_pc34.h', 'DM1_V2_AFFORDANCE_TOUCH_PINCH_ZOOM_OUT'),
    (ROOT / 'include/dm1_v2_touch_controller_affordance_pc34.h', 'DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT'),
    (ROOT / 'include/dm1_v2_touch_controller_affordance_pc34.h', 'DM1_V2_TouchControllerAffordanceRoute'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'DEFS.H:197-211'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'DEFS.H:238-243'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'COMMAND.C:2045-2155'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'CLIKMENU.C:142-174'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'CLIKMENU.C:180-390'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'GAMELOOP.C:164-219'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'DM1_V2_AFFORDANCE_TOUCH_PINCH_ZOOM_IN/OUT'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'SDL_FINGER'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'v2_minimap_zoom'),
    (ROOT / 'src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c', 'dm1_v2_movement_command_route_for_presentation'),
    (ROOT / 'src/dm1v2/dm1_v2_minimap_pc34.c', 'void v2_minimap_zoom'),
    (ROOT / 'src/engine/main_loop_m11.c', 'SDL_EVENT_FINGER_DOWN'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'V1 touch/click parity guard'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'Pinch-to-zoom V2 UI guard'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'v2_minimap_zoom'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'Runtime bridge gate'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'dm1_v2_movement_command_apply'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'runtime.lastCommand == route.route.runtimeCommand'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'route.route.sourceCommand == 0'),
    (ROOT / 'tests/test_dm1_v2_touch_controller_affordance_pc34.c', 'runtime.lastCommand == 0'),
    (ROOT / 'CMakeLists.txt', 'test_dm1_v2_touch_controller_affordance_pc34'),
    (ROOT / 'CMakeLists.txt', 'dm1_v2_touch_controller_affordance_source_lock'),
]

errors = []
anchors = []
for path, needle, line in REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if needle not in text:
        errors.append(f'missing source needle {needle} in {path.name}')
    if line < 1 or line > len(lines):
        errors.append(f'line out of range {path.name}:{line}')
    else:
        line_text = lines[line - 1].strip()
        if needle not in line_text:
            errors.append(f'line anchor mismatch {path.name}:{line}: expected {needle!r}, got {line_text!r}')
        anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': line_text})

for path, needle in FIRESTAFF_REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff affordance source-lock anchor {needle} in {path.relative_to(ROOT)}')

result = {'status': 'failed' if errors else 'passed', 'anchors': anchors, 'errors': errors}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')

if errors:
    for error in errors:
        print('error:', error)
    raise SystemExit(1)
print(f'dm1_v2_touch_controller_affordance_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
