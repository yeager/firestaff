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
    (SOURCE / 'PANEL.C', 'F0395_MENUS_DrawMovementArrows', 2337),
    (SOURCE / 'CHAMDRAW.C', 'F0292_CHAMPION_DrawState', 703),
    (SOURCE / 'COMMAND.C', 'G0454_as_Graphic561_MouseInput_SpellArea', 191),
    (SOURCE / 'COMMAND.C', 'C108_COMMAND_CLICK_IN_SPELL_AREA_CAST_SPELL', 199),
    (SOURCE / 'GAMELOOP.C', 'F0128_DUNGEONVIEW_Draw_CPSF', 90),
    (SOURCE / 'DUNGEON.C', 'G0233_ai_Graphic559_DirectionToStepEastCount', 35),
    (SOURCE / 'DUNGEON.C', 'G0234_ai_Graphic559_DirectionToStepNorthCount', 40),
]

REQUIRED_FIRESTAFF = [
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'presentation-only'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'TIMELINE.C:F0260'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'PANEL.C:F0354'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'CHAMDRAW.C champion status boxes'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'STATS.C F0090-F0092'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'COMMAND.C:461-482'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'does not mutate dungeon, champion, or command runtime state'),
    (ROOT / 'include/dm1_v2_hud_overlay_pc34.h', 'M11_V2_HudRuneOverlayPc34'),
    (ROOT / 'include/dm1_v2_hud_overlay_pc34.h', 'v2_hud_set_rune_overlay'),
    (ROOT / 'src/dm1v2/dm1_v2_hud_overlay_pc34.c', 'direction is a 0..3 logical value'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'north compass needle'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'clamp high to east'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'active action flash strip'),
    (ROOT / 'tests/test_dm1_v2_hud_overlay_pc34.c', 'cast-ready control'),
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
        'completionMatrixGap': 'Phase 4 HUD/champion/action/rune presentation-state pixels are source-locked and CTest-gated; this does not prove finished art, live V1 transactions, or full Phase 4 parity.',
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
