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
        Path('/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'),
    ])
    for candidate in candidates:
        if (candidate / 'DUNGEON.C').exists() and (candidate / 'GAMELOOP.C').exists():
            return candidate
    raise SystemExit('error: ReDMCSB source root not found; set FIRESTAFF_REDMCSB_SOURCE')


SOURCE = redmcsb_source_root()
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_camera_controller_source_lock.json'
REQUIRED = [
    (SOURCE / 'DUNGEON.C', 'G0233_ai_Graphic559_DirectionToStepEastCount', 35),
    (SOURCE / 'DUNGEON.C', 'G0234_ai_Graphic559_DirectionToStepNorthCount', 40),
    (SOURCE / 'DUNGEON.C', 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement', 1371),
    (SOURCE / 'DUNGEON.C', 'G0233_ai_Graphic559_DirectionToStepEastCount', 1389),
    (SOURCE / 'GAMELOOP.C', 'F0128_DUNGEONVIEW_Draw_CPSF', 90),
    (SOURCE / 'GAMELOOP.C', 'F0380_COMMAND_ProcessQueue_CPSC', 215),
]
FIRESTAFF = [
    (ROOT / 'dm1_v2_camera_controller_pc34.c', 'presentation-only'),
    (ROOT / 'dm1_v2_camera_controller_pc34.c', 'DUNGEON.C:1371-1391'),
    (ROOT / 'dm1_v2_camera_controller_pc34.c', 'GAMELOOP.C:90'),
    (ROOT / 'test_dm1_v2_camera_controller_pc34.c', 'dm1_v2_camera_begin_move'),
]
errors=[]; anchors=[]
for path, needle, line in REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if needle not in text:
        errors.append(f'missing {needle} in {path.name}')
    if not (1 <= line <= len(lines)):
        errors.append(f'line out of range {path.name}:{line}')
    else:
        anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': lines[line-1].strip()})
for path, needle in FIRESTAFF:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff camera source-lock text {needle} in {path.name}')
result={'status':'failed' if errors else 'passed','anchors':anchors,'errors':errors}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True)+'\n', encoding='utf-8')
if errors:
    for e in errors: print('error:', e)
    raise SystemExit(1)
print(f'dm1_v2_camera_controller_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
