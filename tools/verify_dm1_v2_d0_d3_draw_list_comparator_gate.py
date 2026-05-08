#!/usr/bin/env python3
"""Gate the DM1 V2 D0-D3 renderer draw-list/comparator scaffold.

This verifies the new renderer-side draw-list emission is source-locked to the
ReDMCSB DUNVIEW.C D0-D3 traversal and that the comparator is explicitly a
matched-state scaffold, not a pixel parity claim.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
DM1_DUNGEON_DAT = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT")
EVIDENCE = ROOT / 'parity-evidence/verification/pass274_dm1_v2_d0_d3_draw_list_comparator_gate.json'

SOURCE_ANCHORS = [
    ('DEFS.H', 992, 'unsigned char MapCount;'),
    ('DEFS.H', 995, 'unsigned int16_t InitialPartyLocation;'),
    ('LOADSAVE.C', 1940, 'if (G0298_B_NewGame)'),
    ('DUNGEON.C', 35, 'G0233_ai_Graphic559_DirectionToStepEastCount'),
    ('DUNGEON.C', 40, 'G0234_ai_Graphic559_DirectionToStepNorthCount'),
    ('DUNGEON.C', 1389, '*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount'),
    ('DUNGEON.C', 1390, 'P0253_i_Direction += 1'),
    ('DUNGEON.C', 1391, '*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount'),
    ('DUNVIEW.C', 8337, 'if (G0297_B_DrawFloorAndCeilingRequested)'),
    ('DUNVIEW.C', 8338, 'F0098_DUNGEONVIEW_DrawFloorAndCeiling();'),
    ('DUNVIEW.C', 8490, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, -1'),
    ('DUNVIEW.C', 8491, 'F0116_DUNGEONVIEW_DrawSquareD3L'),
    ('DUNVIEW.C', 8494, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, 1'),
    ('DUNVIEW.C', 8495, 'F0117_DUNGEONVIEW_DrawSquareD3R'),
    ('DUNVIEW.C', 8498, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, 0'),
    ('DUNVIEW.C', 8499, 'F0118_DUNGEONVIEW_DrawSquareD3C_CPSF'),
    ('DUNVIEW.C', 8512, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, -1'),
    ('DUNVIEW.C', 8513, 'F0119_DUNGEONVIEW_DrawSquareD2L'),
    ('DUNVIEW.C', 8516, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, 1'),
    ('DUNVIEW.C', 8517, 'F0120_DUNGEONVIEW_DrawSquareD2R_CPSF'),
    ('DUNVIEW.C', 8520, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 2, 0'),
    ('DUNVIEW.C', 8521, 'F0121_DUNGEONVIEW_DrawSquareD2C'),
    ('DUNVIEW.C', 8524, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, -1'),
    ('DUNVIEW.C', 8525, 'F0122_DUNGEONVIEW_DrawSquareD1L'),
    ('DUNVIEW.C', 8528, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 1'),
    ('DUNVIEW.C', 8529, 'F0123_DUNGEONVIEW_DrawSquareD1R'),
    ('DUNVIEW.C', 8532, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 1, 0'),
    ('DUNVIEW.C', 8533, 'F0124_DUNGEONVIEW_DrawSquareD1C'),
    ('DUNVIEW.C', 8536, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, -1'),
    ('DUNVIEW.C', 8537, 'F0125_DUNGEONVIEW_DrawSquareD0L'),
    ('DUNVIEW.C', 8540, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 0, 1'),
    ('DUNVIEW.C', 8541, 'F0126_DUNGEONVIEW_DrawSquareD0R'),
    ('DUNVIEW.C', 8542, 'F0127_DUNGEONVIEW_DrawSquareD0C'),
    ('DUNVIEW.C', 6666, 'case C19_ELEMENT_STAIRS_FRONT'),
    ('DUNVIEW.C', 6697, 'case C00_ELEMENT_WALL'),
    ('DUNVIEW.C', 6721, 'case C17_ELEMENT_DOOR_FRONT'),
    ('DUNVIEW.C', 6816, 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
    ('DUNVIEW.C', 6828, 'F0113_DUNGEONVIEW_DrawField'),
]

FIRESTAFF_ANCHORS = [
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_MAX_DRAW_COMMANDS'),
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_VIEW_SQUARE_D0C'),
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_DrawCommand'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_emit_d0_d3_draw_list'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_compare_draw_lists'),
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_DungeonStateFixture'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_build_composition_from_fixture'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_compare_viewport_region'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:8337-8338'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:8490-8542'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:6697-6720'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:6816'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNGEON.C:1371-1391'),
    ('dm1_v2_viewport_renderer_pc34.c', 'dm1_pc34_entry_portrait_wall'),
    ('dm1_v2_viewport_renderer_pc34.c', 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85'),
    ('dm1_v2_viewport_renderer_pc34.c', 'dm1_v2_vp_compare_draw_lists'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_d0_d3_draw_list_comparator_source_lock'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'DM1_V2_VIEW_SQUARE_D0C'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_real_state_fixture_draw_list'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_region_comparator_scaffold'),
    ('CMakeLists.txt', 'dm1_v2_d0_d3_draw_list_comparator_gate'),
    ('tools/verify_dm1_v2_completion_matrix.py', 'dm1_v2_d0_d3_draw_list_comparator_gate'),
]

EXPECTED_ORDER = [
    'DM1_V2_VIEW_SQUARE_D3L', 'DM1_V2_VIEW_SQUARE_D3R', 'DM1_V2_VIEW_SQUARE_D3C',
    'DM1_V2_VIEW_SQUARE_D2L', 'DM1_V2_VIEW_SQUARE_D2R', 'DM1_V2_VIEW_SQUARE_D2C',
    'DM1_V2_VIEW_SQUARE_D1L', 'DM1_V2_VIEW_SQUARE_D1R', 'DM1_V2_VIEW_SQUARE_D1C',
    'DM1_V2_VIEW_SQUARE_D0L', 'DM1_V2_VIEW_SQUARE_D0R', 'DM1_V2_VIEW_SQUARE_D0C',
]


def main() -> int:
    errors: list[str] = []
    source_checks = []
    for filename, line, needle in SOURCE_ANCHORS:
        text = (SOURCE / filename).read_text(encoding='utf-8', errors='replace')
        lines = text.splitlines()
        actual = lines[line - 1].strip() if 1 <= line <= len(lines) else ''
        if needle not in actual:
            errors.append(f'{filename}:{line}: expected {needle!r}, got {actual!r}')
        source_checks.append({'file': filename, 'line': line, 'needle': needle, 'text': actual})

    for rel, needle in FIRESTAFF_ANCHORS:
        text = (ROOT / rel).read_text(encoding='utf-8', errors='replace')
        if needle not in text:
            errors.append(f'missing Firestaff anchor {needle!r} in {rel}')

    if not DM1_DUNGEON_DAT.is_file():
        errors.append(f'missing canonical DM1 DUNGEON.DAT: {DM1_DUNGEON_DAT}')
        dungeon_sha = None
        initial_state = None
    else:
        dungeon_bytes = DM1_DUNGEON_DAT.read_bytes()
        dungeon_sha = hashlib.sha256(dungeon_bytes).hexdigest()
        if dungeon_sha != 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85':
            errors.append(f'DM1 DUNGEON.DAT sha256 mismatch: {dungeon_sha}')
        initial = int.from_bytes(dungeon_bytes[8:10], 'little') if len(dungeon_bytes) >= 10 else -1
        initial_state = {'mapIndex': 0, 'mapX': initial & 31, 'mapY': (initial >> 5) & 31, 'direction': (initial >> 10) & 3, 'rawLE': f'0x{initial:04X}'}
        if initial_state != {'mapIndex': 0, 'mapX': 1, 'mapY': 3, 'direction': 2, 'rawLE': '0x0861'}:
            errors.append(f'DM1 entry state mismatch: {initial_state}')

    c_text = (ROOT / 'dm1_v2_viewport_renderer_pc34.c').read_text(encoding='utf-8', errors='replace')
    order_start = c_text.find('static DM1_V2_ViewSquare dm1_v2_vp_square_id')
    order_end = c_text.find('static int dm1_v2_vp_push_draw')
    order_window = c_text[order_start:order_end]
    positions = []
    last = -1
    for symbol in EXPECTED_ORDER:
        pos = order_window.find(symbol)
        positions.append({'symbol': symbol, 'offset': pos})
        if pos < 0:
            errors.append(f'missing draw-list square symbol {symbol}')
        elif pos <= last:
            errors.append(f'out-of-order draw-list square symbol {symbol}')
        last = pos

    result = {
        'status': 'failed' if errors else 'passed',
        'scope': 'DM1 V2 renderer-side D0-D3 draw-list emission and matched-state comparator scaffold',
        'redmcsbSourceRoot': str(SOURCE),
        'sourceAnchors': source_checks,
        'firestaffOrder': positions,
        'matchedDungeonStateFixture': {
            'name': 'dm1_pc34_entry_portrait_wall',
            'source': str(DM1_DUNGEON_DAT),
            'sha256': dungeon_sha,
            'initialState': initial_state,
            'frontWallSquare': {'mapIndex': 0, 'mapX': 1, 'mapY': 4, 'relativeSquare': 'D1C'},
            'status': 'source-locked sparse fixture feeds mapX/mapY/direction into renderer draw-list; full DUNGEON.DAT square decoder still pending',
        },
        'comparatorStatus': 'exact draw-command comparator plus viewport-region pixel comparator scaffold only; no original/ReDMCSB/Firestaff pixel parity claim',
        'nextBlockers': [
            'Replace sparse source-locked entry fixture with full DUNGEON.DAT square decoder.',
            'Capture original/ReDMCSB and Firestaff viewport pixels for identical mapX/mapY/direction.',
            'Feed captured original/ReDMCSB and Firestaff framebuffers into the region comparator; only then promote to pixel parity.',
        ],
        'errors': errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    if errors:
        for error in errors:
            print('error:', error)
        return 1
    print(f'dm1_v2_d0_d3_draw_list_comparator_gate: ok evidence={EVIDENCE.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
