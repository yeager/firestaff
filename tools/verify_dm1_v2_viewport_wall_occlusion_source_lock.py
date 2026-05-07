#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / 'parity-evidence/verification/dm1_v2_viewport_wall_occlusion_source_lock.json'

REQUIRED = [
    (SOURCE / 'DUNVIEW.C', 'void F0128_DUNGEONVIEW_Draw_CPSF', 8318),
    (SOURCE / 'DUNVIEW.C', 'G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001', 8357),
    (SOURCE / 'DUNVIEW.C', 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, 0', 8498),
    (SOURCE / 'DUNVIEW.C', 'F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);', 8499),
    (SOURCE / 'DUNVIEW.C', 'STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF', 6642),
    (SOURCE / 'DUNVIEW.C', 'case C00_ELEMENT_WALL', 6697),
    (SOURCE / 'DUNVIEW.C', 'F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);', 6714),
    (SOURCE / 'DUNVIEW.C', 'return;', 6720),
    (SOURCE / 'DUNVIEW.C', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0206_ai_SquareAspect[M550_FIRST_THING]', 6816),
]

FIRESTAFF_REQUIRED = [
    (ROOT / 'dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_ELEMENT_WALL 0'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_VIEW_SQUARE_D3C'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_use_flipped_wall_bitmaps'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_square_occludes_beyond'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:8357'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:6697-6720'),
    (ROOT / 'dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:6816'),
    (ROOT / 'test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_wall_occlusion_source_lock'),
    (ROOT / 'test_dm1_v2_movement_viewport_pc34.c', 'DM1_V2_ELEMENT_DOOR_FRONT'),
]

errors: list[str] = []
anchors = []
for path, needle, line in REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    if line < 1 or line > len(lines):
        errors.append(f'line out of range {path.name}:{line}')
        continue
    line_text = lines[line - 1].strip()
    if needle not in line_text:
        errors.append(f'line anchor mismatch {path.name}:{line}: expected {needle!r}, got {line_text!r}')
    anchors.append({'file': path.name, 'line': line, 'needle': needle, 'text': line_text})

for path, needle in FIRESTAFF_REQUIRED:
    text = path.read_text(encoding='utf-8', errors='replace')
    if needle not in text:
        errors.append(f'missing Firestaff V2 viewport/wall source-lock anchor {needle!r} in {path.name}')

result = {
    'status': 'failed' if errors else 'passed',
    'seam': 'dm1_v2_viewport_wall_occlusion',
    'description': 'V2 viewport wall parity flip and D3C wall occlusion termination source-locked to ReDMCSB DUNVIEW.C, distinct from command/movement.',
    'anchors': anchors,
    'errors': errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
if errors:
    for error in errors:
        print('error:', error)
    raise SystemExit(1)
print(f'dm1_v2_viewport_wall_occlusion_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
