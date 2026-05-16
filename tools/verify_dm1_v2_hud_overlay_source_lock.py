#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_hud_overlay_source_lock.json'

REQUIRED_SOURCE = [
    (SOURCE / 'TIMELINE.C', 'F0260_TIMELINE_RefreshAllChampionStatusBoxes', 1817),
    (SOURCE / 'PANEL.C', 'F0354_INVENTORY_DrawStatusBoxPortrait', 2195),
    (SOURCE / 'GAMELOOP.C', 'F0128_DUNGEONVIEW_Draw_CPSF', 90),
    (SOURCE / 'DUNGEON.C', 'G0233_ai_Graphic559_DirectionToStepEastCount', 35),
    (SOURCE / 'DUNGEON.C', 'G0234_ai_Graphic559_DirectionToStepNorthCount', 40),
]

REQUIRED_FIRESTAFF = [
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'presentation-only'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'TIMELINE.C:F0260'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'PANEL.C:F0354'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'direction is a 0..3 logical value'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'north compass needle'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'clamp high to east'),
]

errors = []
anchors = []
for path, needle, line in REQUIRED_SOURCE:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if needle not in text:
        errors.append(f'missing {needle} in {path.name}')
    if not (1 <= line <= len(lines)):
        errors.append(f'line out of range {path.name}:{line}')
    else:
        anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': lines[line - 1].strip()})

for path, needle in REQUIRED_FIRESTAFF:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff HUD source-lock text {needle} in {path.name}')

result = {
    'status': 'failed' if errors else 'passed',
    'scope': 'dm1_v2_hud_overlay_pc34 presentation-only source-lock',
    'evidenceImpact': {
        'completionMatrixGap': 'Phase 4 HUD/champion panels/interaction/touch-zone route equivalence gates were absent; this adds a first HUD overlay CTest plus source-reference gate, but does not prove the full Phase 4 lane complete.',
        'verifiedCompletionPercent': None,
    },
    'anchors': anchors,
    'errors': errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
if errors:
    for e in errors:
        print('error:', e)
    raise SystemExit(1)
print(f'dm1_v2_hud_overlay_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
