#!/usr/bin/env python3
"""Gate pass279 DM1 V2 DUNGEON.DAT square decoder evidence."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DM1_DUNGEON_DAT = (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT")
EVIDENCE = ROOT / 'parity-evidence/verification/pass279_dm1_v2_dungeon_dat_square_decoder.json'
SOURCE_CANDIDATES = [
    (Path.home() / ".openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-dm/HARDDISK/SOURCE"),
    (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source"),
    (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
]

FIRESTAFF_ANCHORS = [
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_DungeonDatState'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_build_composition_from_dungeon'),
    ('dm1_v2_viewport_renderer_pc34.c', 'LOADSAVE.C:906-923'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DEFS.H:972-1016'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNGEON.C:2238-2239'),
    ('dm1_v2_viewport_renderer_pc34.c', 'DUNGEON.C:2243-2246'),
    ('dm1_v2_viewport_renderer_pc34.c', 'dm1_v2_vp_dungeon_dat_get_square_raw'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_dungeon_dat_decoder_entry_draw_list'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'raw == 0xB0'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'DM1_V2_VIEW_SQUARE_D3L'),
    ('CMakeLists.txt', 'dm1_v2_dungeon_dat_square_decoder_source_lock'),
]

SOURCE_ANCHORS = {
    'DEFS.H': ['M034_SQUARE_TYPE(square)', 'RawMapDataByteOffset', 'Width', 'Height'],
    'LOADSAVE.C': ['RawMapDataByteCount', 'G0276_puc_DungeonRawMapData', 'G0279_pppuc_DungeonMapData', 'RawMapDataByteOffset'],
    'DUNGEON.C': ['F0151_DUNGEON_GetSquare', 'F0152_DUNGEON_GetRelativeSquare', 'F0173_DUNGEON_SetCurrentMap'],
    'DUNVIEW.C': ['F0127_DUNGEONVIEW_DrawSquareD0C', 'F0124_DUNGEONVIEW_DrawSquareD1C', 'F0116_DUNGEONVIEW_DrawSquareD3L'],
}


def source_dir() -> Path | None:
    for candidate in SOURCE_CANDIDATES:
        if all((candidate / name).is_file() for name in SOURCE_ANCHORS):
            return candidate
    return None


def le16(data: bytes, offset: int) -> int:
    return data[offset] | (data[offset + 1] << 8)


def parse_map0(data: bytes) -> dict[str, int]:
    off = 44
    raw_offset = le16(data, off)
    packed_a = le16(data, off + 8)
    width = ((packed_a >> 6) & 31) + 1
    height = ((packed_a >> 11) & 31) + 1
    raw_count = le16(data, 2)
    raw_file_offset = len(data) - 2 - raw_count
    def sq(x: int, y: int) -> int:
        return data[raw_file_offset + raw_offset + x * height + y]
    return {
        'rawMapDataByteOffset': raw_offset,
        'level': packed_a & 63,
        'width': width,
        'height': height,
        'rawMapDataFileOffset': raw_file_offset,
        'checksumFileOffset': raw_file_offset + raw_count,
        'entryD0C_raw_x1_y3': sq(1, 3),
        'entryD1C_raw_x1_y4': sq(1, 4),
        'entryD3L_raw_x0_y6': sq(0, 6),
    }


def main() -> int:
    errors: list[str] = []
    source = source_dir()
    source_checks: dict[str, list[str]] = {}
    if source is None:
        errors.append('missing ReDMCSB source directory with DEFS/LOADSAVE/DUNGEON/DUNVIEW')
    else:
        for filename, needles in SOURCE_ANCHORS.items():
            text = (source / filename).read_text(encoding='utf-8', errors='replace')
            source_checks[filename] = needles
            for needle in needles:
                if needle not in text:
                    errors.append(f'missing source anchor {needle!r} in {source / filename}')

    for rel, needle in FIRESTAFF_ANCHORS:
        text = (ROOT / rel).read_text(encoding='utf-8', errors='replace')
        if needle not in text:
            errors.append(f'missing Firestaff anchor {needle!r} in {rel}')

    if not DM1_DUNGEON_DAT.is_file():
        errors.append(f'missing canonical DUNGEON.DAT: {DM1_DUNGEON_DAT}')
        dungeon = {}
        sha = None
    else:
        data = DM1_DUNGEON_DAT.read_bytes()
        sha = hashlib.sha256(data).hexdigest()
        initial = le16(data, 8)
        dungeon = {
            'path': str(DM1_DUNGEON_DAT),
            'byteCount': len(data),
            'sha256': sha,
            'ornamentRandomSeed': le16(data, 0),
            'rawMapDataByteCount': le16(data, 2),
            'mapCount': data[4],
            'textDataWordCount': le16(data, 6),
            'initialPartyLocationRaw': f'0x{initial:04X}',
            'initialState': {'mapIndex': 0, 'mapX': initial & 31, 'mapY': (initial >> 5) & 31, 'direction': (initial >> 10) & 3},
            'squareFirstThingCount': le16(data, 10),
            'map0': parse_map0(data),
        }
        expected = {
            'byteCount': 33357,
            'sha256': 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85',
            'rawMapDataByteCount': 12283,
            'mapCount': 14,
            'initialPartyLocationRaw': '0x0861',
            'initialState': {'mapIndex': 0, 'mapX': 1, 'mapY': 3, 'direction': 2},
        }
        for key, value in expected.items():
            if dungeon.get(key) != value:
                errors.append(f'DUNGEON.DAT {key} mismatch: {dungeon.get(key)!r} != {value!r}')
        expected_map0 = {'level': 0, 'width': 18, 'height': 19, 'rawMapDataFileOffset': 21072,
                         'entryD0C_raw_x1_y3': 0xB0, 'entryD1C_raw_x1_y4': 0x30, 'entryD3L_raw_x0_y6': 0x00}
        for key, value in expected_map0.items():
            if dungeon['map0'].get(key) != value:
                errors.append(f'map0 {key} mismatch: {dungeon["map0"].get(key)!r} != {value!r}')

    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps({
        'pass': 'pass279_dm1_v2_dungeon_dat_square_decoder',
        'status': 'PASS' if not errors else 'FAIL',
        'redmcsbSource': str(source) if source else None,
        'sourceChecks': source_checks,
        'firestaffAnchors': [{'file': rel, 'needle': needle} for rel, needle in FIRESTAFF_ANCHORS],
        'dungeonDat': dungeon,
        'claim': 'Decoder feeds canonical DUNGEON.DAT raw map squares into the D0-D3 draw-list; no pixel parity claim.',
        'errors': errors,
    }, indent=2, sort_keys=True) + '\n')
    if errors:
        for error in errors:
            print(error)
        return 1
    print(f'dm1_v2_dungeon_dat_square_decoder_source_lock: ok evidence={EVIDENCE.relative_to(ROOT)}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
