#!/usr/bin/env python3
from __future__ import annotations

import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SRC = Path.home() / '.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source'
DUNVIEW = SRC / 'DUNVIEW.C'
LOCAL_C = ROOT / 'dm1_v1_viewport_3d_pc34_compat.c'
LOCAL_PROBE = ROOT / 'probes/dm1/firestaff_dm1_v1_wall_composition_contract_probe.c'
MANIFEST = ROOT / 'parity-evidence/verification/pass496_dm1_v1_wall_occlusion_spec_matrix/manifest.json'

SOURCE_CHECKS = [
    {'id': 'd3l2-side-wall-flip-zone-return', 'function': 'F0676_DrawD3L2', 'needles': ['G2107_WallSet[C10_WALL_D3R2], C702_ZONE_WALL_D3L2', 'G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2', 'M551_RIGHT_WALL_ORNAMENT_ORDINAL', 'return;']},
    {'id': 'd3r2-side-wall-flip-zone-return', 'function': 'F0677_DrawD3R2', 'needles': ['G2107_WallSet[C11_WALL_D3L2], C703_ZONE_WALL_D3R2', 'G2107_WallSet[C10_WALL_D3R2], C703_ZONE_WALL_D3R2', 'M553_LEFT_WALL_ORNAMENT_ORDINAL', 'return;']},
    {'id': 'd3l-front-alcove-or-return', 'function': 'F0116_DUNGEONVIEW_DrawSquareD3L', 'needles': ['G2107_WallSet[C12_WALL_D3R], C705_ZONE_WALL_D3L', 'G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L', 'M577_VIEW_WALL_D3L_FRONT', 'C0x0000_CELL_ORDER_ALCOVE', 'return;']},
    {'id': 'd3c-center-wall-with-alcove-exception', 'function': 'F0118_DUNGEONVIEW_DrawSquareD3C_CPSF', 'needles': ['G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C', 'M578_VIEW_WALL_D3C_FRONT', 'C0x0000_CELL_ORDER_ALCOVE', 'return;']},
    {'id': 'd2c-center-wall-with-alcove-exception', 'function': 'F0121_DUNGEONVIEW_DrawSquareD2C', 'needles': ['G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C', 'M583_VIEW_WALL_D2C_FRONT', 'C0x0000_CELL_ORDER_ALCOVE', 'return;']},
    {'id': 'd1c-center-wall-front-alcove-no-hard-return', 'function': 'F0124_DUNGEONVIEW_DrawSquareD1C', 'needles': ['G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C', 'M587_VIEW_WALL_D1C_FRONT', 'C0x0000_CELL_ORDER_ALCOVE']},
    {'id': 'd0l-near-side-wall-flip-zone-return', 'function': 'F0125_DUNGEONVIEW_DrawSquareD0L', 'needles': ['G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L', 'G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L', 'return;']},
    {'id': 'd0r-near-side-wall-flip-zone-return', 'function': 'F0126_DUNGEONVIEW_DrawSquareD0R', 'needles': ['G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R', 'G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R', 'return;']},
]

LOCAL_CHECKS = [
    ('local-spec-table-present', LOCAL_C, 'static const DM1_ViewportWallDrawSpec s_wall_draw_specs[]'),
    ('local-d3l2-spec-lock', LOCAL_C, 'DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2'),
    ('local-d3c-center-alcove-spec-lock', LOCAL_C, 'DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C'),
    ('local-d1c-no-hard-return-spec-lock', LOCAL_C, 'DM1_PC34_ZONE_WALL_D1C,  false, true'),
    ('probe-wall-matrix-present', LOCAL_PROBE, 'wallCompositionMatrix source='),
    ('probe-d3l2-expected-anchor', LOCAL_PROBE, 'DUNVIEW.C:6254-6260'),
    ('probe-d0r-expected-anchor', LOCAL_PROBE, 'DUNVIEW.C:8126-8139'),
]

def lines(path: Path) -> list[str]:
    return path.read_text(errors='replace').splitlines()

def find_line(haystack: list[str], needle: str, start: int = 1) -> int | None:
    for idx in range(max(start, 1), len(haystack) + 1):
        if needle in haystack[idx - 1]:
            return idx
    return None

def source_window(all_lines: list[str], function: str) -> tuple[int, int, list[str]]:
    start = find_line(all_lines, 'void ' + function + '(')
    if start is None:
        start = find_line(all_lines, 'void ' + function)
    if start is not None and start < 2200:
        start = find_line(all_lines, 'void ' + function + '(', start + 1) or find_line(all_lines, 'void ' + function, start + 1)
    if start is None:
        return 0, 0, []
    end = len(all_lines)
    for idx in range(start + 1, len(all_lines) + 1):
        line = all_lines[idx - 1]
        if (line.startswith('void F') or line.startswith('STATICFUNCTION void F')) and '(' in line:
            end = idx - 1
            break
    return start, end, all_lines[start - 1:end]

def main() -> int:
    dunview_lines = lines(DUNVIEW)
    results = []
    failed: list[str] = []

    for check in SOURCE_CHECKS:
        start, end, window = source_window(dunview_lines, check['function'])
        text = '\n'.join(window)
        missing = [needle for needle in check['needles'] if needle not in text]
        ok = start != 0 and not missing
        results.append({'id': check['id'], 'ok': ok, 'file': str(DUNVIEW), 'function': check['function'], 'line_range': [start, end], 'missing': missing, 'why': 'ReDMCSB wall branch keeps PC34 bitmap/zone/parity and occlusion return or alcove exception contract.'})
        if not ok:
            failed.append(check['id'])

    for cid, path, needle in LOCAL_CHECKS:
        file_lines = lines(path)
        line = find_line(file_lines, needle)
        ok = line is not None
        results.append({'id': cid, 'ok': ok, 'file': str(path), 'line': line, 'needle': needle})
        if not ok:
            failed.append(cid)

    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps({'pass': 'pass496_dm1_v1_wall_occlusion_spec_matrix', 'ok': not failed, 'source_root': str(SRC), 'results': results}, indent=2, sort_keys=True) + '\n')

    if failed:
        print('FAIL pass496_dm1_v1_wall_occlusion_spec_matrix ' + ','.join(failed), file=sys.stderr)
        print(f'manifest={MANIFEST.relative_to(ROOT)}', file=sys.stderr)
        return 1

    print('PASS pass496_dm1_v1_wall_occlusion_spec_matrix')
    print(f'manifest={MANIFEST.relative_to(ROOT)}')
    for result in results:
        if 'line_range' in result:
            start, end = result['line_range']
            print(f"- {result['id']}: DUNVIEW.C:{start}-{end}")
        else:
            print(f"- {result['id']}: {Path(result['file']).name}:{result.get('line')}")
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
