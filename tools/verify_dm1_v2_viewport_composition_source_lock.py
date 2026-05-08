#!/usr/bin/env python3
"""Source-lock the next DM1 V2 viewport composition gate to ReDMCSB.

This is intentionally not a screenshot/parity claim. It verifies the source
stack/order that a real D0-D3 V2 renderer must implement after pass271 bound the
wall/floor/ceiling/door/stairs logical assets: floor/ceiling base first,
far-to-near D4->D0 square walk, D3 side/center before D2/D1/D0, and element
branches for wall, door, stairs, fields, ceiling pits and object/creature lanes.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EVIDENCE = ROOT / 'parity-evidence/verification/pass272_dm1_v2_viewport_composition_source_lock.json'

LINE_ANCHORS = [
    ('DUNVIEW.C', 8318, 'void F0128_DUNGEONVIEW_Draw_CPSF'),
    ('DUNVIEW.C', 8337, 'if (G0297_B_DrawFloorAndCeilingRequested)'),
    ('DUNVIEW.C', 8338, 'F0098_DUNGEONVIEW_DrawFloorAndCeiling();'),
    ('DUNVIEW.C', 8357, 'G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001'),
    ('DUNVIEW.C', 8468, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, -1'),
    ('DUNVIEW.C', 8469, 'M598_VIEW_SQUARE_D4L'),
    ('DUNVIEW.C', 8472, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 1'),
    ('DUNVIEW.C', 8473, 'M599_VIEW_SQUARE_D4R'),
    ('DUNVIEW.C', 8476, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 4, 0'),
    ('DUNVIEW.C', 8477, 'M597_VIEW_SQUARE_D4C'),
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
    ('DUNVIEW.C', 6642, 'STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF'),
    ('DUNVIEW.C', 6666, 'case C19_ELEMENT_STAIRS_FRONT'),
    ('DUNVIEW.C', 6697, 'case C00_ELEMENT_WALL'),
    ('DUNVIEW.C', 6721, 'case C17_ELEMENT_DOOR_FRONT'),
    ('DUNVIEW.C', 6816, 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
    ('DUNVIEW.C', 6828, 'F0113_DUNGEONVIEW_DrawField'),
    ('DUNVIEW.C', 7960, 'STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L'),
    ('DUNVIEW.C', 8005, 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
    ('DUNVIEW.C', 8007, 'case C00_ELEMENT_WALL'),
    ('DUNVIEW.C', 8038, 'return;'),
    ('DUNVIEW.C', 8164, 'STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C'),
    ('DUNVIEW.C', 8241, 'case C19_ELEMENT_STAIRS_FRONT'),
    ('DUNVIEW.C', 8294, 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF'),
]

ORDER_SYMBOLS = [
    'M598_VIEW_SQUARE_D4L', 'M599_VIEW_SQUARE_D4R', 'M597_VIEW_SQUARE_D4C',
    'F0116_DUNGEONVIEW_DrawSquareD3L', 'F0117_DUNGEONVIEW_DrawSquareD3R', 'F0118_DUNGEONVIEW_DrawSquareD3C_CPSF',
    'F0119_DUNGEONVIEW_DrawSquareD2L', 'F0120_DUNGEONVIEW_DrawSquareD2R_CPSF', 'F0121_DUNGEONVIEW_DrawSquareD2C',
    'F0122_DUNGEONVIEW_DrawSquareD1L', 'F0123_DUNGEONVIEW_DrawSquareD1R', 'F0124_DUNGEONVIEW_DrawSquareD1C',
    'F0125_DUNGEONVIEW_DrawSquareD0L', 'F0126_DUNGEONVIEW_DrawSquareD0R', 'F0127_DUNGEONVIEW_DrawSquareD0C',
]

PASS271_REQUIRED_LOGICAL_IDS = [
    'fs.v2.shared.dungeon-view.wall.front',
    'fs.v2.shared.dungeon-view.wall.side',
    'fs.v2.shared.dungeon-view.floor.base',
    'fs.v2.shared.dungeon-view.ceiling.base',
    'fs.v2.shared.dungeon-view.door.wood.closed',
    'fs.v2.shared.dungeon-view.stairs.down',
]

FIRESTAFF_REQUIRED = [
    ('CMakeLists.txt', 'dm1_v2_viewport_composition_source_lock'),
    ('tools/verify_dm1_v2_completion_matrix.py', 'dm1_v2_viewport_composition_source_lock'),
    ('tools/verify_dm1_v2_dungeon_view_asset_bindings.py', 'REQUIRED_LOGICAL_IDS'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:8357'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNVIEW.C:6697-6720'),
]


def read(path: Path) -> str:
    return path.read_text(encoding='utf-8', errors='replace')


def source_line(text: str, line: int) -> str:
    lines = text.splitlines()
    if line < 1 or line > len(lines):
        raise ValueError(f'line out of range: {line}')
    return lines[line - 1].strip()


def line_anchor_checks() -> tuple[list[dict], list[str]]:
    errors: list[str] = []
    anchors: list[dict] = []
    cache: dict[str, str] = {}
    for filename, line, needle in LINE_ANCHORS:
        text = cache.setdefault(filename, read(SOURCE / filename))
        try:
            actual = source_line(text, line)
        except ValueError as exc:
            errors.append(f'{filename}:{line}: {exc}')
            actual = ''
        if needle not in actual:
            errors.append(f'{filename}:{line}: expected {needle!r}, got {actual!r}')
        anchors.append({'file': filename, 'line': line, 'needle': needle, 'text': actual})
    return anchors, errors


def order_checks(dunview: str) -> tuple[list[dict], list[str]]:
    errors: list[str] = []
    window = '\n'.join(dunview.splitlines()[8465:8543])
    positions: list[dict] = []
    last = -1
    for symbol in ORDER_SYMBOLS:
        pos = window.find(symbol)
        if pos < 0:
            errors.append(f'missing order symbol in F0128 draw window: {symbol}')
        elif pos <= last:
            errors.append(f'out-of-order symbol in F0128 draw window: {symbol}')
        positions.append({'symbol': symbol, 'offset': pos})
        last = pos
    return positions, errors


def pass271_binding_checks() -> tuple[dict[str, str], list[str]]:
    errors: list[str] = []
    bindings: dict[str, str] = {}
    catalog = json.loads(read(ROOT / 'assets-v2/catalog/logical-ids/shared-v2-logical-ids.json'))
    for category in catalog.get('categories', []):
        for entry in category.get('entries', []):
            logical_id = entry.get('logicalId')
            if logical_id in PASS271_REQUIRED_LOGICAL_IDS:
                existing = entry.get('binding', {}).get('existingManifestId')
                if isinstance(existing, str) and existing:
                    bindings[logical_id] = existing
    missing = sorted(set(PASS271_REQUIRED_LOGICAL_IDS) - set(bindings))
    if missing:
        errors.append(f'pass271 source-evidenced logical bindings absent from current worktree: {missing}')

    manifests: dict[str, dict] = {}
    for path in sorted((ROOT / 'assets-v2/manifests').glob('firestaff-v2-*.manifest.json')):
        data = json.loads(read(path))
        for asset in data.get('assets', []):
            if isinstance(asset, dict) and isinstance(asset.get('id'), str):
                manifests[asset['id']] = asset
    for logical_id, asset_id in sorted(bindings.items()):
        asset = manifests.get(asset_id)
        if not asset:
            errors.append(f'{logical_id} binds missing manifest asset {asset_id}')
            continue
        source = asset.get('sourceReference')
        if not isinstance(source, dict) or not source.get('sourceEvidence'):
            errors.append(f'{logical_id} bound asset lacks sourceReference.sourceEvidence: {asset_id}')
    return bindings, errors


def firestaff_checks() -> list[str]:
    errors: list[str] = []
    for rel, needle in FIRESTAFF_REQUIRED:
        text = read(ROOT / rel)
        if needle not in text:
            errors.append(f'missing Firestaff anchor {needle!r} in {rel}')
    return errors


def main() -> int:
    anchors, errors = line_anchor_checks()
    dunview = read(SOURCE / 'DUNVIEW.C')
    order, order_errors = order_checks(dunview)
    bindings, binding_errors = pass271_binding_checks()
    firestaff_errors = firestaff_checks()
    errors.extend(order_errors)
    errors.extend(binding_errors)
    errors.extend(firestaff_errors)

    result = {
        'status': 'failed' if errors else 'passed',
        'scope': 'DM1 V2 viewport composition source stack/order gate',
        'redmcsbSourceRoot': str(SOURCE),
        'sourceAnchors': anchors,
        'drawOrder': order,
        'pass271LogicalBindings': bindings,
        'compositionStatus': 'source stack/order locked; full rendered screenshot/pixel parity remains unproven',
        'nextBlockers': [
            'Build renderer-side D0-D3 draw-list emission from this source order.',
            'Capture original/Firestaff viewport screenshots for same dungeon state.',
            'Compare full viewport pixels/regions; only then claim visual parity.',
        ],
        'errors': errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    if errors:
        for error in errors:
            print('error:', error)
        return 1
    print(f'dm1_v2_viewport_composition_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
