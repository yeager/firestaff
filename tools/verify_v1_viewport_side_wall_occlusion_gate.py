#!/usr/bin/env python3
"""Verify DM1 V1 side-wall near-lane occlusion stays source-locked.

This gate covers a narrow invariant outside the D1C/D2C/D3C front-wall gate:
ReDMCSB draws side squares far-to-near, and a near D1L/D1R wall branch draws the
C10-keyed side wall then returns.  Therefore farther same-side open-cell contents
must not be drawn over a nearer side wall in Firestaff's split renderer.
"""
from __future__ import annotations

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src/engine/m11_game_view.c'
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


def find_static_array(text: str, name: str) -> tuple[int, int, str]:
    needle = name + '[]'
    start = text.find(needle)
    if start < 0:
        raise AssertionError(f'missing static array for {name}')
    brace = text.find('{', start)
    if brace < 0:
        raise AssertionError(f'missing initializer for {name}')
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                line_start = text.rfind('\n', 0, start) + 1
                return line_start, i + 1, text[line_start:i + 1]
    raise AssertionError(f'unterminated static array for {name}')


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
            ('D1L C10-keyed side-wall blit', 'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3008_i_WallSet_Wall_D1L'),
            ('D1L near wall returns', 'return;'),
            ('D1L open-cell content branch', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
        ],
        'ReDMCSB D1L near-wall branch',
    )
    require_in_order(
        d1r,
        [
            ('D1R wall case', 'case C00_ELEMENT_WALL:'),
            ('D1R C10-keyed side-wall blit', 'F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G3009_i_WallSet_Wall_D1R'),
            ('D1R near wall returns', 'return;'),
            ('D1R open-cell content branch', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
        ],
        'ReDMCSB D1R near-wall branch',
    )
    ok.append(f'ReDMCSB D1L/D1R wall-return evidence: {REDMCSB_DUNVIEW.name}:{line_no(red, d1l_start)}, {line_no(red, d1r_start)}')

    # 2026-07-20 round 16 re-anchor (same-drift-family): the m11-local
    # before-depth rescan helper was replaced by the shared PC34 lane
    # visibility receipt; m11_dm1_side_lane_clear_for_rel now delegates to
    # dm1_viewport_3d_side_lane_clear_from_visibility_pc34, which walks the
    # receipt's per-lane open-depth masks.
    rel_helper_start, _rel_helper_end, rel_helper = find_function(text, 'm11_dm1_side_lane_clear_for_rel')
    for token in [
        'm11_dm1_lane_visibility(cells)',
        'dm1_viewport_3d_side_lane_clear_from_visibility_pc34(&visibility,',
        'relForward,',
        'relSide)',
    ]:
        if token not in rel_helper:
            raise AssertionError(f'Firestaff side-lane relative helper missing {token!r}')
    contract = (ROOT / 'src/dm1/dm1_v1_viewport_3d_pc34_compat.c').read_text(encoding='utf-8')
    lane_start, _lane_end, lane = find_function(contract, 'dm1_viewport_3d_side_lane_clear_from_visibility_pc34')
    require_in_order(
        lane,
        [
            ('center side always clear', 'if (rel_side == 0)'),
            ('left lane mask', 'visibility->left_open_depth_mask'),
            ('right lane mask', 'visibility->right_open_depth_mask'),
            ('rel mask walk', 'dm1_viewport_3d_side_lane_clear_for_rel_pc34(rel_forward,'),
        ],
        'Firestaff side-lane visibility contract',
    )
    ok.append(f'Firestaff side-lane non-open occlusion helpers: m11_game_view.c:{line_no(text, rel_helper_start)}, dm1_v1_viewport_3d_pc34_compat.c:{line_no(contract, lane_start)}')

    # 2026-07-20 round 16 re-anchor (same-drift-family): the per-depth side
    # contents pass is m11_draw_dm1_side_contents_at_depth; its same-lane
    # near-wall guard uses the relative helper at depth + 1.
    contents_start, _contents_end, contents = find_function(text, 'm11_draw_dm1_side_contents_at_depth')
    require_in_order(
        contents,
        [
            ('side index selected', 'int sideIndex = side < 0 ? 0 : 2;'),
            ('open-cell guard', '!cell->valid || !m11_viewport_cell_is_open(cell)'),
            ('near side-wall guard', '!m11_dm1_side_lane_clear_for_rel(cells, depth + 1, side)'),
            ('item draw section', 'if (cell->floorItemCount > 0'),
            ('creature draw section', 'if (cell->creatureGroupCount > 0)'),
            ('projectile draw section', 'cell->renderableProjectileCount > 0'),
        ],
        'Firestaff side contents same-lane near-wall guard before drawing',
    )
    ok.append(f'Firestaff side contents guard before item/creature/projectile draw: m11_game_view.c:{line_no(text, contents_start)}')

    # 2026-07-20 round 16 re-anchor (same-drift-family): the side wall
    # far-to-near table moved into the PC34 contract module's
    # s_wall_draw_specs[] (DM1_VIEW_SQUARE_*/DM1_WALL_* rows).
    side_table_start, _side_table_end, side_table = find_static_array(contract, 's_wall_draw_specs')
    require_in_order(
        side_table,
        [
            ('D3L2 side wall first', '{ DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2,'),
            ('D2L2 side wall after D3', '{ DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2,'),
            ('D1L side wall after D2', '{ DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,'),
            ('D0L side wall nearest', '{ DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,'),
        ],
        'Firestaff side wall far-to-near table',
    )
    side_walls_start, _side_walls_end, side_walls = find_function(text, 'm11_draw_dm1_side_walls')
    require_in_order(
        side_walls,
        [
            ('max-visible guard', 'spec->runtime_rel_forward > maxVisibleForward'),
            # Round-14 source review: F0128 draws side walls far-to-near
            # without testing nearer side-lane occupancy; the visibility
            # receipt flows into the DM1-owned host receipt builder instead.
            ('visibility-aware receipt', 'dm1_viewport_3d_build_side_wall_host_receipt_pc34'),
            ('C10-keyed wall blit', 'm11_draw_dm1_side_wall_host_receipt(state'),
        ],
        'Firestaff side wall occlusion guards and C10-keyed blits',
    )
    if 'handoff.transparent_color = 10;' not in contract:
        raise AssertionError('Firestaff side wall blit does not pass C10 transparency key')
    if 'without testing nearer side-lane occupancy' not in side_walls:
        raise AssertionError('Firestaff side-wall pass lost the round-14 source note on same-lane occlusion')
    ok.append(f'Firestaff side-wall C10-keyed far-to-near blits with visibility-aware receipts: dm1_v1_viewport_3d_pc34_compat.c:{line_no(contract, side_table_start)}, m11_game_view.c:{line_no(text, side_walls_start)}')

    ornaments_start, _ornaments_end, ornaments = find_function(text, 'm11_draw_dm1_wall_ornaments')
    require_in_order(
        ornaments,
        [
            ('bounded replay limit', 'maxVisibleForwardLimit'),
            ('max-visible guard', 'spec.relForward > maxVisibleForward'),
            # Round-14 source review: F0107 wall-ornament material follows
            # its wall panel even when a nearer side square is closed; the
            # side-lane-open gate is for floor/content passes only.
            ('side-lane policy note', 'The side-lane-open gate is for floor/content'),
            ('sample side/front wall cell', 'm11_sample_viewport_cell(state, spec.relForward, spec.relSide, &cell)'),
            ('ornament asset blit', 'm11_draw_dm1_wall_ornament_host_material_receipt'),
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
                ('same-lane guard', 'm11_dm1_side_lane_clear_for_rel(cells,'),
                # 2026-07-20 round 16 re-anchor (same-drift-family): side door
                # specs now come from the PC34 render plan, sampled via
                # plan.relForward/plan.relSide.
                ('sample side door cell', 'm11_sample_viewport_cell(state, plan.relForward, plan.relSide, &cell)'),
            ],
            f'Firestaff {fn} same-lane near-wall guard',
        )
        ok.append(f'Firestaff side door/ornament/mask guard in {fn}: m11_game_view.c:{line_no(text, fn_start)}')

    draw_start, _draw_end, draw = find_function(text, 'm11_draw_viewport')
    # 2026-07-20 round 16 re-anchor (same-drift-family): the primary side
    # wall pass now receives the shared lane-visibility receipt (bounded by
    # dm1_viewport_3d_primary_side_wall_max_forward_pc34) instead of the raw
    # sampled cells.
    for call, arg in [
        ('m11_draw_dm1_side_walls', '&visibility, 99);'),
        ('m11_draw_dm1_side_doors', 'maxVisibleForward, cells, 99'),
        ('m11_draw_dm1_side_door_ornaments', 'maxVisibleForward, cells, 99'),
        ('m11_draw_dm1_side_destroyed_door_masks', 'maxVisibleForward, cells, 99'),
    ]:
        pos = draw.find(call + '(state, framebuffer, framebufferWidth, framebufferHeight,')
        if pos < 0:
            raise AssertionError(f'Firestaff draw viewport missing side feature call {call}')
        # The primary wall route has a nested source-bound max-forward
        # expression; keep the inspection span wide enough to reach its
        # explicit any-side filter.
        snippet = draw[pos:pos + 360]
        if arg not in snippet:
            raise AssertionError(f'Firestaff draw viewport missing side occlusion argument {arg!r} for {call}')
    normal_orn = draw.find('m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,')
    if normal_orn < 0 or 'maxVisibleForward, cells, 99' not in draw[normal_orn:normal_orn + 180]:
        raise AssertionError('Firestaff draw viewport missing normal wall ornament maxVisibleForward/cells arguments')
    replay_orn = draw.find('m11_draw_dm1_wall_ornaments(state, framebuffer, framebufferWidth, framebufferHeight,', normal_orn + 1)
    if replay_orn < 0 or 'nearMaxVisibleForward, cells, 99' not in draw[replay_orn:replay_orn + 180]:
        raise AssertionError('Firestaff center-occluder replay does not bound wall ornaments to nearer side layers')
    if 'int blockingCenterDepth = visibility.nearest_blocking_center_depth_index;' not in draw:
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
