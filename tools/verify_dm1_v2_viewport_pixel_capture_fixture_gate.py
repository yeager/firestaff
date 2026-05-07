#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source")
FIXTURE = ROOT / 'parity-evidence/fixtures/pass280_dm1_v2_entry_viewport_pixel_capture_fixture.json'
EVIDENCE = ROOT / 'parity-evidence/verification/pass280_dm1_v2_viewport_pixel_capture_fixture.json'

SOURCE_ANCHORS = [
    ('DEFS.H', 2478, 'C112_BYTE_WIDTH_VIEWPORT 112'),
    ('DEFS.H', 2484, 'C136_HEIGHT_VIEWPORT 136'),
    ('DEFS.H', 3752, 'C007_ZONE_VIEWPORT'),
    ('DUNVIEW.C', 2962, 'void F0098_DUNGEONVIEW_DrawFloorAndCeiling'),
    ('DUNVIEW.C', 2992, 'G0086_puc_Bitmap_ViewportBlackArea'),
    ('DUNVIEW.C', 2995, 'F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport)'),
    ('DUNVIEW.C', 2996, 'F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea)'),
    ('DUNVIEW.C', 2999, 'M100_PIXEL_WIDTH(G0296_puc_Bitmap_Viewport) = G2073_C224_ViewportPixelWidth'),
    ('DUNVIEW.C', 3000, 'M101_PIXEL_HEIGHT(G0296_puc_Bitmap_Viewport) = G2074_C136_ViewportHeight'),
    ('DUNVIEW.C', 8337, 'if (G0297_B_DrawFloorAndCeilingRequested)'),
    ('DUNVIEW.C', 8338, 'F0098_DUNGEONVIEW_DrawFloorAndCeiling();'),
    ('DUNVIEW.C', 8490, 'F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(P0183_i_Direction, 3, -1'),
    ('DUNVIEW.C', 8542, 'F0127_DUNGEONVIEW_DrawSquareD0C'),
    ('DUNVIEW.C', 8606, 'F0097_DUNGEONVIEW_DrawViewport'),
    ('DUNVIEW.C', 8610, 'F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)'),
    ('BASE.C', 1309, 'F0132_VIDEO_Blit(G0296_puc_Bitmap_Viewport, G0348_Bitmap_Screen'),
]

FIRESTAFF_ANCHORS = [
    ('dm1_v2_viewport_renderer_pc34.h', '#define DM1_V2_VIEWPORT_W 224'),
    ('dm1_v2_viewport_renderer_pc34.h', '#define DM1_V2_VIEWPORT_H 136'),
    ('dm1_v2_viewport_renderer_pc34.h', 'DM1_V2_ViewportRegion'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_compare_viewport_region'),
    ('dm1_v2_viewport_renderer_pc34.h', 'dm1_v2_vp_build_composition_from_dungeon'),
    ('dm1_v2_viewport_renderer_pc34.c', 'dm1_v2_vp_dungeon_dat_get_square_raw'),
    ('dm1_v2_viewport_renderer_pc34.c', 'dm1_v2_vp_compare_viewport_region'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_dungeon_dat_decoder_entry_draw_list'),
    ('test_dm1_v2_movement_viewport_pc34.c', 'test_viewport_region_comparator_scaffold'),
    ('tools/verify_dm1_v2_completion_matrix.py', 'dm1_v2_viewport_pixel_capture_fixture_gate'),
    ('CMakeLists.txt', 'dm1_v2_viewport_pixel_capture_fixture_gate'),
]

EXPECTED_FIXTURE = {
    'fixture': 'pass280_dm1_v2_entry_viewport_pixel_capture_fixture',
    'status': 'source_locked_capture_path_only',
    'pixelParityClaim': False,
    'state': {
        'game': 'Dungeon Master PC34',
        'dungeonDatSha256': 'd90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85',
        'initialPartyLocationRawLE': '0x0861',
        'mapIndex': 0,
        'mapX': 1,
        'mapY': 3,
        'direction': 2,
    },
    'region': {'name': 'full_pc34_viewport', 'x': 0, 'y': 0, 'width': 224, 'height': 136},
    'referenceCapture': {
        'runtime': 'original DM1 PC34 under DOSBox/ReDMCSB source-locked DUNVIEW path',
        'requiredArtifact': 'verification-screens/pass280-dm1-v2-entry-viewport/original/entry_viewport_224x136.png',
        'routeRequirement': 'validated route must leave entrance/menu and land on map=0 x=1 y=3 dir=2 before capture; do not infer from a screenshot alone',
        'captureHarness': 'scripts/dosbox_dm1_original_viewport_reference_capture.sh --run then normalize/crop to 224x136',
    },
    'firestaffCapture': {
        'runtime': 'Firestaff DM1 V2 renderer using decoded canonical DUNGEON.DAT state',
        'requiredArtifact': 'verification-screens/pass280-dm1-v2-entry-viewport/firestaff/entry_viewport_224x136.png',
        'stateBuilder': 'dm1_v2_vp_build_composition_from_dungeon(map=0,x=1,y=3,dir=2)',
    },
    'compare': {
        'comparator': 'dm1_v2_vp_compare_viewport_region',
        'mode': 'exact RGBA pixel equality over full_pc34_viewport',
        'expectedComparedPixels': 224 * 136,
        'promotionRule': 'Only promote beyond blocker when both required artifacts exist for the same state and mismatch count is zero.',
    },
    'knownBlocker': 'No matched original/ReDMCSB and Firestaff entry-state viewport PNG pair is present in this pass.',
}

def line_at(rel: str, line: int) -> str:
    lines = (SOURCE / rel).read_text(encoding='utf-8', errors='replace').splitlines()
    return lines[line - 1].strip() if 1 <= line <= len(lines) else ''

def main() -> int:
    errors: list[str] = []
    checks = []
    for rel, line, needle in SOURCE_ANCHORS:
        actual = line_at(rel, line)
        if needle not in actual:
            errors.append(f'{rel}:{line}: expected {needle!r}, got {actual!r}')
        checks.append({'file': rel, 'line': line, 'needle': needle, 'actual': actual})
    for rel, needle in FIRESTAFF_ANCHORS:
        text = (ROOT / rel).read_text(encoding='utf-8', errors='replace')
        if needle not in text:
            errors.append(f'missing Firestaff anchor {needle!r} in {rel}')
    if not FIXTURE.exists():
        errors.append(f'missing fixture {FIXTURE.relative_to(ROOT)}')
        fixture = None
    else:
        fixture = json.loads(FIXTURE.read_text(encoding='utf-8'))
        for key in ('fixture', 'status', 'pixelParityClaim', 'state', 'region', 'referenceCapture', 'firestaffCapture', 'compare', 'knownBlocker'):
            if key not in fixture:
                errors.append(f'fixture missing key {key}')
        if fixture != EXPECTED_FIXTURE:
            errors.append('fixture JSON differs from source-locked expected schema/content')
        if fixture.get('pixelParityClaim') is not False:
            errors.append('fixture must explicitly avoid a pixel parity claim')
        region = fixture.get('region', {})
        if region.get('width') * region.get('height') != 224 * 136:
            errors.append(f'fixture region must cover 224x136 pixels, got {region}')
        if fixture.get('compare', {}).get('expectedComparedPixels') != 224 * 136:
            errors.append('expectedComparedPixels must be 30464')
    result = {
        'status': 'failed' if errors else 'passed',
        'pass': 'pass280_dm1_v2_viewport_pixel_capture_fixture',
        'scope': 'source-locked path for matched original/ReDMCSB and Firestaff entry viewport pixel capture/comparison',
        'redmcsbSourceRoot': str(SOURCE),
        'sourceAnchors': checks,
        'fixturePath': str(FIXTURE.relative_to(ROOT)),
        'fixture': fixture,
        'noPixelParityClaim': fixture.get('pixelParityClaim') is False if isinstance(fixture, dict) else False,
        'blockerIsExact': 'No matched original/ReDMCSB and Firestaff entry-state viewport PNG pair is present in this pass.' if not errors else None,
        'errors': errors,
    }
    EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
    EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    if errors:
        for error in errors:
            print('error:', error)
        return 1
    print(f'dm1_v2_viewport_pixel_capture_fixture_gate: ok evidence={EVIDENCE.relative_to(ROOT)}')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
