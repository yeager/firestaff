#!/usr/bin/env python3
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
DUNVIEW = SRC / 'DUNVIEW.C'
DUNGEON = SRC / 'DUNGEON.C'
FIRE = ROOT / 'm11_game_view.c'
REPORT = ROOT / 'parity-evidence/pass490_dm1_v1_wall_occlusion_merge_readiness.md'
MANIFEST = ROOT / 'parity-evidence/verification/pass490_dm1_v1_wall_occlusion_merge_readiness/manifest.json'

checks = [
    ('redmcsb-f0128-entry', DUNVIEW, 'void F0128_DUNGEONVIEW_Draw_CPSF'),
    ('redmcsb-d4-before-d3', DUNVIEW, 'M598_VIEW_SQUARE_D4L'),
    ('redmcsb-d3l-d3r-d3c-order', DUNVIEW, 'F0116_DUNGEONVIEW_DrawSquareD3L'),
    ('redmcsb-d2-row-order', DUNVIEW, 'F0119_DUNGEONVIEW_DrawSquareD2L'),
    ('redmcsb-d1-row-order', DUNVIEW, 'F0122_DUNGEONVIEW_DrawSquareD1L'),
    ('redmcsb-d0-center-last', DUNVIEW, 'F0127_DUNGEONVIEW_DrawSquareD0C'),
    ('redmcsb-center-wall-return', DUNVIEW, 'F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C]'),
    ('redmcsb-side-wall-return-left', DUNVIEW, 'F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0214_ai_SquareAspect[M551_RIGHT_WALL_ORNAMENT_ORDINAL]'),
    ('redmcsb-side-wall-return-right', DUNVIEW, 'F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0216_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL]'),
    ('redmcsb-square-aspect-wall-door', DUNGEON, 'case C04_ELEMENT_DOOR:'),
    ('firestaff-nearest-center-blocker', FIRE, 'm11_dm1_nearest_blocking_center_depth_index'),
    ('firestaff-front-wall-depth-collapse', FIRE, 'm11_draw_dm1_front_walls'),
    ('firestaff-side-lane-clear', FIRE, 'm11_dm1_side_lane_clear_before_depth'),
    ('firestaff-side-content-center-blocker', FIRE, 'blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);'),
    ('firestaff-side-door-occlusion', FIRE, 'm11_draw_dm1_side_doors'),
]

def find_line(path: Path, needle: str):
    text = path.read_text(errors='replace').splitlines()
    for idx, line in enumerate(text, 1):
        if needle in line:
            return idx
    return None

results = []
failed = []
for cid, path, needle in checks:
    line = find_line(path, needle)
    ok = line is not None
    results.append({'id': cid, 'file': str(path), 'needle': needle, 'line': line, 'ok': ok})
    if not ok:
        failed.append(cid)

order_needles = [
    'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject',
    'F0116_DUNGEONVIEW_DrawSquareD3L',
    'F0117_DUNGEONVIEW_DrawSquareD3R',
    'F0118_DUNGEONVIEW_DrawSquareD3C_CPSF',
    'F0119_DUNGEONVIEW_DrawSquareD2L',
    'F0120_DUNGEONVIEW_DrawSquareD2R_CPSF',
    'F0121_DUNGEONVIEW_DrawSquareD2C',
    'F0122_DUNGEONVIEW_DrawSquareD1L',
    'F0123_DUNGEONVIEW_DrawSquareD1R',
    'F0124_DUNGEONVIEW_DrawSquareD1C',
    'F0125_DUNGEONVIEW_DrawSquareD0L',
    'F0126_DUNGEONVIEW_DrawSquareD0R',
    'F0127_DUNGEONVIEW_DrawSquareD0C',
]

# Scope ordering to F0128 so earlier helper references do not satisfy the order gate.
dunview_lines = DUNVIEW.read_text(errors='replace').splitlines()
f0128_start = find_line(DUNVIEW, 'void F0128_DUNGEONVIEW_Draw_CPSF') or 1
order_lines = []
for n in order_needles:
    found = None
    for idx in range(f0128_start, len(dunview_lines) + 1):
        if n in dunview_lines[idx - 1]:
            found = idx
            break
    order_lines.append(found)
order_ok = all(x is not None for x in order_lines) and order_lines == sorted(order_lines)
results.append({'id': 'redmcsb-f0128-far-to-near-order', 'file': str(DUNVIEW), 'lines': order_lines, 'ok': order_ok})
if not order_ok:
    failed.append('redmcsb-f0128-far-to-near-order')

MANIFEST.parent.mkdir(parents=True, exist_ok=True)
MANIFEST.write_text(json.dumps({
    'pass': 'pass490_dm1_v1_wall_occlusion_merge_readiness',
    'ok': not failed,
    'source_root': str(SRC),
    'results': results,
    'salvageable_prior_commits': [
        '31c6584 pass365: gate DM1 V1 side-lane occlusion',
        'f1a926e pass368: gate solid wall occlusion',
        'fc62317 pass372: lock dm1 viewport door occlusion source follow-up',
        '76e7fa9 pass395: Lock DM1 viewport wall source runtime path',
        'c363bd1 pass401: source-lock viewport door occlusion',
        '1e0dba7 pass402: order DM1 side viewport contents',
        'e3ee04e pass404: preserve near side contents before center blockers',
        '8a2ce63 pass442: consolidate DM1 viewport wall merge readiness',
    ],
}, indent=2) + '\n')

if failed:
    print('FAIL pass490_dm1_v1_wall_occlusion_merge_readiness ' + ','.join(failed), file=sys.stderr)
    sys.exit(1)

print('PASS pass490_dm1_v1_wall_occlusion_merge_readiness')
print(f'manifest={MANIFEST.relative_to(ROOT)}')
print(f'report={REPORT.relative_to(ROOT)}')
for r in results:
    if 'line' in r:
        print(f"- {r['id']}: {Path(r['file']).name}:{r['line']}")
