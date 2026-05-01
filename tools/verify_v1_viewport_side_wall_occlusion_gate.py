#!/usr/bin/env python3
"""Verify DM1 V1 side-wall near-lane occlusion stays source-locked.

This gate covers a narrow invariant outside the D1C/D2C/D3C front-wall gate:
ReDMCSB draws side squares far-to-near, and a near D1L/D1R wall branch draws the
opaque side wall then returns.  Therefore farther same-side open-cell contents
must not be drawn over a nearer side wall in Firestaff's split renderer.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'm11_game_view.c'
REDMCSB_DUNVIEW = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C'


def line_no(text: str, offset: int) -> int:
    return text.count('\n', 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r'\b(?:static\s+)?(?:int|void)\s+' + re.escape(name) + r'\s*\(')
    for m in pattern.finditer(text):
        brace = text.find('{', m.end())
        if brace < 0:
            continue
        semicolon = text.find(';', m.end(), brace)
        if semicolon >= 0:
            continue
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
                if depth == 0:
                    return m.start(), i + 1, text[m.start():i + 1]
    raise AssertionError(f'missing function body for {name}')


def find_redmcsb_region(text: str, name: str) -> tuple[int, int, str]:
    pattern = re.compile(r'(?m)^(?:STATICFUNCTION\s+void|void)\s+' + re.escape(name) + r'\s*\(')
    next_pattern = re.compile(r'(?m)^\s*(?:STATICFUNCTION\s+void|void)\s+F\d{4}_')
    for match in pattern.finditer(text):
        brace = text.find('{', match.end())
        if brace < 0:
            continue
        semicolon = text.find(';', match.end(), brace)
        if semicolon >= 0:
            continue
        line_start = text.rfind('\n', 0, match.start()) + 1
        end_match = next_pattern.search(text, brace + 1)
        end = end_match.start() if end_match else len(text)
        return line_start, end, text[line_start:end]
    raise AssertionError(f'missing ReDMCSB region {name}')

def require_in_order(body: str, markers: list[tuple[str, str]], label: str) -> None:
    last = -1
    last_name = ''
    for name, needle in markers:
        pos = body.find(needle)
        if pos < 0:
            raise AssertionError(f'{label}: missing {name}: {needle!r}')
        if pos <= last:
            raise AssertionError(f'{label}: {name} appears before/at {last_name}')
        last = pos
        last_name = name


def main() -> int:
    text = SRC.read_text(encoding='utf-8')
    red = REDMCSB_DUNVIEW.read_text(encoding='utf-8')
    ok: list[str] = []

    f0128_start, _f0128_end, f0128 = find_redmcsb_region(red, 'F0128_DUNGEONVIEW_Draw_CPSF')
    require_in_order(
        f0128,
        [
            ('D2L side square', 'F0119_DUNGEONVIEW_DrawSquareD2L'),
            ('D2R side square', 'F0120_DUNGEONVIEW_DrawSquareD2R_CPSF'),
            ('D1L side square', 'F0122_DUNGEONVIEW_DrawSquareD1L'),
            ('D1R side square', 'F0123_DUNGEONVIEW_DrawSquareD1R'),
        ],
        'ReDMCSB far-to-near side draw order',
    )
    ok.append(f'ReDMCSB F0128 far-to-near side order: {REDMCSB_DUNVIEW.name}:{line_no(red, f0128_start)}')

    d1l_start, _d1l_end, d1l = find_redmcsb_region(red, 'F0122_DUNGEONVIEW_DrawSquareD1L')
    d1r_start, _d1r_end, d1r = find_redmcsb_region(red, 'F0123_DUNGEONVIEW_DrawSquareD1R')
    require_in_order(
        d1l,
        [
            ('D1L wall case', 'case C00_ELEMENT_WALL:'),
            ('D1L opaque side-wall blit', 'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3008_i_WallSet_Wall_D1L'),
            ('D1L near wall returns', 'return;'),
            ('D1L open-cell content branch', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
        ],
        'ReDMCSB D1L near-wall branch',
    )
    require_in_order(
        d1r,
        [
            ('D1R wall case', 'case C00_ELEMENT_WALL:'),
            ('D1R opaque side-wall blit', 'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3009_i_WallSet_Wall_D1R'),
            ('D1R near wall returns', 'return;'),
            ('D1R open-cell content branch', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
        ],
        'ReDMCSB D1R near-wall branch',
    )
    ok.append(f'ReDMCSB D1L/D1R wall-return evidence: {REDMCSB_DUNVIEW.name}:{line_no(red, d1l_start)}, {line_no(red, d1r_start)}')

    helper_start, _helper_end, helper = find_function(text, 'm11_dm1_side_lane_wall_clear_before_depth')
    for token in [
        'for (d = 0; d < depthIndex && d < 3; ++d)',
        'm11_viewport_cell_is_wall_like(&cells[d][sideIndex])',
        'return 0;',
    ]:
        if token not in helper:
            raise AssertionError(f'Firestaff side-lane wall helper missing {token!r}')
    rel_helper_start, _rel_helper_end, rel_helper = find_function(text, 'm11_dm1_side_lane_wall_clear_for_rel')
    for token in [
        'relSide == 0 || relForward <= 0',
        'relForward - 1',
        'relSide < 0 ? 0 : 2',
    ]:
        if token not in rel_helper:
            raise AssertionError(f'Firestaff side-lane relative helper missing {token!r}')
    ok.append(f'Firestaff side-lane wall occlusion helpers: m11_game_view.c:{line_no(text, helper_start)}, {line_no(text, rel_helper_start)}')

    contents_start, _contents_end, contents = find_function(text, 'm11_draw_dm1_side_contents')
    require_in_order(
        contents,
        [
            ('side index selected', 'int sideIndex = side < 0 ? 0 : 2;'),
            ('open-cell guard', '!cell->valid || !m11_viewport_cell_is_open(cell)'),
            ('near side-wall guard', '!m11_dm1_side_lane_wall_clear_before_depth(cells, depth, sideIndex)'),
            ('item draw section', 'if (cell->floorItemCount > 0)'),
            ('creature draw section', 'if (cell->creatureGroupCount > 0)'),
            ('projectile draw section', 'if (cell->summary.projectiles > 0)'),
        ],
        'Firestaff side contents same-lane near-wall guard before drawing',
    )
    ok.append(f'Firestaff side contents guard before item/creature/projectile draw: m11_game_view.c:{line_no(text, contents_start)}')

    side_walls_start, _side_walls_end, side_walls = find_function(text, 'm11_draw_dm1_side_walls')
    require_in_order(
        side_walls,
        [
            ('D3L2 side wall first', '{3, 3, -2, M11_GFX_WALLSET0_D3L2'),
            ('D2L2 side wall after D3', '{2, 2, -2, M11_GFX_WALLSET0_D2L2'),
            ('D1L side wall after D2', '{1, 1, -1, M11_GFX_WALLSET0_D1L'),
            ('D0L side wall nearest', '{0, 0, -1, M11_GFX_WALLSET0_D0L'),
            ('opaque wall blit', 'm11_draw_dm1_wall_blit_with_transparency'),
        ],
        'Firestaff side wall far-to-near opaque blits',
    )
    if '-1);' not in side_walls:
        raise AssertionError('Firestaff side wall blit does not pass -1 no-transparency key')
    if 'm11_dm1_side_lane_wall_clear_for_rel(cells,' not in side_walls:
        raise AssertionError('Firestaff side-wall panels are not guarded by same-lane near-wall occlusion')
    ok.append(f'Firestaff side-wall opaque far-to-near blits guarded by same-lane occlusion: m11_game_view.c:{line_no(text, side_walls_start)}')

    ornaments_start, _ornaments_end, ornaments = find_function(text, 'm11_draw_dm1_wall_ornaments')
    require_in_order(
        ornaments,
        [
            ('bounded replay limit', 'maxVisibleForwardLimit'),
            ('same-lane guard', 'm11_dm1_side_lane_wall_clear_for_rel(cells,'),
            ('sample side/front wall cell', 'm11_sample_viewport_cell(state, kWallOrnaments[i].relForward'),
            ('ornament asset blit', 'm11_blit_scaled_palette_map_maybe_flip'),
            ('alcove item draw', 'm11_draw_dm1_alcove_wall_items'),
        ],
        'Firestaff wall ornaments/alcove items same-lane near-wall guard',
    )
    ok.append(f'Firestaff wall ornament/alcove item bounded same-lane guard: m11_game_view.c:{line_no(text, ornaments_start)}')

    for fn in ['m11_draw_dm1_side_doors', 'm11_draw_dm1_side_door_ornaments', 'm11_draw_dm1_side_destroyed_door_masks']:
        fn_start, _fn_end, body = find_function(text, fn)
        require_in_order(
            body,
            [
                ('same-lane guard', 'm11_dm1_side_lane_wall_clear_for_rel(cells,'),
                ('sample side door cell', 'm11_sample_viewport_cell(state, kSpecs[i].relForward'),
            ],
            f'Firestaff {fn} same-lane near-wall guard',
        )
        ok.append(f'Firestaff side door/ornament/mask guard in {fn}: m11_game_view.c:{line_no(text, fn_start)}')

    draw_start, _draw_end, draw = find_function(text, 'm11_draw_viewport')
    for call, arg in [
        ('m11_draw_dm1_side_walls', 'maxVisibleForward, cells'),
        ('m11_draw_dm1_side_doors', 'maxVisibleForward, cells'),
        ('m11_draw_dm1_side_door_ornaments', 'maxVisibleForward, cells'),
        ('m11_draw_dm1_side_destroyed_door_masks', 'maxVisibleForward, cells'),
    ]:
        pos = draw.find(call + '(state, framebuffer, framebufferWidth, framebufferHeight,')
        if pos < 0:
            raise AssertionError(f'Firestaff draw viewport missing side feature call {call}')
        snippet = draw[pos:pos + 220]
        if arg not in snippet:
            raise AssertionError(f'Firestaff draw viewport missing side occlusion cells argument for {call}')
    normal_orn = draw.find('m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,')
    if normal_orn < 0 or 'maxVisibleForward, cells' not in draw[normal_orn:normal_orn + 180]:
        raise AssertionError('Firestaff draw viewport missing normal wall ornament maxVisibleForward/cells arguments')
    replay_orn = draw.find('m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,', normal_orn + 1)
    if replay_orn < 0 or 'nearMaxVisibleForward, cells' not in draw[replay_orn:replay_orn + 180]:
        raise AssertionError('Firestaff center-occluder replay does not bound wall ornaments to nearer side layers')
    if 'm11_dm1_nearest_blocking_center_depth_index(cells)' not in draw:
        raise AssertionError('Firestaff draw viewport missing nearest blocking center replay trigger')
    ok.append(f'Firestaff side feature calls receive sampled cells and replay is near-bound: m11_game_view.c:{line_no(text, draw_start)}')

    print('V1 viewport side-wall occlusion source-shape verification passed')
    for line in ok:
        print(f'- {line}')
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f'FAIL: {exc}')
        raise SystemExit(1)
