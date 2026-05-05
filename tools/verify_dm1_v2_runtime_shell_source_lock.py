#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path('/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source')
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_runtime_shell_source_lock.json'
REQUIRED = [
    (SOURCE / 'GAMELOOP.C', 'F0380_COMMAND_ProcessQueue_CPSC', 215),
    (SOURCE / 'GAMELOOP.C', 'G0321_B_StopWaitingForPlayerInput', 164),
    (SOURCE / 'GAMELOOP.C', 'G0301_B_GameTimeTicking', 219),
    (SOURCE / 'DUNGEON.C', 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 1371),
    (SOURCE / 'DUNGEON.C', 'G0233_ai_Graphic559_DirectionToStepEastCount', 1389),
]
FIRESTAFF_REQUIRED = [
    (ROOT / 'dm1_v2_runtime_pc34.c', 'GAMELOOP.C:215'),
    (ROOT / 'dm1_v2_runtime_pc34.c', 'DUNGEON.C:1371-1391'),
    (ROOT / 'dm1_v2_runtime_pc34.h', 'DM1_V2_RuntimeState'),
    (ROOT / 'test_dm1_v2_runtime_shell_pc34.c', 'dm1_v2_runtime_apply_command'),
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
        anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': lines[line - 1].strip()})
for path, needle in FIRESTAFF_REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff runtime shell anchor {needle} in {path.name}')
result = {'status': 'failed' if errors else 'passed', 'anchors': anchors, 'errors': errors}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
if errors:
    for error in errors:
        print('error:', error)
    raise SystemExit(1)
print(f'dm1_v2_runtime_shell_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
