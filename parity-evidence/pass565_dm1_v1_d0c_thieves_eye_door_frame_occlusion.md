# Pass565 DM1 V1 D0C Thieves Eye door-frame occlusion

Status: passed

Claim: ReDMCSB PC34/I34E D0C door-side with Thieves Eye copies the front door frame into a temporary bitmap, composites the hole-in-wall graphic into that temporary frame, blits the temporary frame to C728, then reaches the common D0C F0115 pass with C0x0021. This is a source-lock only.

## Primary ReDMCSB Evidence

- PASS d0c-door-side-thieves-eye-frame-branch (DUNVIEW.C:8185-8216)
  - line 8186: case C16_ELEMENT_DOOR_SIDE:
  - line 8188: if (G0407_s_Party.Event73Count_ThievesEye)
  - line 8200: F0616_CopyBitmap(F0631_GetBitmapPointer(G2116_DoorFrameFrontD0C), G0074_puc_Bitmap_Temporary);
  - line 8201: F0630_InitBitmapStruct2(M711_NEGGRAPHIC_HOLE_IN_WALL, &L2495_s_Struct2);
  - line 8207: F0635_(NULL, L2496_ai_XYZ, C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME, &L2495_s_Struct2.Width, &L2495_s_Struct2.Height);
  - line 8210: F0654_Call_F0132_VIDEO_Blit(M772_CAST_PC(L2495_s_Struct2.s2m1), M772_CAST_PC(G0074_puc_Bitmap_Temporary), L2496_ai_XYZ
  - line 8216: F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C, C10_COLOR_FLESH);

- PASS d0c-common-f0115-after-frame (DUNVIEW.C:8215-8295)
  - line 8216: F0656_BlitBitmapToViewportZoneIndexWithTransparency(G0074_puc_Bitmap_Temporary, C728_ZONE_DOOR_FRAME_D0C, C10_COLOR_FLESH);
  - line 8240: break;
  - line 8294: F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0222_ai_SquareAspect[M550_FIRST_THING], P0180_i_Direction, P0181_i_MapX, P0182_i_MapY, M609_VIEW_SQUARE_D0C, C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT);

- PASS pc34-i34e-zone-ids (DEFS.H:4084-4095)
  - line 4086: #define C728_ZONE_DOOR_FRAME_D0C
  - line 4095: #define C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME

## Firestaff Evidence

- PASS firestaff-d0c-thieves-eye-metadata (dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 305: DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736
  - line 306: DUNVIEW.C:8185-8188
  - line 307: DUNVIEW.C:8199-8201
  - line 308: DUNVIEW.C:8206-8210
  - line 309: DUNVIEW.C:8215-8216
  - line 310: DUNVIEW.C:8240,8294

- PASS firestaff-d0c-thieves-eye-test (test_dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 860: test_d0c_thieves_eye_door_frame_occlusion_order
  - line 863: dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C)
  - line 870: spec->door_frame_zone, 728
  - line 871: spec->hole_zone, 736
  - line 875: 8215-8216
  - line 876: 8294

- PASS firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:1-9999)
  - line 2338: DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion
  - line 2338: copy front frame, composite hole, blit temporary frame before common F0115

## Verification

- /Users/bosse/workspace-main/firestaff/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS drift.pass570.d2c_zone_top present in include/dm1_v1_viewport_3d_pc34_compat.h
PASS drift.pass570.runtime_test present in tests/test_dm1_v1_viewport_3d_pc34_compat.c
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

- /opt/homebrew/opt/python@3.14/bin/python3.14 /Users/bosse/workspace-main/firestaff/tools/verify_pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion.py --check-only: rc=0
~~~
PASS pass565 check-only
~~~
