# Pass565 DM1 V1 D1 side door-front source lock

Status: passed

Claim: D1L and mirrored D1R front-door branches use ReDMCSB two-pass door-front order: one rear side cell is drawn before the top frame and door, then one front side cell is drawn after the door.

## Primary ReDMCSB Evidence

- PASS d1l-door-front-split (DUNVIEW.C:7492-7536)
  - line 7492: case C17_ELEMENT_DOOR_FRONT:
  - line 7493: F0108_DUNGEONVIEW_DrawFloorOrnament(L0214_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M594_VIEW_FLOOR_D1L);
  - line 7494: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT);
  - line 7503: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L);
  - line 7506: F0111_DUNGEONVIEW_DrawDoor(L0214_ai_SquareAspect[M557_DOOR_THING_INDEX], L0214_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M630_ZONE_DOOR_D1L);
  - line 7508: L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;
  - line 7536: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, L0213_i_Order);

- PASS d1r-door-front-split (DUNVIEW.C:7660-7704)
  - line 7660: case C17_ELEMENT_DOOR_FRONT:
  - line 7661: F0108_DUNGEONVIEW_DrawFloorOrnament(L0216_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M596_VIEW_FLOOR_D1R);
  - line 7662: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT);
  - line 7671: F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_FRAME_TOP_D1R);
  - line 7674: F0111_DUNGEONVIEW_DrawDoor(L0216_ai_SquareAspect[M557_DOOR_THING_INDEX], L0216_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M632_ZONE_DOOR_D1R);
  - line 7676: L0215_i_Order = C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT;
  - line 7704: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, L0215_i_Order);

## Firestaff Evidence

- PASS firestaff-d1-side-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:135-148)
  - line 144: DM1_VIEW_SQUARE_D1L, 0x0028, 0x0039
  - line 144: DUNVIEW.C:7508-7536 pass2 front-right cell after door
  - line 145: DM1_VIEW_SQUARE_D1R, 0x0018, 0x0049
  - line 145: DUNVIEW.C:7676-7704 pass2 front-left cell after door

- PASS firestaff-d1-side-door-front-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:435-505)
  - line 458: DM1_VIEW_SQUARE_D1L
  - line 458: 0x0028, 0x0039
  - line 459: DM1_VIEW_SQUARE_D1R
  - line 459: 0x0018, 0x0049
  - line 463: door_front_occlusion.count
  - line 463: 11
  - line 501: door_front_occlusion.d1l_side_door_front_spec

- PASS firestaff-d1-side-door-front-source-evidence (dm1_v1_viewport_3d_pc34_compat.c:970-985)
  - line 976: DUNVIEW.C:7493-7536
  - line 977: DUNVIEW.C:7661-7704

## Verification

- /home/trv2/work/firestaff-worktrees/pass565-dm1v1-d1-side-door-front-source-lock-20260515-bosse/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.door_front_occlusion == 1
PASS source_evidence.far_door_front_occlusion == 1
PASS source_evidence.d1_side_door_front_occlusion == 1
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

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/pass565-dm1v1-d1-side-door-front-source-lock-20260515-bosse/tools/verify_pass565_dm1_v1_d1_side_door_front_source_lock.py --check-only: rc=0
~~~
PASS pass565 check-only
~~~

## Non-Claims

- No input or movement code was changed.
- No original DOS pixel parity is claimed.
- DANNESBURK was not used.
