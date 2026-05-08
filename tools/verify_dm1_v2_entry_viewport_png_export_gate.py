#!/usr/bin/env python3
from __future__ import annotations
import json
import struct
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PNG = ROOT / 'parity-evidence/verification/pass285_dm1_v2_firestaff_entry_viewport_224x136.png'
EVIDENCE = ROOT / 'parity-evidence/verification/pass285_dm1_v2_entry_viewport_png_export_gate.json'
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")


def png_size(path: Path) -> tuple[int, int]:
    data = path.read_bytes()
    if data[:8] != b'\x89PNG\r\n\x1a\n' or data[12:16] != b'IHDR':
        raise ValueError('not a PNG with IHDR')
    return struct.unpack('>II', data[16:24])


def main() -> int:
    errors: list[str] = []
    anchors = [
        ('DUNVIEW.C', 8337, 'if (G0297_B_DrawFloorAndCeilingRequested)'),
        ('DUNVIEW.C', 8338, 'F0098_DUNGEONVIEW_DrawFloorAndCeiling();'),
        ('DUNVIEW.C', 8490, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, -1'),
        ('DUNVIEW.C', 8542, 'F0127_DUNGEONVIEW_DrawSquareD0C'),
        ('DUNVIEW.C', 8606, 'F0097_DUNGEONVIEW_DrawViewport'),
    ]
    anchor_checks = []
    for rel, line, needle in anchors:
        lines = (SOURCE / rel).read_text(encoding='utf-8', errors='replace').splitlines()
        actual = lines[line - 1].strip() if 0 <= line - 1 < len(lines) else ''
        if needle not in actual:
            errors.append(f'{rel}:{line}: expected {needle!r}, got {actual!r}')
        anchor_checks.append({'file': rel, 'line': line, 'needle': needle, 'actual': actual})
    for rel, needle in [
        ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_render_composition_flat'),
        ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_write_png_rgba'),
        ('dm1_v2_viewport_renderer_pc34.c', 'dm1_v2_vp_build_composition_from_dungeon'),
        ('tools/dm1_v2_export_entry_viewport_png.c', 'dm1_v2_vp_build_composition_from_dungeon(&dungeon, 0, 1, 3, 2'),
        ('CMakeLists.txt', 'dm1_v2_entry_viewport_png_export_gate'),
    ]:
        text = (ROOT / rel).read_text(encoding='utf-8', errors='replace')
        if needle not in text:
            errors.append(f'missing Firestaff anchor {needle!r} in {rel}')
    if not PNG.exists():
        errors.append(f'missing PNG {PNG.relative_to(ROOT)}')
        size = None
        sha = None
    else:
        try:
            size = png_size(PNG)
            if size != (224, 136):
                errors.append(f'PNG size must be 224x136, got {size[0]}x{size[1]}')
        except ValueError as exc:
            errors.append(str(exc))
            size = None
        sha = subprocess.check_output(['sha256sum', str(PNG)], text=True).split()[0]
    result = {
        'status': 'failed' if errors else 'passed',
        'pass': 'pass285_dm1_v2_entry_viewport_png_export_gate',
        'scope': 'Firestaff-side PNG export seam for dm1_v2_vp_build_composition_from_dungeon(map=0,x=1,y=3,dir=2)',
        'png': str(PNG.relative_to(ROOT)),
        'pngSize': {'width': size[0], 'height': size[1]} if size else None,
        'pngSha256': sha,
        'pixelParityClaim': False,
        'explicitBlocker': 'PNG is a deterministic Firestaff composition/materialization seam only; original asset-backed renderer pixels are not wired and no matched original-vs-Firestaff pixel comparison was performed.',
        'redmcsbSourceRoot': str(SOURCE),
        'sourceAnchors': anchor_checks,
        'errors': errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    if errors:
        for error in errors:
            print('error:', error)
        return 1
    print(f'dm1_v2_entry_viewport_png_export_gate: ok png={PNG.relative_to(ROOT)} sha256={sha}')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
