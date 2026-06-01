# Pass570 DM1 V1 D2C front-order source lock

Status: passed

Claim: ReDMCSB D2C is drawn after D2L/D2R and before D1. Its front wall returns unless the front ornament is an alcove, its front door uses rear-cell pass, frame/button/door, then front-cell pass, and its open/pit/teleporter tail draws floor/ceiling/F0115 before the teleporter field overlay.

## Primary ReDMCSB Evidence

- PASS f0128-d2c-position (DUNVIEW.C:8510-8522)
  - line 8513: F0119_DUNGEONVIEW_DrawSquareD2L
  - line 8517: F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
  - line 8521: F0121_DUNGEONVIEW_DrawSquareD2C

- PASS d2c-front-wall-alcove-return (DUNVIEW.C:7289-7312)
  - line 7289: case C00_ELEMENT_WALL:
  - line 7300: F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C
  - line 7308: F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0212_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M583_VIEW_WALL_D2C_FRONT)
  - line 7309: L0211_i_Order = C0x0000_CELL_ORDER_ALCOVE;
  - line 7310: goto T0121016;
  - line 7312: return;

- PASS d2c-door-front-two-pass-order (DUNVIEW.C:7313-7342)
  - line 7313: case C17_ELEMENT_DOOR_FRONT:
  - line 7314: F0108_DUNGEONVIEW_DrawFloorOrnament(L0212_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M592_VIEW_FLOOR_D2C);
  - line 7315: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING], P0162_i_Direction, P0163_i_MapX, P0164_i_MapY, M603_VIEW_SQUARE_D2C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);
  - line 7328: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2115_DoorFrameTopD2LCR, C730_ZONE_DOOR_FRAME_TOP_D2C);
  - line 7329: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2118_DoorFrameLeftD2C, C724_ZONE_DOOR_FRAME_LEFT_D2C);
  - line 7330: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2118_DoorFrameLeftD2C, C725_ZONE_DOOR_FRAME_RIGHT_D2C);
  - line 7333: F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C2_VIEW_DOOR_BUTTON_D2C);
  - line 7339: F0111_DUNGEONVIEW_DrawDoor(L0212_ai_SquareAspect[M557_DOOR_THING_INDEX], L0212_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M628_ZONE_DOOR_D2C);
  - line 7341: L0211_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;
  - line 7342: goto T0121016;

- PASS d2c-open-pit-teleporter-tail (DUNVIEW.C:7353-7388)
  - line 7353: case C05_ELEMENT_TELEPORTER:
  - line 7354: case C01_ELEMENT_CORRIDOR:
  - line 7356: L0211_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;
  - line 7357: F0108_DUNGEONVIEW_DrawFloorOrnament(L0212_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M592_VIEW_FLOOR_D2C);
  - line 7365: F0112_DUNGEONVIEW_DrawCeilingPit(C065_GRAPHIC_CEILING_PIT_D2C, C865_ZONE_CEILING_PIT_D2C
  - line 7368: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING], P0162_i_Direction, P0163_i_MapX, P0164_i_MapY, M603_VIEW_SQUARE_D2C, L0211_i_Order);
  - line 7386: F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M603_VIEW_SQUARE_D2C]], C709_ZONE_WALL_D2C);

- PASS pc34-i34e-d2c-zones (DEFS.H:4049-4088)
  - line 4049: #define C709_ZONE_WALL_D2C
  - line 4082: #define C724_ZONE_DOOR_FRAME_LEFT_D2C
  - line 4083: #define C725_ZONE_DOOR_FRAME_RIGHT_D2C
  - line 4088: #define C730_ZONE_DOOR_FRAME_TOP_D2C

## Firestaff Evidence

- PASS firestaff-d2c-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:240-244)
  - line 240: DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349
  - line 240: DUNVIEW.C:7314 floor ornament under rear pass
  - line 240: DUNVIEW.C:7315 pass1 rear cells before frame
  - line 240: DUNVIEW.C:7317-7333 top/side frame and button draw
  - line 240: DUNVIEW.C:7332-7334 optional button before door panel
  - line 240: DUNVIEW.C:7339 F0111 door bitmap/ornament
  - line 240: DUNVIEW.C:7341 pass2 front cells after door

- PASS firestaff-d2c-floor-field-metadata (dm1_v1_viewport_3d_pc34_compat.c:346-353)
  - line 346: DM1_VIEW_SQUARE_D2C, 0x3421
  - line 348: DUNVIEW.C:7260-7288 stairs front bitmap before common floor/thing path
  - line 349: DUNVIEW.C:7343-7353 pit bitmap before floor ornament
  - line 350: DUNVIEW.C:7355-7357 order then F0108 floor ornament
  - line 351: DUNVIEW.C:7367-7368 F0115 object/creature/projectile/explosion handoff
  - line 352: DUNVIEW.C:7370-7388 teleporter field after F0115
  - line 353: DUNVIEW.C:7289-7312 wall bitmap/ornament then return unless front alcove branches to F0115

- PASS firestaff-d2c-wall-metadata (dm1_v1_viewport_3d_pc34_compat.c:420-428)
  - line 422: DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C
  - line 422: DM1_PC34_ZONE_WALL_D2C
  - line 422: DUNVIEW.C:7299-7306
  - line 422: DUNVIEW.C:7308-7312 front alcove branches to F0115, else return

- PASS firestaff-d2c-zone-defines (dm1_v1_viewport_3d_pc34_compat.h:460-477)
  - line 463: #define DM1_PC34_ZONE_WALL_D2C
  - line 474: #define DM1_PC34_ZONE_DOOR_FRAME_LEFT_D2C   724
  - line 475: #define DM1_PC34_ZONE_DOOR_FRAME_RIGHT_D2C  725
  - line 476: #define DM1_PC34_ZONE_DOOR_FRAME_TOP_D2C    730

- PASS firestaff-d2c-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:707-770)
  - line 729: { DM1_VIEW_SQUARE_D2C, "7314", "7315", "7317", "7332", "7339", "7341", 0x0218, 0x0349, {1, 2}, {4, 3} },
  - line 735: check_int("door_front_occlusion.count", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 11);
  - line 754: rear.cells[0] == expected[i].rear_cells[0]
  - line 758: front.cells[0] == expected[i].front_cells[0]

- PASS firestaff-d2c-source-evidence (dm1_v1_viewport_3d_pc34_compat.c:2114-2126)
  - line 2118: DUNVIEW.C:7314-7341 D2C door-front occlusion: rear pass, frame/door, front pass
  - line 2121: DEFS.H:4082-4088 PC34/I34E D2C door-frame zones 724/725/730
  - line 2122: DUNVIEW.C:7289-7312 D2C front wall: wall zone, front ornament/alcove exception, else return before open-cell draw
  - line 2123: DUNVIEW.C:7353-7387 D2C open/pit/teleporter order: 0x3421 floor/ceiling/F0115, then field overlay

## Verification

- /Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS source_evidence.same_viewport_mouse == 1
PASS source_evidence.same_viewport_turn == 1
PASS source_evidence.same_viewport_move == 1
PASS source_evidence.same_viewport_draw == 1
PASS source_evidence.same_viewport_present == 1
PASS source_evidence.same_viewport_assets == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/.openclaw/workspace-main/tools/verify_pass570_dm1_v1_d2c_front_order_source_lock.py --check-only: rc=0
~~~
PASS pass570 check-only
~~~

## Non-Claims

- No renderer runtime behavior was changed.
- No original DOS pixel parity is claimed.
- No movement/input/capture behavior is changed.
- DANNESBURK was not used.
