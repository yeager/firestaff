# Pass565 DM1 V1 D0C Thieves Eye door-frame occlusion

Status: failed

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

- PASS firestaff-d0c-thieves-eye-metadata (dm1_v1_viewport_3d_pc34_compat.c:155-180)
  - line 161: DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736
  - line 162: DUNVIEW.C:8185-8188
  - line 163: DUNVIEW.C:8199-8201
  - line 164: DUNVIEW.C:8206-8210
  - line 165: DUNVIEW.C:8215-8216
  - line 166: DUNVIEW.C:8240,8294

- FAIL firestaff-d0c-thieves-eye-test (test_dm1_v1_viewport_3d_pc34_compat.c:609-628)
  - missing: test_d0c_thieves_eye_door_frame_occlusion_order
  - missing: dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_VIEW_SQUARE_D0C)
  - missing: spec->door_frame_zone, 728
  - missing: spec->hole_zone, 736
  - missing: 8215-8216
  - missing: 8294

- FAIL firestaff-source-evidence-string (dm1_v1_viewport_3d_pc34_compat.c:1112-1115)
  - missing: DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion
  - missing: copy front frame, composite hole, blit temporary frame before common F0115

## Verification

- /home/trv2/work/firestaff-worktrees/pass593-dm1v1-landable-batch/build/test_dm1_v1_viewport_3d_pc34_compat: rc=0
~~~
PASS source_evidence.door_front_occlusion == 1
PASS source_evidence.d2c_front_order == 1
PASS source_evidence.far_door_front_occlusion == 1
PASS source_evidence.d1_side_door_front_occlusion == 1
PASS source_evidence.d1c_door_front_occlusion == 1
PASS source_evidence.d1c_door_button_occlusion == 1
PASS source_evidence.d0c_thieves_eye_frame_occlusion == 1
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

- /usr/bin/python3 /home/trv2/work/firestaff-worktrees/pass593-dm1v1-landable-batch/tools/verify_pass565_dm1_v1_d0c_thieves_eye_door_frame_occlusion.py --check-only: rc=1
~~~
FAIL pass565 check-only: firestaff-d0c-thieves-eye-test, firestaff-source-evidence-string
~~~
