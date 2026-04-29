#!/usr/bin/env python3
"""Source-locked probe for ReDMCSB D3C viewport draw ordering.

This does not emulate rendering. It audits DUNVIEW.C and verifies that the
D3C front-square function still contains the source call sequence used in the
companion report.
"""
from __future__ import annotations
import hashlib
import re
from pathlib import Path

SRC = Path('/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C')
text = SRC.read_text(errors='replace')
lines = text.splitlines()

def find_line(needle: str) -> int:
    for i, line in enumerate(lines, 1):
        if needle in line:
            return i
    raise SystemExit(f'MISSING: {needle}')

anchors = {
    'F0118_start': 'STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF',
    'D3C_set_aspect': 'F0172_DUNGEON_SetSquareAspect(L0206_ai_SquareAspect',
    'D3C_wall_draw': 'F0100_DUNGEONVIEW_DrawWallSetBitmap(G0698_puc_Bitmap_WallSet_Wall_D3LCR, G0163_aauc_Graphic558_Frame_Walls[M600_VIEW_SQUARE_D3C]',
    'D3C_wall_ornament_alcove': 'F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0206_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M578_VIEW_WALL_D3C_FRONT)',
    'D3C_door_floor_ornament': 'F0108_DUNGEONVIEW_DrawFloorOrnament(L0206_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M589_VIEW_FLOOR_D3C);',
    'D3C_door_pass1': 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0206_ai_SquareAspect[M550_FIRST_THING], P0153_i_Direction, P0154_i_MapX, P0155_i_MapY, M600_VIEW_SQUARE_D3C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);',
    'D3C_door_draw': 'F0111_DUNGEONVIEW_DrawDoor(L0206_ai_SquareAspect[M557_DOOR_THING_INDEX], L0206_ai_SquareAspect[M556_DOOR_STATE], G0693_ai_DoorNativeBitmapIndex_Front_D3LCR',
    'D3C_door_pass2_order': 'L0204_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;',
    'D3C_corridor_order': 'L0204_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;',
    'D3C_final_f0115': 'F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0206_ai_SquareAspect[M550_FIRST_THING], P0153_i_Direction, P0154_i_MapX, P0155_i_MapY, M600_VIEW_SQUARE_D3C, L0204_i_Order);',
    'D3C_teleporter_field_after': 'F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M600_VIEW_SQUARE_D3C]], C704_ZONE_WALL_D3C);',
    'F0115_start': 'STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF',
    'F0115_comment_objects': 'draw each object found',
    'F0115_comment_creature': 'Draw one creature at the cell being processed',
    'F0115_comment_projectiles': 'Draw only projectiles at specified cell',
    'F0115_comment_explosions': 'Draw only explosions at specified cell',
    'F0115_nibble_decode': 'AL0126_i_ViewCell = M001_ORDINAL_TO_INDEX',
    'F0115_object_blit': 'F0132_VIDEO_Blit(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport, L0145_auc_Box, AL0127_i_X',
    'F0115_creature_blit': 'F0132_VIDEO_Blit(AL0128_puc_Bitmap, G0296_puc_Bitmap_Viewport, L0145_auc_Box, AP0141_ui_CreatureX',
    'F0115_projectile_loop': 'P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of objects from the beginning. The next loop draws only projectile objects among the list */',
}
positions = {name: find_line(needle) for name, needle in anchors.items()}
# The D3C corridor/pit/teleporter path must set order, draw floor ornament, then call F0115.
assert positions['D3C_corridor_order'] < positions['D3C_final_f0115']
assert positions['D3C_door_floor_ornament'] < positions['D3C_door_pass1'] < positions['D3C_door_draw'] < positions['D3C_door_pass2_order'] < positions['D3C_final_f0115']
assert positions['F0115_comment_objects'] < positions['F0115_comment_creature'] < positions['F0115_comment_projectiles'] < positions['F0115_comment_explosions']
assert positions['F0115_nibble_decode'] < positions['F0115_object_blit'] < positions['F0115_creature_blit'] < positions['F0115_projectile_loop']
start, end = positions['F0118_start'], positions['D3C_teleporter_field_after'] + 3
block = '\n'.join(lines[start-1:end])
print(f'source={SRC}')
print(f'dunview_sha256={hashlib.sha256(text.encode()).hexdigest()}')
print(f'F0118_D3C_range={start}-{end}')
print(f'F0118_D3C_block_sha256={hashlib.sha256(block.encode()).hexdigest()}')
for name in sorted(positions, key=positions.get):
    print(f'{name}={positions[name]}')
print('verified=d3c_source_order_anchors_ok')
