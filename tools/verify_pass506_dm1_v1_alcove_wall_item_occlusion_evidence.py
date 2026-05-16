#!/usr/bin/env python3
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import re
from typing import Any
ROOT = Path(__file__).resolve().parents[1]
FIRE = ROOT / 'src/engine/m11_game_view.c'
CMAKE = ROOT / 'CMakeLists.txt'
EVIDENCE = ROOT / 'parity-evidence/pass506_dm1_v1_alcove_wall_item_occlusion_evidence.md'
OUT_DIR = ROOT / 'parity-evidence/verification/pass506_dm1_v1_alcove_wall_item_occlusion_evidence'
MANIFEST = OUT_DIR / 'manifest.json'
RED_ROOT = Path('~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source').expanduser()
def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()
def line_no(text: str, offset: int) -> int:
    return text.count('\n', 0, offset) + 1
def require(text: str, needle: str, label: str) -> int:
    pos = text.find(needle)
    if pos < 0:
        raise AssertionError(f'{label}: missing {needle!r}')
    return pos
def require_in_order(text: str, markers: list[str], label: str) -> list[int]:
    positions = []
    last = -1
    for marker in markers:
        pos = text.find(marker, last + 1)
        if pos < 0:
            raise AssertionError(f'{label}: missing {marker!r}')
        positions.append(pos)
        last = pos
    return positions
def find_function(text: str, name: str) -> tuple[int, int, str]:
    m = re.search(r'\b' + re.escape(name) + r'\s*\(', text)
    if not m:
        raise AssertionError(f'missing function {name}')
    brace = text.find('{', m.end())
    if brace < 0:
        raise AssertionError(f'missing body for {name}')
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return m.start(), pos + 1, text[m.start():pos + 1]
    raise AssertionError(f'unterminated function {name}')
def check_file_needles(path: Path, checks: list[dict[str, Any]]) -> list[dict[str, Any]]:
    text = path.read_text(encoding='latin-1')
    out = []
    for check in checks:
        positions = require_in_order(text, check['needles'], f"ReDMCSB {check['id']}") if check.get('ordered') else [require(text, needle, f"ReDMCSB {check['id']}") for needle in check['needles']]
        out.append({'id': check['id'], 'file': path.name, 'lines': f'{line_no(text, min(positions))}-{line_no(text, max(positions))}', 'needles': check['needles']})
    return out
def main() -> int:
    fire = FIRE.read_text(encoding='utf-8')
    cmake = CMAKE.read_text(encoding='utf-8')
    evidence = EVIDENCE.read_text(encoding='utf-8')
    red_checks = []
    red_checks.extend(check_file_needles(RED_ROOT / 'DUNVIEW.C', [
        {'id': 'alcove-global-ornament-identities', 'needles': ['unsigned char G0192_auc_Graphic558_AlcoveOrnamentIndices[C003_ALCOVE_ORNAMENT_COUNT] = {', '1,   /* Square Alcove */', '2,   /* Vi Altar */', '3 }; /* Arched Alcove */'], 'ordered': True},
        {'id': 'f0107-draws-wall-ornament-and-classifies-alcove', 'needles': ['STATICFUNCTION BOOLEAN F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(', 'AL0088_i_NativeBitmapIndex = G0101_as_CurrentMapWallOrnamentsInfo[AP0116_i_WallOrnamentIndex].NativeBitmapIndex;', 'L0096_B_IsAlcove = F0149_DUNGEON_IsWallOrnamentAnAlcove(AP0116_i_WallOrnamentIndex);', 'F0791_DUNGEONVIEW_DrawBitmapXX(AL0091_puc_Bitmap, G0296_puc_Bitmap_Viewport', 'return L0096_B_IsAlcove;'], 'ordered': True},
        {'id': 'f0115-zero-nibble-alcove-cell', 'needles': ['If the first nibble is 0, then the function call is to draw objects in an alcove on a wall square.', 'L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals);', '/* Draw objects */', 'AL0126_i_ViewCell = C04_VIEW_CELL_ALCOVE; /* Index of coordinates to draw objects in alcoves */', 'L0139_i_Cell = M018_OPPOSITE(P0142_i_Direction);'], 'ordered': True},
        {'id': 'd2r-front-alcove-switches-to-alcove-order', 'needles': ['F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0210_ai_SquareAspect[M553_LEFT_WALL_ORNAMENT_ORDINAL], M581_VIEW_WALL_D2R_LEFT);', 'if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0210_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M584_VIEW_WALL_D2R_FRONT))', 'L0209_i_Order = C0x0000_CELL_ORDER_ALCOVE;'], 'ordered': True},
        {'id': 'd1c-wall-then-alcove-f0115', 'needles': ['F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);', 'if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))', 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);'], 'ordered': True},
    ]))
    red_checks.extend(check_file_needles(RED_ROOT / 'DUNGEON.C', [{'id': 'f0149-current-map-alcove-index-test', 'needles': ['BOOLEAN F0149_DUNGEON_IsWallOrnamentAnAlcove(', 'if (G0267_ai_CurrentMapAlcoveOrnamentIndices[L0247_i_Counter] == P0252_i_WallOrnamentIndex)', 'return C1_TRUE;', 'return C0_FALSE;'], 'ordered': True}]))
    fire_checks = []
    sample_start, _, sample_body = find_function(fire, 'm11_sample_viewport_cell')
    require_in_order(sample_body, ['Do not filter WALL squares here.', 'if (cell.summary.items > 0 && state->world.things)', 'cell.floorItemCells[cell.floorItemCount]'], 'Firestaff item extraction keeps wall-square items for alcoves')
    if 'cell.summary.items > 0 && state->world.things &&\n        cell.elementType != DUNGEON_ELEMENT_WALL' in sample_body:
        raise AssertionError('Firestaff still filters wall-square items before alcove rendering')
    fire_checks.append({'id': 'sample-view-cell-preserves-wall-square-items', 'file': FIRE.name, 'lines': f'{line_no(fire, sample_start)}-{line_no(fire, sample_start + len(sample_body))}'})
    items_start, _, items_body = find_function(fire, 'm11_draw_dm1_alcove_wall_items')
    require_in_order(items_body, ['cell->floorItemCount <= 0', 'C0x0000_CELL_ORDER_ALCOVE', 'C04_VIEW_CELL_ALCOVE', 'M018_OPPOSITE(direction)', 'cell->floorItemCells[ii] != alcoveCellRelativeToParty', 'm11_draw_item_sprite(state, framebuffer, fbW, fbH,'], 'Firestaff alcove draw filters sub-cell then draws item sprite')
    fire_checks.append({'id': 'alcove-pass-filters-to-source-subcell', 'file': FIRE.name, 'lines': f'{line_no(fire, items_start)}-{line_no(fire, items_start + len(items_body))}'})
    wall_start, _, wall_body = find_function(fire, 'm11_draw_dm1_wall_ornaments')
    require_in_order(wall_body, ['m11_blit_scaled_palette_map_maybe_flip(slot, framebuffer, fbW, fbH,', 'if (m11_dm1_wall_ornament_is_alcove_global(ornGlobalIdx))', 'm11_draw_dm1_alcove_wall_items(state, framebuffer, fbW, fbH,', 'm11_dm1_f0115_c2500_c2900_row('], 'Firestaff wall ornament draws before alcove item pass')
    fire_checks.append({'id': 'wall-ornament-before-alcove-item-pass', 'file': FIRE.name, 'lines': f'{line_no(fire, wall_start)}-{line_no(fire, wall_start + len(wall_body))}'})
    require(cmake, 'NAME pass506_dm1_v1_alcove_wall_item_occlusion_evidence', 'CMake registration')
    require(evidence, 'DUNVIEW.C:7840-7844', 'evidence D1C source citation')
    require(evidence, 'DUNVIEW.C:4555-4582', 'evidence F0115 source citation')
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    manifest = {'pass': 'pass506_dm1_v1_alcove_wall_item_occlusion_evidence', 'status': 'passed', 'scope': 'DM1 V1 viewport/world visuals: alcove wall-item occlusion evidence', 'redmcsbRoot': str(RED_ROOT), 'sourceHashes': {'DUNVIEW.C': sha256(RED_ROOT / 'DUNVIEW.C'), 'DUNGEON.C': sha256(RED_ROOT / 'DUNGEON.C'), 'm11_game_view.c': sha256(FIRE), 'CMakeLists.txt': sha256(CMAKE), 'evidence': sha256(EVIDENCE)}, 'redmcsbChecks': red_checks, 'firestaffChecks': fire_checks, 'notClaimed': ['pixel parity', 'new original runtime capture', 'movement-route ownership']}
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    print('pass506 DM1 V1 alcove wall-item occlusion evidence passed')
    for check in red_checks:
        print(f"- ReDMCSB {check['file']}:{check['lines']} {check['id']}")
    for check in fire_checks:
        print(f"- Firestaff {check['file']}:{check['lines']} {check['id']}")
    print(f'- manifest {MANIFEST.relative_to(ROOT)}')
    return 0
if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f'FAIL: {exc}')
        raise SystemExit(1)
