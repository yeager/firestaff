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

- PASS firestaff-d1-side-door-front-metadata (dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 288: DM1_VIEW_SQUARE_D1L, 0x0028, 0x0039
  - line 289: DM1_VIEW_SQUARE_D1R, 0x0018, 0x0049

- PASS firestaff-d1-side-door-front-runtime-test (test_dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 45: DM1_VIEW_SQUARE_D1L
  - line 46: DM1_VIEW_SQUARE_D1R
  - line 767: door_front_occlusion.count
  - line 805: door_front_occlusion.d1l_side_door_front_spec

- PASS firestaff-d1-side-door-front-source-evidence (dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 2332: DUNVIEW.C:7493-7536
  - line 2333: DUNVIEW.C:7661-7704

## Verification

- /Users/bosse/workspace-main/firestaff/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS drift.pass576.d2l2_wall present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass576.d0l_wall present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass576.wall_clip_gate present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass576.test_wall_source_row_clip present in tests/test_dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d1l_visible_square present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d0c_visible_square present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.d1c_projectile present in src/dm1/dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass577.runtime_test present in tests/test_dm1_v1_viewport_3d_pc34_compat.c
PASS drift.pass510.party_tuple_source_citation present in src/engine/m11_game_view.c
PASS drift.pass510.party_tuple_flip_predicate present in src/engine/m11_game_view.c
PASS drift.pass510.wallset_variant_binding present in src/engine/m11_game_view.c
PASS drift.pass510.center_wall_flip_path present in src/engine/m11_game_view.c
PASS drift.pass510.side_wall_lr_swap_path present in src/engine/m11_game_view.c
PASS dm1_v1_viewport_3d_source_lock
~~~

- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/workspace-main/firestaff/tools/verify_pass565_dm1_v1_d1_side_door_front_source_lock.py --check-only: rc=0
~~~
PASS pass565 check-only
~~~

## Non-Claims

- No input or movement code was changed.
- No original DOS pixel parity is claimed.
- DANNESBURK was not used.
