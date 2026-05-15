# Pass560 DM1 V1 mirrored door-front source lock

Status: passed

Claim: D3R, D2L, and D2R front-door branches use ReDMCSB's two-pass door-front order, including mirrored right-side cell orders for D3R/D2R.

## Primary ReDMCSB Evidence

- PASS d3r-mirrored-door-front-split (DUNVIEW.C:6578-6602)
  - line 6578: case C17_ELEMENT_DOOR_FRONT:
  - line 6579: F0108_DUNGEONVIEW_DrawFloorOrnament(L0203_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M590_VIEW_FLOOR_D3R);
  - line 6580: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0203_ai_SquareAspect[M550_FIRST_THING], P0150_i_Direction, P0151_i_MapX, P0152_i_MapY, M602_VIEW_SQUARE_D3R, C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT);
  - line 6589: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2122_, C720_ZONE_DOOR_FRAME_LEFT_D3R);
  - line 6590: F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2120_DoorFrameLeftD3L, C721_ZONE_DOOR_FRAME_RIGHT_D3R);
  - line 6593: F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C0_VIEW_DOOR_BUTTON_D3R);
  - line 6599: F0111_DUNGEONVIEW_DrawDoor(L0203_ai_SquareAspect[M557_DOOR_THING_INDEX], L0203_ai_SquareAspect[M556_DOOR_STATE], G0693_ai_DoorNativeBitmapIndex_Front_D3LCR, C0_VIEW_DOOR_ORNAMENT_D3LCR, M626_ZONE_DOOR_D3R);
  - line 6601: L0202_i_Order = C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT;
  - line 6602: goto T0117018;

- PASS d2l-door-front-split (DUNVIEW.C:6987-7004)
  - line 6987: case C17_ELEMENT_DOOR_FRONT:
  - line 6988: F0108_DUNGEONVIEW_DrawFloorOrnament(L0208_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M591_VIEW_FLOOR_D2L);
  - line 6989: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0208_ai_SquareAspect[M550_FIRST_THING], P0156_i_Direction, P0157_i_MapX, P0158_i_MapY, M604_VIEW_SQUARE_D2L, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);
  - line 6998: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2114_DoorFrameTopD2L, C729_ZONE_DOOR_FRAME_TOP_D2L);
  - line 7001: F0111_DUNGEONVIEW_DrawDoor(L0208_ai_SquareAspect[M557_DOOR_THING_INDEX], L0208_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M627_ZONE_DOOR_D2L);
  - line 7003: L0207_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;
  - line 7004: goto T0119020;

- PASS d2r-mirrored-door-front-split (DUNVIEW.C:7180-7197)
  - line 7180: case C17_ELEMENT_DOOR_FRONT:
  - line 7181: F0108_DUNGEONVIEW_DrawFloorOrnament(L0210_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M593_VIEW_FLOOR_D2R);
  - line 7182: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0210_ai_SquareAspect[M550_FIRST_THING], P0159_i_Direction, P0160_i_MapX, P0161_i_MapY, M605_VIEW_SQUARE_D2R, C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT);
  - line 7191: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2113_DoorFrameTopD2R, C731_ZONE_DOOR_FRAME_TOP_D2R);
  - line 7194: F0111_DUNGEONVIEW_DrawDoor(L0210_ai_SquareAspect[M557_DOOR_THING_INDEX], L0210_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M629_ZONE_DOOR_D2R);
  - line 7196: L0209_i_Order = C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT;
  - line 7197: goto T0120029;

## Firestaff Evidence

- PASS firestaff-mirrored-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:135-142)
  - line 137: DM1_VIEW_SQUARE_D3R, 0x0128, 0x0439
  - line 137: DUNVIEW.C:6579 floor ornament under mirrored rear pass
  - line 137: DUNVIEW.C:6592-6593 optional button before door panel
  - line 139: DM1_VIEW_SQUARE_D2L, 0x0218, 0x0349
  - line 139: DUNVIEW.C:6988 floor ornament under rear pass
  - line 140: DM1_VIEW_SQUARE_D2R, 0x0128, 0x0439
  - line 140: DUNVIEW.C:7181 floor ornament under mirrored rear pass

- PASS firestaff-mirrored-door-front-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:435-491)
  - line 451: { DM1_VIEW_SQUARE_D3R, "6579", "6580", "6582", "6592", "6598", "6601", 0x0128, 0x0439, {2, 1}, {3, 4} },
  - line 453: { DM1_VIEW_SQUARE_D2L, "6988", "6989", "6991", NULL,   "7000", "7003", 0x0218, 0x0349, {1, 2}, {4, 3} },
  - line 454: { DM1_VIEW_SQUARE_D2R, "7181", "7182", "7184", NULL,   "7193", "7196", 0x0128, 0x0439, {2, 1}, {3, 4} },
  - line 459: check_int("door_front_occlusion.count", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 7);
  - line 478: rear.cells[0] == expected[i].rear_cells[0]
  - line 482: front.cells[0] == expected[i].front_cells[0]

## Verification

- /home/trv2/work/firestaff-worktrees/pass560-dm1v1-mirrored-door-front-source-lock-20260515-bosse/build-pass560/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.door_front_occlusion == 1
PASS source_evidence.d1c_door_front_occlusion == 1
PASS source_evidence.d1c_door_button_occlusion == 1
PASS source_evidence.side_occlusion == 1
PASS source_evidence.defs_zones == 1
PASS source_evidence.wall_source_clip_gate == 1
PASS source_evidence.wall_empty_blit_gate == 1
PASS source_evidence.occlusion == 1
PASS source_evidence.command_dispatch == 1
PASS source_evidence.next_redraw == 1
PASS source_evidence.present_wait == 1
PASS dm1_v1_viewport_3d_source_lock
~~~

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/pass560-dm1v1-mirrored-door-front-source-lock-20260515-bosse/tools/verify_pass560_dm1_v1_mirrored_door_front_source_lock.py --check-only: rc=0
~~~
PASS pass560 check-only
~~~

## Non-Claims

- No input or movement queue code was changed.
- No original DOS pixel parity is claimed.
- DANNESBURK was not used.
